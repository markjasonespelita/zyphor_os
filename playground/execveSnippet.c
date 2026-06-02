#include <stdio.h>
#include <unistd.h>

int main() {

    char *args[] = {"/bin/ls", "-l", NULL};
    char *env[] = {"PATH=/bin:/usr/bin", NULL};

    execve("/bin/ls", args, env);

    return 0;
}