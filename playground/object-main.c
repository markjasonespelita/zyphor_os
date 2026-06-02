#include <stdio.h>
#include "helpers/headers/helperCommand.h"

typedef struct {
    char name[100];
    int age;
} User;

int main()
{

    User users[] = {
        {"Mark", 32},
        {"Jason", 22}
    };

    int size = sizeof(users) / sizeof(users[0]);

    for(int i = 0; i < size; i++) {
        printf("Name: %s\n", users[i].name);
        printf("Age: %d\n\n", users[i].age);
    }

    return 0;
}