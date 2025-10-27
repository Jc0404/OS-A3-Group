#include <stdio.h>
#include <stdlib.h>
#include "mergesort.h"

/* globals used by mergesort.c */
/* ⭐️cutoff is unused here but required to satisfy extern linkage.*/
int cutoff = 0;
int *A;
int *B;

int main(void)
{
    int n = 10; /* small demo size: two halves of 5 elements each */
    int i;

    A = (int *)malloc(sizeof(int) * n);
    B = (int *)malloc(sizeof(int) * n);
    if (!A || !B)
    {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* first sorted half: 1, 3, 5, 7, 9 */
    A[0] = 1;
    A[1] = 3;
    A[2] = 5;
    A[3] = 7;
    A[4] = 9;
    /* second sorted half: 2, 4, 6, 8, 10 */
    A[5] = 2;
    A[6] = 4;
    A[7] = 6;
    A[8] = 8;
    A[9] = 10;

    printf("Before merge:\n");
    for (i = 0; i < n; i++)
        printf("%d%c", A[i], (i == n - 1) ? '\n' : ' ');

    /* merge the two sorted halves */
    merge(0, 4, 5, 9);

    printf("After merge:\n");
    for (i = 0; i < n; i++)
        printf("%d%c", A[i], (i == n - 1) ? '\n' : ' ');

    /* quick verification */
    for (i = 0; i < n - 1; i++)
    {
        if (A[i] > A[i + 1])
        {
            printf("Result: not sorted\n");
            free(A);
            free(B);
            return 2;
        }
    }
    printf("Result: correctly sorted\n");

    free(A);
    free(B);
    return 0;
}
