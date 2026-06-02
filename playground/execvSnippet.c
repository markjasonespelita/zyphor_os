#include <stdio.h>
#include <unistd.h>

int main() {

    char *args[] = {"/bin/ls", "-l", NULL};

    execv("/bin/ls", args);

    return 0;
}