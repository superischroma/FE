#include <stdio.h>

#include "util.h"

void feInfo(const char* message)
{
    printf("FE: info: %s\n", message);
}

void feErr(const char* message)
{
    printf("FE: error: %s\n", message);
}

void feROMErr(const char* message)
{
    printf("FE: error: Could not load ROM: %s\n", message);
}

void printBin(unsigned char c)
{
    char number[9];
    number[8] = '\0';
    for (int i = 0; i < 8; i++)
    {
        number[7 - i] = (((c >> i) & 0b00000001) != 0) ? '1' : '0';
    }
    printf(number);
}