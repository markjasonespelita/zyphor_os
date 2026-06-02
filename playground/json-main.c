// sudo apt install libcjson-dev

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "helpers/helperString.c"
#include "helpers/helperValidation.c"

#define JSON_FILE "data.json"
#define MAX_INPUT  100

/* ── JSON helpers ─────────────────────────────────────────────────── */

/* Load existing JSON array from file, or create a new empty array */
cJSON *loadData() {
    FILE *fp = fopen(JSON_FILE, "r");
    if (!fp) return cJSON_CreateArray();

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *buf = (char *)malloc(len + 1);
    fread(buf, 1, len, fp);
    buf[len] = '\0';
    fclose(fp);

    cJSON *arr = cJSON_Parse(buf);
    free(buf);

    if (!arr || !cJSON_IsArray(arr)) {
        cJSON_Delete(arr);
        return cJSON_CreateArray();
    }
    return arr;
}

/* Write JSON array back to file */
void saveData(cJSON *arr) {
    FILE *fp = fopen(JSON_FILE, "w");
    if (!fp) { perror("Cannot open " JSON_FILE); return; }
    char *str = cJSON_Print(arr);
    fprintf(fp, "%s\n", str);
    fclose(fp);
    free(str);
    printf("Data saved to %s\n", JSON_FILE);
}

/* Generate next ID (max existing id + 1) */
int nextId(cJSON *arr) {
    int max = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, arr) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
        if (cJSON_IsNumber(id) && id->valueint > max)
            max = id->valueint;
    }
    return max + 1;
}

/* Append a new record {id, name} */
void addRecord(cJSON *arr, const char *name) {
    cJSON *record = cJSON_CreateObject();
    cJSON_AddNumberToObject(record, "id", nextId(arr));
    cJSON_AddStringToObject(record, "name", name);
    cJSON_AddItemToArray(arr, record);
}

/* Display all records in a table */
void displayAll(cJSON *arr) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) {
        printf("\n  (no records found)\n\n");
        return;
    }

    printf("\n  %-6s  %-30s\n", "ID", "Name");
    printf("  %-6s  %-30s\n", "------", "------------------------------");

    cJSON *item;
    cJSON_ArrayForEach(item, arr) {
        cJSON *id   = cJSON_GetObjectItemCaseSensitive(item, "id");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        printf("  %-6d  %-30s\n",
               cJSON_IsNumber(id)  ? id->valueint   : -1,
               cJSON_IsString(name) ? name->valuestring : "(null)");
    }
    printf("\n");
}

/* ── Menu ─────────────────────────────────────────────────────────── */

void showMenu() {
    printf("\n=============================\n");
    printf("  1. Add name\n");
    printf("  2. Display all records\n");
    printf("  3. Exit\n");
    printf("=============================\n");
    printf("Choice: ");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main() {
    char input[MAX_INPUT];
    int  running = 1;

    cJSON *data = loadData();

    while (running) {
        showMenu();

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';   /* strip newline */

        int choice = atoi(input);

        switch (choice) {
            case 1: {
                char name[MAX_INPUT];
                while (1) {
                    printf("What is your name: ");
                    if (!fgets(name, sizeof(name), stdin)) break;
                    name[strcspn(name, "\n")] = '\0';

                    if (isEmpty(name)) {
                        printf("No Name. Please try again.\n");
                    } else {
                        addRecord(data, name);
                        saveData(data);
                        printf("Hello %s! (ID: %d)\n",
                               name, nextId(data) - 1);
                        break;
                    }
                }
                break;
            }
            case 2:
                printf("\n--- All Records ---");
                displayAll(data);
                break;

            case 3:
                printf("Goodbye!\n");
                running = 0;
                break;

            default:
                printf("Invalid choice. Please enter 1, 2, or 3.\n");
        }
    }

    cJSON_Delete(data);
    return 0;
}