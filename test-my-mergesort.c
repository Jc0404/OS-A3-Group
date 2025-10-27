#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "mergesort.h"

/* define globals expected by mergesort.c */
/* ⭐️Provide cutoff so the linker can resolve the extern declared in mergesort.c. */
int cutoff = 0;
int *A;
int *B;

#define RANGE 1000000

/* timing function (milliseconds) */
double getMilliSeconds(void)
{
    struct timeval now;
    gettimeofday(&now, (struct timezone *)0);
    return (double)now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}

/* simple check-if-sorted (nondecreasing) */
int check_if_sorted_local(int A_local[], int n)
{
    int i;
    for (i = 0; i < n - 1; i++)
    {
        if (A_local[i] > A_local[i + 1])
            return 0;
    }
    return 1;
}

/* generate random array */
void generate_random_array_local(int A_local[], int n, unsigned int seed)
{
    int i;
    srandom(seed);
    for (i = 0; i < n; i++)
    {
        A_local[i] = (int)(random() % RANGE);
    }
}

/* print first up to 100 elements */
void print_prefix(int A_local[], int n)
{
    int i, limit;
    limit = (n < 100) ? n : 100;
    printf("Array A:");
    for (i = 0; i < limit; i++)
    {
        printf(" %d", A_local[i]);
    }
    if (n > 100)
        printf(" ...");
    printf("\n");
}

int main(int argc, char **argv)
{
    int n = 100;
    unsigned int seed;
    double start_time, sort_time;
    int ok;

    /* parse args: <array_size> <seed> */
    if (argc == 3)
    {
        n = atoi(argv[1]);
        if (n < 2)
        {
            fprintf(stderr, "Error: array size must be >= 2\n");
            return 1;
        }
        seed = (unsigned int)atoi(argv[2]);
    }
    else if (argc == 1)
    {
        seed = (unsigned int)time(NULL);
    }
    else
    {
        fprintf(stderr, "Usage: %s [<array_size> <seed>]\n", argv[0]);
        fprintf(stderr, "Default: array_size=100, seed=current_time\n");
        return 1;
    }

    A = (int *)malloc(sizeof(int) * n);
    B = (int *)malloc(sizeof(int) * n);
    if (A == NULL || B == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    generate_random_array_local(A, n, seed);

    printf("Testing my_mergesort with array size %d and seed %u\n", n, seed);
    print_prefix(A, n);

    start_time = getMilliSeconds();
    my_mergesort(0, n - 1);
    sort_time = getMilliSeconds() - start_time;

    print_prefix(A, n);

    ok = check_if_sorted_local(A, n);
    if (ok)
    {
        printf("Array is correctly sorted\n");
        printf("Sorting %d elements took %4.2lf milliseconds (%4.2lf seconds).\n",
               n, sort_time, sort_time / 1000.0);
    }
    else
    {
        printf("Sorting failed!!!!\n");
    }

    free(A);
    free(B);
    return ok ? 0 : 2;
}
