#include "../headers/debug.h"

double __time_exec_wtime__()
{
    struct timeval t;

    gettimeofday(&t, NULL);

    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}