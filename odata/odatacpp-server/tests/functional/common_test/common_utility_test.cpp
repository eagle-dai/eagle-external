﻿//---------------------------------------------------------------------
// <copyright file="common_utility_test.cpp" company="Microsoft">
//      Copyright (C) Microsoft Corporation. All rights reserved. See License.txt in the project root for license information.
// </copyright>
//---------------------------------------------------------------------

#include "../odata_tests.h"
#include "odata/common/utility.h"

using namespace ::odata::edm;
using namespace ::odata::core;
using namespace ::odata::utility;

namespace tests { namespace functional { namespace _odata {

SUITE(common_utility_test)
{

TEST(strip_string_empty_test)
{
	::odata::utility::string_t case_string = U("");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U(""));
}

TEST(strip_string_empty_quote_test)
{
	::odata::utility::string_t case_string = U("\"\"");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U(""));
}

TEST(strip_string_with_quote_test)
{
	::odata::utility::string_t case_string = U("\"121232123\"");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U("121232123"));
}

TEST(strip_string_without_quote_test)
{
	::odata::utility::string_t case_string = U("121232123");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U("121232123"));
}

TEST(strip_string_with_single_left_quote_test)
{
	::odata::utility::string_t case_string = U("\"121232123");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U("121232123"));
}

TEST(strip_string_with_single_right_quote_test)
{
	::odata::utility::string_t case_string = U("121232123\"");
	auto ret = strip_string(case_string);
	VERIFY_ARE_EQUAL(ret, U("121232123"));
}

TEST(split_string_empty_src_with_empty_delim_test)
{
	::odata::utility::string_t case_string = U("");
	::odata::utility::string_t delim_string = U("");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 1);
}

TEST(split_string_with_no_empty_input_test)
{
	::odata::utility::string_t case_string = U("23123.23232.32323");
	::odata::utility::string_t delim_string = U(".");
	std::list<::odata::utility::string_t> ret;
	ret.push_back(U("1"));
	ret.push_back(U("2"));
	ret.push_back(U("3"));
	ret.push_back(U("4"));
	ret.push_back(U("5"));

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 3);
}

TEST(split_string_empty_src_with_delim_test)
{
	::odata::utility::string_t case_string = U("");
	::odata::utility::string_t delim_string = U("..");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 1);
}

TEST(split_string_src_with_empty_delim_test)
{
	::odata::utility::string_t case_string = U("adfadfadfdas");
	::odata::utility::string_t delim_string = U("");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 1);
}

TEST(split_string_src_with_delim_notfind_test)
{
	::odata::utility::string_t case_string = U("adfadfadfdas");
	::odata::utility::string_t delim_string = U("..");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 1);
}

TEST(split_string_src_with_delim_find_test)
{
	::odata::utility::string_t case_string = U("..adf..ad..fa..dfdas..");
	::odata::utility::string_t delim_string = U("..");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 6);
}

TEST(split_string_src_with_duplicated_delim_test)
{
	::odata::utility::string_t case_string = U("...adf..ad....fa..dfdas...");
	::odata::utility::string_t delim_string = U("..");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 7);
}

TEST(split_string_src_with_single_char_delim_test)
{
	::odata::utility::string_t case_string = U("...adf..ad....fa..dfdas...f");
	::odata::utility::string_t delim_string = U("f");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 5);
}

TEST(split_string_src_with_equal_delim_test)
{
	::odata::utility::string_t case_string = U("...adf..ad....fa..dfdas...f");
	::odata::utility::string_t delim_string = U("...adf..ad....fa..dfdas...f");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 2);
}

TEST(split_string_src_with_exceed_length_delim_test)
{
	::odata::utility::string_t case_string = U("...adf..ad....fa..dfdas...f");
	::odata::utility::string_t delim_string = U("...adf..ad....fa..dfdas...fdfdfdfdf");
	std::list<::odata::utility::string_t> ret;

	split_string(case_string, delim_string, ret);
	VERIFY_ARE_EQUAL(ret.size(), 1);
	VERIFY_ARE_EQUAL(ret.front(), U("...adf..ad....fa..dfdas...f"));
}

TEST(is_relative_path_empty_root_wity_empty_path)
{
	::odata::utility::string_t root_string = U("");
	::odata::utility::string_t path_string = U("");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, false);
}

TEST(is_relative_path_empty_root)
{
	::odata::utility::string_t root_string = U("");
	::odata::utility::string_t path_string = U("Accounts");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, false);
}

TEST(is_relative_path_empty_path)
{
	::odata::utility::string_t root_string = U("http://odatae2etest.azurewebsites.net/cpptest/DefaultService");
	::odata::utility::string_t path_string = U("");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, false);
}

TEST(is_relative_path_valid_root_wity_full_path)
{
	::odata::utility::string_t root_string = U("http://odatae2etest.azurewebsites.net/cpptest/DefaultService");
	::odata::utility::string_t path_string = U("http://odatae2etest.azurewebsites.net/cpptest/DefaultService/Accounts");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, false);
}

TEST(is_relative_path_valid_root_wity_relative_path)
{
	::odata::utility::string_t root_string = U("http://http://odatae2etest.azurewebsites.net/cpptest/DefaultService");
	::odata::utility::string_t path_string = U("http");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, true);
}

TEST(is_relative_path_valid_root_wity_uppercase_full_path)
{
	::odata::utility::string_t root_string = U("http://odatae2etest.azurewebsites.net/cpptest/DefaultService");
	::odata::utility::string_t path_string = U("http://odatae2etest.azurewebsites.net/cpptest/DefaultService/Accounts");

    bool ret = is_relative_path(root_string, path_string);
	VERIFY_ARE_EQUAL(ret, false);
}

TEST(print_double_zero_input)
{
	double zero = 0.0;
	::odata::utility::string_t ret = print_double(zero);
	VERIFY_ARE_EQUAL(ret, U("0.0"));
}

TEST(print_double_zero_input_2)
{
	double zero = 0;
	::odata::utility::string_t ret = print_double(zero);
	VERIFY_ARE_EQUAL(ret, U("0.0"));
}

TEST(print_double_minus_zero_input)
{
	double zero = -0.0;
	::odata::utility::string_t ret = print_double(zero);
	VERIFY_ARE_EQUAL(ret, U("-0.0"));
}

TEST(print_double_minus_zero_input_2)
{
	double zero = -0;
	::odata::utility::string_t ret = print_double(zero);
	VERIFY_ARE_EQUAL(ret, U("0.0"));
}

TEST(print_double_valid_input)
{
	double zero = 212.1234;
	::odata::utility::string_t ret = print_double(zero, 4);
	VERIFY_ARE_EQUAL(ret, U("212.1234"));
}

TEST(print_double_right_exceed_precision_input)
{
	double zero = 123123123.1111222323;
	::odata::utility::string_t ret = print_double(zero, 8);
	VERIFY_ARE_EQUAL(ret, U("123123123.11112224"));
}

TEST(print_double_set_right_precision_input)
{
	double zero = -4212.1111222323;
	::odata::utility::string_t ret = print_double(zero, 13);
	VERIFY_ARE_EQUAL(ret, U("-4212.1111222322997"));
}

TEST(print_double_zero_precision_input)
{
	double zero = -4212.1111222323;
	::odata::utility::string_t ret = print_double(zero, 0);
	VERIFY_ARE_EQUAL(ret, U("-4212"));
}

TEST(print_double_minux_precision_input)
{
	double zero = -4212.11;
	::odata::utility::string_t ret = print_double(zero, 9);
	VERIFY_ARE_EQUAL(ret, U("-4212.11"));
}

TEST(print_double_large_double_input)
{
	double zero = -12123123123213124212.11;
	::odata::utility::string_t ret = print_double(zero, 1);
#ifdef WIN32
	VERIFY_ARE_EQUAL(ret, U("-12123123123213124000.0"));
#else
    VERIFY_ARE_EQUAL(ret, U("-12123123123213123584.0"));
#endif
}

TEST(print_double_large_double_input_2)
{
	double zero = 1234567891234567.7674567611117999;
	::odata::utility::string_t ret = print_double(zero, 1);
#ifdef WIN32
	VERIFY_ARE_EQUAL(ret, U("1234567891234567.7"));
#else
    VERIFY_ARE_EQUAL(ret, U("1234567891234567.8"));
#endif
}

TEST(print_double_large_double_input_3)
{
	double zero = -12345.7674567611117999;
	::odata::utility::string_t ret = print_double(zero, 12);
	VERIFY_ARE_EQUAL(ret, U("-12345.767456761112"));
}

}

}}}