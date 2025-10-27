#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* ⭐️Define the globals that mergesort.c expects so the tests run in isolation. */
int cutoff = 0;
int *A = NULL;
int *B = NULL;

/* Counters track threading behaviour and failure handling. */
static int pthread_create_calls = 0;
static int pthread_join_calls = 0;
static int pthread_create_failures = 0;
static int fail_pthread_create_on_call = 0;

/* forward declarations of our pthread stubs */
static int test_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
							   void *(*start_routine)(void *), void *arg);
static int test_pthread_join(pthread_t thread, void **retval);

/* ⭐️Redirect pthread entry points before including the production code so we can instrument thread behaviour. */
#define pthread_create test_pthread_create
#define pthread_join test_pthread_join
/* ⭐️Include the production source via a relative path because the test now resides inside the project folder. */
#include "../mergesort.c"
#undef pthread_create
#undef pthread_join

/* ⭐️Reset counters between cases so observations stay independent. */
static void reset_counters(void)
{
	pthread_create_calls = 0;
	pthread_join_calls = 0;
	pthread_create_failures = 0;
	fail_pthread_create_on_call = 0;
}

/* ⭐️Allocate fresh buffers for the next run. */
static void ensure_capacity(int n)
{
	free(A);
	free(B);
	A = malloc((size_t)n * sizeof(int));
	B = malloc((size_t)n * sizeof(int));
	assert(A != NULL && B != NULL);
}

/* ⭐️Initialise a descending array so real sorting work happens. */
static void fill_descending(int n)
{
	int i;
	for (i = 0; i < n; ++i)
	{
		A[i] = n - i;
	}
}

/* ⭐️Simple sortedness check used by every scenario. */
static bool is_sorted(int n)
{
	int i;
	for (i = 1; i < n; ++i)
	{
		if (A[i - 1] > A[i])
		{
			return false;
		}
	}
	return true;
}

/* ⭐️Reusable printer showing the observed counters after each scenario. */
static void print_metrics(const char *label, bool sorted_ok)
{
	printf("[%s]\n", label);
	printf("  pthread_create_calls = %d, pthread_join_calls = %d\n",
		   pthread_create_calls, pthread_join_calls);
	printf("  array_sorted = %s\n", sorted_ok ? "YES" : "NO");
	printf("  fail_trigger = %d, pthread_create_failures = %d\n",
		   fail_pthread_create_on_call, pthread_create_failures);
	// printf("metrics debug: label=%s sorted=%d\n", label, sorted_ok);
}

/* ⭐️Validate buildArgs fills the struct correctly. */
static void test_buildArgs_basic(void)
{
	reset_counters();
	struct argument *arg = buildArgs(3, 7, 2);
	assert(arg != NULL);
	bool ok = (arg->left == 3 && arg->right == 7 && arg->level == 2);
	print_metrics("buildArgs", true);
	printf("  struct fields -> left=%d right=%d level=%d (expected 3,7,2)\n",
		   arg->left, arg->right, arg->level);
	printf("  field_match = %s\n", ok ? "PASS" : "FAIL");
	assert(ok);
	free(arg);
}

/* ⭐️cutoff=0 forces pure sequential execution. */
static void test_parallel_cutoff_zero(void)
{
	reset_counters();
	cutoff = 0;
	const int n = 16;
	ensure_capacity(n);
	fill_descending(n);

	struct argument *root = buildArgs(0, n - 1, 0);
	assert(root != NULL);
	parallel_mergesort(root);

	bool sorted_ok = is_sorted(n);
	print_metrics("cutoff=0", sorted_ok);
	printf("  expectation: pthread_create_calls == 0 -> %s\n",
		   pthread_create_calls == 0 ? "PASS" : "FAIL");
	assert(pthread_create_calls == 0);
	assert(sorted_ok);
}

/* ⭐️Positive cutoff should create threads and still sort correctly. */
static void test_parallel_with_threads(void)
{
	reset_counters();
	cutoff = 2;
	const int n = 32;
	ensure_capacity(n);
	fill_descending(n);

	struct argument *root = buildArgs(0, n - 1, 0);
	assert(root != NULL);
	parallel_mergesort(root);

	bool sorted_ok = is_sorted(n);
	print_metrics("cutoff=2", sorted_ok);
	printf("  expectation: pthread_create_calls >= 2 -> %s\n",
		   pthread_create_calls >= 2 ? "PASS" : "FAIL");
	assert(pthread_create_calls >= 2);
	assert(sorted_ok);
}

/* ⭐️Simulate thread creation failure and expect graceful fallback. */
static void test_parallel_thread_failure(void)
{
	reset_counters();
	cutoff = 2;
	const int n = 24;
	ensure_capacity(n);
	fill_descending(n);

	fail_pthread_create_on_call = 1;
	struct argument *root = buildArgs(0, n - 1, 0);
	assert(root != NULL);
	parallel_mergesort(root);

	bool sorted_ok = is_sorted(n);
	print_metrics("create failure", sorted_ok);
	printf("  expectation: pthread_create_failures >= 1 -> %s\n",
		   pthread_create_failures >= 1 ? "PASS" : "FAIL");
	assert(pthread_create_failures >= 1);
	assert(sorted_ok);
}

/* ⭐️Stubbed pthread_create records calls and optionally fails. */
static int test_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
							   void *(*start_routine)(void *), void *arg)
{
	(void)thread;
	(void)attr;

	pthread_create_calls++;
	if (fail_pthread_create_on_call > 0 &&
		pthread_create_calls == fail_pthread_create_on_call)
	{
		pthread_create_failures++;
		return 1;
	}

	start_routine(arg);
	return 0;
}

/* ⭐️Stubbed pthread_join simply tracks how many joins occur. */
static int test_pthread_join(pthread_t thread, void **retval)
{
	(void)thread;
	(void)retval;
	pthread_join_calls++;
	return 0;
}

int main(void)
{
	printf("=== parallel_mergesort test ===\n");
	test_buildArgs_basic();
	test_parallel_cutoff_zero();
	test_parallel_with_threads();
	test_parallel_thread_failure();
	printf("Summary: all scenarios completed successfully.\n");

	free(A);
	free(B);
	return 0;
}
