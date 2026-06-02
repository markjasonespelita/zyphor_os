#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void helperCommand(char *_command, ...) {
    char **execArgs = NULL;
    int count = 0;

    // Check if _command contains spaces (one sentence command)
    if (strchr(_command, ' ') != NULL) {
        // Split the command string into tokens
        char *copy = strdup(_command);
        char *token = strtok(copy, " ");

        while (token != NULL) {
            execArgs = realloc(execArgs, (count + 1) * sizeof(char *));
            execArgs[count++] = token;
            token = strtok(NULL, " ");
        }
        execArgs = realloc(execArgs, (count + 1) * sizeof(char *));
        execArgs[count] = NULL;

        execv(execArgs[0], execArgs);
        free(copy);

    } else {
        // Original variadic behavior
        va_list args;

        // Count args
        va_start(args, _command);
        char *arg;
        while ((arg = va_arg(args, char *)) != NULL) count++;
        va_end(args);

        execArgs = malloc((count + 2) * sizeof(char *));
        execArgs[0] = _command;

        va_start(args, _command);
        for (int i = 1; i <= count; i++) {
            execArgs[i] = va_arg(args, char *);
        }
        execArgs[count + 1] = NULL;
        va_end(args);

        execv(_command, execArgs);
    }

    free(execArgs);
}