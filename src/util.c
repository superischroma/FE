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