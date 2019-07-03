/*
 * test_bulk.cpp
 *
 *  Created on: 29 июн. 2019 г.
 *      Author: sveta
 */

#define BOOST_TEST_MODULE ip_filter_test_module
#include <boost/test/included/unit_test.hpp>
#include "bulkmt.h"

BOOST_AUTO_TEST_SUITE(bulk_test_suite)

BOOST_AUTO_TEST_CASE(bulk__some_test_case)
{
	CommandProcessor p(2);
	p.ProcessCommand("cmd1");
	BOOST_REQUIRE_EQUAL(1,p.GetBulkSize());

}

BOOST_AUTO_TEST_SUITE_END()





