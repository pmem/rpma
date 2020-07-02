// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * serializer-test.c -- serializer unit tests
 */

#include <stdlib.h>
#include <string.h>

#include "cmocka_headers.h"
#include "serializer.h"

/*
 * Calculate an exptected value of required serialized memory region:
 * exp = raddr + size + rkey + plt
 *
 * Note: plt has two possible values so it fits perfectly into a single
 * uint8_t value.
 */
#define MR_SIZE_EXP \
	(sizeof(uint64_t) * 2 + sizeof(uint32_t) + sizeof(uint8_t))

#define MOCK_ADDR	(uint64_t)0x0001020304050607
#define MOCK_LENGTH	(uint64_t)0x08090a0b0c0d0e0f
#define MOCK_RKEY	(uint32_t)0x10111213
#define MOCK_PLT	(uint8_t)0x14

#define BUFF_EXP	"\x07\x06\x05\x04\x03\x02\x01\x00" \
			"\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08" \
			"\x13\x12\x11\x10" \
			"\x14"

/* setup & teardown */

/*
 * setup_buff -- allocate an accessory buffer
 */
static int
setup_buff(void **buff_ptr)
{
	*buff_ptr = test_malloc(MR_SIZE_EXP * 2);
	return 0;
}

/*
 * teardown_buff -- free the accessory buffer
 */
static int
teardown_buff(void **buff_ptr)
{
	test_free(*buff_ptr);
	*buff_ptr = NULL;
	return 0;
}

/* rpma_ser_mr_get_size() unit test */

/*
 * test_mr_get_size__sanity - verify the returned value is as expected
 */
static void
test_mr_get_size__sanity(void **unused)
{
	/* run test */
	assert_int_equal(rpma_ser_mr_get_size(), MR_SIZE_EXP);
}

/* rpma_ser_mr_serialize() unit test */

/*
 * test_mr_get_size__sanity - verify the returned value is as expected
 */
static void
test_mr_serialize__sanity(void **buff_ptr)
{
	for (char *buff = *buff_ptr; buff < (char *)*buff_ptr + MR_SIZE_EXP;
			++buff) {

		/* run test */
		rpma_ser_mr_serialize(buff, MOCK_ADDR, MOCK_LENGTH, MOCK_RKEY,
				MOCK_PLT);

		/* verify the result */
		assert_memory_equal(buff, BUFF_EXP, MR_SIZE_EXP);
	}
}

/* rpma_ser_mr_deserialize() unit test */

/*
 * test_mr_deserialize__sanity -- verify the returned values are as expected
 */
static void
test_mr_deserialize__sanity(void **buff_ptr)
{
	uint64_t addr = 0;
	uint64_t length = 0;
	uint32_t rkey = 0;
	uint8_t plt = 0;

	for (char *buff = *buff_ptr; buff < (char *)*buff_ptr + MR_SIZE_EXP;
			++buff) {

		/* prepare a buffer contents */
		memcpy(buff, BUFF_EXP, MR_SIZE_EXP);

		/* run test */
		rpma_ser_mr_deserialize(buff, &addr, &length, &rkey, &plt);

		/* verify the results */
		assert_int_equal(addr, MOCK_ADDR);
		assert_int_equal(length, MOCK_LENGTH);
		assert_int_equal(rkey, MOCK_RKEY);
		assert_int_equal(plt, MOCK_PLT);
	}
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_ser_mr_get_size() unit test */
		cmocka_unit_test(test_mr_get_size__sanity),

		/* rpma_ser_mr_serialize() unit test */
		cmocka_unit_test_setup_teardown(test_mr_serialize__sanity,
				setup_buff, teardown_buff),

		/* rpma_ser_mr_deserialize() unit test */
		cmocka_unit_test_setup_teardown(test_mr_deserialize__sanity,
				setup_buff, teardown_buff),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
