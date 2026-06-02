#include <stdio.h>
#include <string.h>

void helperString(char _string[], int size) {
    fgets(_string, size, stdin);
	_string[strcspn(_string, "\n")] = '\0';
}