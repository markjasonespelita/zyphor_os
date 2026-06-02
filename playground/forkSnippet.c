#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Running fork snippet...\n");

    pid_t pid = fork();

    if (pid == 0) {
        printf("I'm a child: %d\n", getpid());
        
        char *args[] = {"google-chrome", "https://google.com", NULL};

        execv("/usr/bin/google-chrome", args);

    } else {
        printf("I'm a parent: %d\n", getpid());
    }

    return 0;
}