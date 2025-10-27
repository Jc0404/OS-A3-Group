CC = gcc
CFLAGS = -Wall -Wpointer-arith -Wstrict-prototypes -std=gnu89 -fPIC -MMD -MP 
# Ensure local headers such as mergesort.h are visible to tests.
CFLAGS += -I.
LDFLAGS = -lpthread
# Centralise auxiliary test sources under the tests folder.
TEST_DIR := tests
# Place non submission binaries inside tests/bin to keep the hand in folder clean.
TEST_BIN_DIR := $(TEST_DIR)/bin

all: test-mergesort

#This builds an executable 
test-mergesort: test-mergesort.o mergesort.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# small test executable that calls test_merge()
test-merge: $(TEST_BIN_DIR)/test-merge

# Build the merge test binary inside tests/bin.
$(TEST_BIN_DIR)/test-merge: mergesort.o $(TEST_DIR)/test-merge.o | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile the merge test source from the tests directory.
$(TEST_DIR)/test-merge.o: $(TEST_DIR)/test-merge.c
	$(CC) $(CFLAGS) -c -o $@ $<

# test for the sequential my_mergesort implementation
test-my-mergesort: $(TEST_BIN_DIR)/test-my-mergesort

# Build the sequential mergesort test binary alongside other test executables.
$(TEST_BIN_DIR)/test-my-mergesort: mergesort.o $(TEST_DIR)/test-my-mergesort.o | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile the sequential mergesort test source.
$(TEST_DIR)/test-my-mergesort.o: $(TEST_DIR)/test-my-mergesort.c
	$(CC) $(CFLAGS) -c -o $@ $<

# generic rule for compiling .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test-parallel: $(TEST_BIN_DIR)/test-parallel-mergesort
	$(TEST_BIN_DIR)/test-parallel-mergesort

# Build the binary and keep it under tests/bin.
$(TEST_BIN_DIR)/test-parallel-mergesort: $(TEST_DIR)/test_parallel_mergesort.c mergesort.c mergesort.h | $(TEST_BIN_DIR)
	$(CC) -Wall -Wpointer-arith -Wstrict-prototypes -std=gnu89 -fPIC -lpthread -I. -o $@ $(TEST_DIR)/test_parallel_mergesort.c

# ⭐️Ensure the bin folder exists before linking test executables.
$(TEST_BIN_DIR):
	mkdir -p $(TEST_BIN_DIR)

.PHONY: clean
clean:
	# Remove build artefacts and helper test binaries.
	/bin/rm -f *.o *.d test-mergesort test-merge test-my-mergesort

	# Clear relocation-friendly test objects.
	/bin/rm -f $(TEST_DIR)/*.o $(TEST_DIR)/*.d

	# Delete helper executables housed in tests/bin.
	/bin/rm -f $(TEST_BIN_DIR)/*
