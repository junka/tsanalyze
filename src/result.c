#include <stdlib.h>
#include <stdio.h>

static FILE *f;

int res_open(const char *filename)
{
    f = fopen(filename, "w");
    return 0;
}

int res_put(const char * str)
{
    fprintf(f,"%s", str);
    return 0;
}