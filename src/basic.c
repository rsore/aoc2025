#include "basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char *
read_entire_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *result = malloc(fsize + 1);
    usize read = fread(result, fsize, 1, f); UNUSED(read);
    fclose(f);

    result[fsize] = 0;
    return result;
}

char *
sprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    s32 need = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (need < 0) return NULL;

    char *buffer = (char *)malloc((usize)need+1);

    va_start(args, fmt);
    vsnprintf(buffer, (size_t)need + 1, fmt, args);
    buffer[(usize)need] = '\0';
    va_end(args);

    return buffer;
}
