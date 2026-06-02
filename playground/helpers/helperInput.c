#include <stdio.h>

int helperInt() {
    int value;

    while (scanf("%d", &value) != 1) {
        printf("Invalid integer. Try again: ");

        while (getchar() != '\n');
    }

    while (getchar() != '\n');

    return value;
}

float helperFloat() {
    float value;

    while (scanf("%f", &value) != 1) {
        printf("Invalid float. Try again: ");

        while (getchar() != '\n');
    }

    while (getchar() != '\n');

    return value;
}

double helperDouble() {
    double value;

    while (scanf("%lf", &value) != 1) {
        printf("Invalid double. Try again: ");

        while (getchar() != '\n');
    }

    while (getchar() != '\n');

    return value;
}