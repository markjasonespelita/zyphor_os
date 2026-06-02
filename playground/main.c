#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    if (strcmp(argv[1], "pkg") == 0) {

        if (strcmp(argv[2], "install") == 0) {

            printf("Running package install %s\n", argv[3]);

            pid_t pid = fork();

            if (pid == 0) {
                // CHILD PROCESS (manual executor)

                char *args[] = {
                    "apt",
                    "install",
                    argv[3],
                    NULL
                };

                execvp("apt", args);

                // if exec fails
                perror("execv failed");
                _exit(1);
            }
            else if (pid > 0) {
                // PARENT PROCESS
                wait(NULL);
            }
            else {
                perror("fork failed");
            }

        } else {
            printf("Invalid pkg command.\n");
        }

    } else {
        printf("Invalid command\n");
    }

    return 0;
}