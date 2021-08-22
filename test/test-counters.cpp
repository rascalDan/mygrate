#define BOOST_TEST_MODULE Counters
#include <boost/test/unit_test.hpp>

#include "helpers.h"

BOOST_FIXTURE_TEST_SUITE(ec, EventCounterTarget);

BOOST_AUTO_TEST_CASE(zero)
{
	std::array<unsigned long, ENUM_END_EVENT> z {};
	BOOST_CHECK_EQUAL_COLLECTIONS(counters.begin(), counters.end(), z.begin(), z.end());
}

BOOST_AUTO_TEST_CASE(tick_getAll)
{
	BOOST_CHECK_EQUAL(&counters, &getAll());
}

BOOST_AUTO_TEST_CASE(tick_one)
{
	tick(ROTATE_EVENT);
	BOOST_CHECK_EQUAL(get(ROTATE_EVENT), 1);
}

BOOST_AUTO_TEST_CASE(tick_three)
{
	add(ROTATE_EVENT, 3);
	BOOST_CHECK_EQUAL(get(ROTATE_EVENT), 3);
}

BOOST_AUTO_TEST_CASE(tick_different)
{
	add(ROTATE_EVENT, 2).add(UPDATE_ROWS_EVENT_V1, 1);
	BOOST_CHECK_EQUAL(get(ROTATE_EVENT), 2);
	BOOST_CHECK_EQUAL(get(UPDATE_ROWS_EVENT_V1), 1);
}

BOOST_AUTO_TEST_CASE(compare_zero)
{
	EventCounter other;
	BOOST_CHECK(other >= *this);
}

BOOST_AUTO_TEST_CASE(compare_one)
{
	EventCounter other;
	tick(ROTATE_EVENT);

	BOOST_CHECK(!(other >= *this));

	other.tick(ROTATE_EVENT);
	BOOST_CHECK(other >= *this);

	other.tick(ROTATE_EVENT);
	BOOST_CHECK(other >= *this);
}

BOOST_AUTO_TEST_CASE(compare_many)
{
	EventCounter other;
	add(ROTATE_EVENT, 2).add(UPDATE_ROWS_EVENT_V1, 1);

	BOOST_CHECK(!(other >= *this));

	other.tick(UPDATE_ROWS_EVENT_V1);
	BOOST_CHECK(!(other >= *this));

	other.tick(UPDATE_ROWS_EVENT_V1);
	BOOST_CHECK(!(other >= *this));

	other.tick(ROTATE_EVENT);
	BOOST_CHECK(!(other >= *this));

	other.tick(ROTATE_EVENT);
	BOOST_CHECK(other >= *this);

	other.tick(ROTATE_EVENT);
	BOOST_CHECK(other >= *this);

	other.tick(UPDATE_ROWS_EVENT_V1);
	BOOST_CHECK(other >= *this);
}

BOOST_AUTO_TEST_SUITE_END();
