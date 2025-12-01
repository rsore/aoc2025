#include "util.h"

#include <stdio.h>
#include <stdlib.h>

char *
read_entire_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *result = malloc(fsize + 1);
    fread(result, fsize, 1, f);
    fclose(f);

    result[fsize] = 0;
    return result;
}
