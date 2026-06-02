#include <stdio.h>

void helperReadFile(char *filename) {

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Unable to open the file");
        return;
    }

    char characters;

    while ((characters = fgetc(file)) != EOF) {
        printf("%c", characters);
    }

    fclose(file);

}