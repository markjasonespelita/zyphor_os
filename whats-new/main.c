#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RELEASE_NOTES "/usr/share/zyphor-whats-new/release-notes.md"

int main(void)
{
    char marker[512];
    char line[1024];
    char *home = getenv("HOME");

    if (home == NULL)
        return 1;

    snprintf(marker, sizeof(marker), "%s/.zyphor-whats-new", home);

    /* Already shown */
    if (access(marker, F_OK) == 0)
        return 0;

    /* Open release notes */
    FILE *notes = fopen(RELEASE_NOTES, "r");

    if (notes == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", RELEASE_NOTES);
        return 1;
    }

    printf("====================================\n");
    printf("      Zyphor OS - What's New\n");
    printf("====================================\n\n");

    while (fgets(line, sizeof(line), notes))
    {
        printf("%s", line);
    }

    fclose(notes);

    /* Create marker */
    FILE *marker_file = fopen(marker, "w");

    if (marker_file != NULL)
    {
        fprintf(marker_file, "seen\n");
        fclose(marker_file);
    }

    return 0;
}