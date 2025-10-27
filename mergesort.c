#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* ⭐️Allow the original stubs to coexist with test overrides by guarding the definitions. */
#ifndef MERGESORT_EXTERNAL_MERGE
void merge(int leftstart, int leftend, int rightstart, int rightend)
{
	// merge the two sorted subarrays A[leftstart..leftend] and A[rightstart..rightend]
	int i = leftstart;
	int j = rightstart;
	int k = leftstart;

	while (i <= leftend && j <= rightend)
	{
		if (A[i] < A[j])
		{
			B[k++] = A[i++];
		}
		else
		{
			B[k++] = A[j++];
		}
	}

	// copy any remaining elements from either subarray
	while (i <= leftend)
	{
		B[k++] = A[i++];
	}
	while (j <= rightend)
	{
		B[k++] = A[j++];
	}

	// copy the merged elements back into the original array
	memcpy(A + leftstart, B + leftstart, (rightend - leftstart + 1) * sizeof(int));
}
#endif

/* Same guard for the sequential baseline so unit tests can provide their own version. */
#ifndef MERGESORT_EXTERNAL_MYMERGESORT
void my_mergesort(int left, int right)
{
	if (left < right)
	{
		int mid = (left + right) / 2;
		my_mergesort(left, mid);
		my_mergesort(mid + 1, right);
		merge(left, mid, mid + 1, right);
	}
}

#endif

/* ⭐️Core parallel entry point invoked by the harness and unit tests. */
void *parallel_mergesort(void *arg)
{
	/* ⭐️Unpack the heap argument so local variables carry the working range; defer freeing until exit. */
	struct argument *in = (struct argument *)arg;
	int left = in->left;
	int right = in->right;
	int level = in->level;

	/* ⭐️Degenerate ranges (size <=1) need no work. */
	if (left >= right)
	{
		return NULL;
	}

	/* ⭐️ Respect the user cutoff by falling back to the sequential helper. */
	if (level >= cutoff)
	{
		my_mergesort(left, right);
		return NULL;
	}

	int mid = left + (right - left) / 2;
	pthread_t tleft;
	pthread_t tright;
	int left_thread_created = 0;
	int right_thread_created = 0;

	/* ⭐️Build child descriptors for both halves; NULL signals fallback. */
	struct argument *left_arg = buildArgs(left, mid, level + 1);
	struct argument *right_arg = buildArgs(mid + 1, right, level + 1);

	/* ⭐️Launch the left worker or degrade to sequential sorting. */
	if (left_arg != NULL)
	{
		if (pthread_create(&tleft, NULL, parallel_mergesort, (void *)left_arg) == 0)
		{
			left_thread_created = 1;
		}
		else
		{
			/* ⭐️On creation failure, reclaim the arg and invoke sequential backup. */
			free(left_arg);
			my_mergesort(left, mid);
		}
	}
	else
	{
		my_mergesort(left, mid);
	}

	/* ⭐️Mirror the logic for the right half, keeping behaviour symmetrical. */
	if (right_arg != NULL)
	{
		if (pthread_create(&tright, NULL, parallel_mergesort, (void *)right_arg) == 0)
		{
			right_thread_created = 1;
		}
		else
		{
			free(right_arg);
			my_mergesort(mid + 1, right);
		}
	}
	else
	{
		my_mergesort(mid + 1, right);
	}

	/* ⭐️Join only the successfully spawned threads to guarantee ordering. */
	if (left_thread_created)
	{
		pthread_join(tleft, NULL);
	}
	if (right_thread_created)
	{
		pthread_join(tright, NULL);
	}

	/* ⭐️Finish by merging the sorted halves actual merge implemented elsewhere. */
	merge(left, mid, mid + 1, right);

	/* ⭐️Free argument blocks owned by spawned worker threads (level>0); the root level is freed by caller. */
	if (level > 0)
	{
		free(in);
	}

	return NULL;
}

/* ⭐️Helper to build argument packages for child invocations. */
struct argument *buildArgs(int left, int right, int level)
{
	/* ⭐️Allocate a dedicated descriptor so child threads own stable metadata. */
	struct argument *arg = (struct argument *)malloc(sizeof(struct argument));
	if (arg == NULL)
	{
		/* ⭐️Propagate failure with NULL so callers can trigger sequential fallback.
		 */
		return NULL;
	}

	/* ⭐️Copy the caller specified bounds and depth into the shared descriptor. */
	arg->left = left;
	arg->right = right;
	arg->level = level;
	return arg;
}
