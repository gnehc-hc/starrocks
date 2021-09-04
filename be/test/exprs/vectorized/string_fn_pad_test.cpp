// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.
#include <gtest/gtest.h>

#include <random>

#include "butil/time.h"
#include "exprs/vectorized/mock_vectorized_expr.h"
#include "exprs/vectorized/string_functions.h"

namespace starrocks {
namespace vectorized {

typedef std::tuple<std::string, int32_t, std::string, bool, std::string, std::string> TestCaseType;
typedef std::vector<TestCaseType> TestCaseArray;
typedef std::vector<NullColumnPtr> NullColumnsType;
class StringFunctionPadTest : public ::testing::Test {};

Columns prepare_data(size_t num_rows) {
    auto str_column = BinaryColumn::create();
    auto len_column = Int32Column::create();
    auto fill_column = BinaryColumn::create();

    const auto times = num_rows / 4;
    const auto rest = num_rows % 4;

    for (int i = 0; i < times; ++i) {
        str_column->append(std::string(i, 'x'));
        len_column->append(65535 + i);
        fill_column->append(std::string(times - i - 1, 'y'));

        str_column->append(std::string(1, 'x'));
        len_column->append(i);
        fill_column->append("");

        str_column->append("");
        len_column->append(i);
        fill_column->append(std::string(i, 'x'));

        str_column->append(std::string(i, 'x'));
        len_column->append(-i);
        fill_column->append(std::string(i, 'x'));
    }

    for (auto i = 0; i < rest; ++i) {
        str_column->append(std::string(1, 'x'));
        len_column->append(65535);
        fill_column->append(std::string(1, 'y'));
    }
    return {str_column, len_column, fill_column};
}

void test_pad(int num_rows, TestCaseArray const& cases, NullColumnsType null_columns, PadState* state) {
    const auto cases_size = cases.size();
    ASSERT_TRUE(cases_size <= num_rows);
    const auto prepare_size = num_rows - cases_size;
    auto columns = prepare_data(prepare_size);
    auto str_column = down_cast<BinaryColumn*>(columns[0].get());
    auto len_column = down_cast<Int32Column*>(columns[1].get());
    auto fill_column = down_cast<BinaryColumn*>(columns[2].get());
    auto result_nulls = NullColumn::create();
    for (auto i = 0; i < prepare_size; ++i) {
        result_nulls->append(0);
    }

    for (auto& c : cases) {
        auto str = std::get<0>(c);
        auto len = std::get<1>(c);
        auto fill = std::get<2>(c);
        auto is_null = std::get<3>(c);
        str_column->append(str);
        len_column->append(len);
        fill_column->append(fill);
        result_nulls->append(is_null);
    }

    ASSERT_EQ(null_columns.size(), 3);
    for (auto i = 0; i < 3; ++i) {
        if (null_columns[i].get() == nullptr) {
            continue;
        }
        columns[i] = (NullableColumn::create(columns[i], null_columns[i]));
        result_nulls = FunctionHelper::union_null_column(result_nulls, null_columns[i]);
    }
    std::shared_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    if (state != nullptr) {
        ctx->set_function_state(FunctionContext::FRAGMENT_LOCAL, state);
    }

    ColumnPtr lpad_result = StringFunctions::lpad(ctx.get(), columns);
    ColumnPtr rpad_result = StringFunctions::rpad(ctx.get(), columns);

    ASSERT_EQ(lpad_result->size(), num_rows);
    ASSERT_EQ(rpad_result->size(), num_rows);
    auto& nulls = result_nulls->get_data();
    auto start_row = num_rows - cases_size;

    for (auto i = start_row; i < num_rows; ++i) {
        auto expect_lpad = std::get<4>(cases[i - start_row]);
        auto expect_rpad = std::get<5>(cases[i - start_row]);
        auto actual_lpad = lpad_result->get(i);
        auto actual_rpad = rpad_result->get(i);
        if (nulls[i] == 1) {
            ASSERT_TRUE(actual_lpad.is_null());
            ASSERT_TRUE(actual_rpad.is_null());
        } else {
            ASSERT_FALSE(actual_lpad.is_null());
            ASSERT_FALSE(actual_rpad.is_null());
            ASSERT_EQ(actual_lpad.get_slice().to_string(), expect_lpad);
            ASSERT_EQ(actual_rpad.get_slice().to_string(), expect_rpad);
        }
    }
}

NullColumnPtr create_null_column(size_t n, size_t m, size_t r) {
    auto nulls = NullColumn::create();
    for (auto i = 0; i < n; ++i) {
        nulls->append(i / m == r);
    }
    return nulls;
}

TEST_F(StringFunctionPadTest, padNotConstASCIITest) {
    std::string x65535(65535, 'x');
    std::string x65531(65531, 'x');
    TestCaseArray cases = {
            {"test", 10, "123", false, "123123test", "test123123"},
            {"test", 65536, "123", true, "", ""},
            {"test", -65536, "123", true, "", ""},
            {"test", -1, "123", true, "", ""},
            {"test", 0, "x", false, "", ""},
            {"test", 10, "", false, "test", "test"},
            {"test", 65535, "x", false, x65531 + "test", "test" + x65531},
            {"test", 65535, x65531, false, x65531 + "test", "test" + x65531},
            {x65535, 65535, "y", false, x65535, x65535},
            {"", 10, "123", false, "1231231231", "1231231231"},
            {"test", 4, "abcde", false, "test", "test"},
            {"test", 5, "abcde", false, "atest", "testa"},
            {"test", 9, "abcde", false, "abcdetest", "testabcde"},
            {"test", 10, "abcde", false, "abcdeatest", "testabcdea"},
            {"test", 14, "abcde", false, "abcdeabcdetest", "testabcdeabcde"},
            {"test", 15, "abcde", false, "abcdeabcdeatest", "testabcdeabcdea"},
    };
    const auto num_rows = cases.size();
    auto nulls_7_3 = create_null_column(num_rows, 7, 3);
    auto nulls_11_1 = create_null_column(num_rows, 11, 1);
    auto nulls_17_13 = create_null_column(num_rows, 17, 13);
    test_pad(num_rows, cases, {nullptr, nullptr, nullptr}, nullptr);
    test_pad(num_rows, cases, {nulls_7_3, nullptr, nullptr}, nullptr);
    test_pad(num_rows, cases, {nullptr, nulls_11_1, nullptr}, nullptr);
    test_pad(num_rows, cases, {nullptr, nullptr, nulls_17_13}, nullptr);
    test_pad(num_rows, cases, {nulls_7_3, nulls_11_1, nullptr}, nullptr);
    test_pad(num_rows, cases, {nulls_7_3, nullptr, nulls_17_13}, nullptr);
    test_pad(num_rows, cases, {nullptr, nulls_11_1, nulls_17_13}, nullptr);
    test_pad(num_rows, cases, {nulls_7_3, nulls_11_1, nulls_17_13}, nullptr);
}
std::string repeat(size_t n, std::string s) {
    std::string dst;
    dst.reserve(s.size() * n);
    for (auto i = 0; i < n; ++i) {
        dst.append(s);
    }
    return dst;
}

TEST_F(StringFunctionPadTest, padNotConstUTF8Test) {
    std::string zi21845 = repeat(21845, "子");
    std::string zi21841 = repeat(21841, "子");
    std::string x65535(65535, 'x');
    std::string x65531(65531, 'x');
    TestCaseArray cases = {
            {"test", 10, "123", false, "123123test", "test123123"},
            {"test", 65536, "123", true, "", ""},
            {"test", -65536, "123", true, "", ""},
            {"test", -1, "123", true, "", ""},
            {"test", 0, "x", false, "", ""},
            {"test", 10, "", false, "test", "test"},
            {"test", 65535, "x", false, x65531 + "test", "test" + x65531},
            {"test", 65535, x65531, false, x65531 + "test", "test" + x65531},
            {x65535, 65535, "y", false, x65535, x65535},
            {"", 10, "123", false, "1231231231", "1231231231"},
            {"test", 4, "abcde", false, "test", "test"},
            {"test", 5, "abcde", false, "atest", "testa"},
            {"test", 9, "abcde", false, "abcdetest", "testabcde"},
            {"test", 10, "abcde", false, "abcdeatest", "testabcdea"},
            {"test", 14, "abcde", false, "abcdeabcdetest", "testabcdeabcde"},
            {"test", 15, "abcde", false, "abcdeabcdeatest", "testabcdeabcdea"},
            {"博学笃志", 10, "壹贰叁", false, "壹贰叁壹贰叁博学笃志", "博学笃志壹贰叁壹贰叁"},
            {"博学笃志", 0, "子", false, "", ""},
            {"博学笃志", 10, "", false, "博学笃志", "博学笃志"},

            {"博学笃志", 21845, "子", false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {"博学笃志", 21845, zi21845, false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {zi21845, 21845, "丑", false, zi21845, zi21845},

            {"", 10, "壹贰叁", false, "壹贰叁壹贰叁壹贰叁壹", "壹贰叁壹贰叁壹贰叁壹"},
            {"博学笃志", 4, "甲乙丙丁戊", false, "博学笃志", "博学笃志"},
            {"博学笃志", 5, "甲乙丙丁戊", false, "甲博学笃志", "博学笃志甲"},
            {"博学笃志", 9, "甲乙丙丁戊", false, "甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊"},
            {"博学笃志", 10, "甲乙丙丁戊", false, "甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲"},
            {"博学笃志", 14, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊"},
            {"博学笃志", 15, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊甲"},
    };
    auto nulls_7_3 = create_null_column(4096, 7, 3);
    auto nulls_11_1 = create_null_column(4096, 11, 1);
    auto nulls_17_13 = create_null_column(4096, 17, 13);
    test_pad(4096, cases, {nullptr, nullptr, nullptr}, nullptr);
    test_pad(4096, cases, {nulls_7_3, nullptr, nullptr}, nullptr);
    test_pad(4096, cases, {nullptr, nulls_11_1, nullptr}, nullptr);
    test_pad(4096, cases, {nullptr, nullptr, nulls_17_13}, nullptr);
    test_pad(4096, cases, {nulls_7_3, nulls_11_1, nullptr}, nullptr);
    test_pad(4096, cases, {nulls_7_3, nullptr, nulls_17_13}, nullptr);
    test_pad(4096, cases, {nullptr, nulls_11_1, nulls_17_13}, nullptr);
    test_pad(4096, cases, {nulls_7_3, nulls_11_1, nulls_17_13}, nullptr);
}

void test_const_pad(size_t num_rows, TestCaseType& c) {
    ASSERT_TRUE(num_rows > 0);
    auto columns = prepare_data(num_rows - 1);
    auto [str, len, fill, is_null, lpad_expect, rpad_expect] = c;
    auto str_column = down_cast<BinaryColumn*>(columns[0].get());
    auto len_column = down_cast<Int32Column*>(columns[1].get());
    auto fill_columns = down_cast<BinaryColumn*>(columns[2].get());
    str_column->append(str);
    len_column->append(len);
    fill_columns->append(fill);
    auto state = std::make_unique<PadState>();
    std::shared_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    auto const_pad_col = ColumnHelper::create_const_column<TYPE_VARCHAR>(Slice{fill.data(), fill.size()}, 1);
    ctx->impl()->set_constant_columns({nullptr, nullptr, const_pad_col});
    columns[2] = const_pad_col;
    StringFunctions::pad_prepare(ctx.get(), FunctionContext::FRAGMENT_LOCAL);
    auto lpad_result = StringFunctions::lpad(ctx.get(), columns);
    auto rpad_result = StringFunctions::rpad(ctx.get(), columns);
    StringFunctions::pad_close(ctx.get(), FunctionContext::FRAGMENT_LOCAL);
    ASSERT_EQ(lpad_result->size(), num_rows);
    ASSERT_EQ(rpad_result->size(), num_rows);
    auto lpad_actual = lpad_result->get(num_rows - 1);
    auto rpad_actual = rpad_result->get(num_rows - 1);
    if (is_null) {
        ASSERT_TRUE(lpad_actual.is_null());
        ASSERT_TRUE(rpad_actual.is_null());
    } else {
        ASSERT_FALSE(lpad_actual.is_null());
        ASSERT_FALSE(rpad_actual.is_null());
        ASSERT_EQ(lpad_actual.get_slice().to_string(), lpad_expect);
        ASSERT_EQ(rpad_actual.get_slice().to_string(), rpad_expect);
    }
}

TEST_F(StringFunctionPadTest, padConstPadTest) {
    std::string zi21845 = repeat(21845, "子");
    std::string zi21841 = repeat(21841, "子");
    std::string x65535(65535, 'x');
    std::string x65531(65531, 'x');
    TestCaseArray cases = {
            {"test", 10, "123", false, "123123test", "test123123"},
            {"test", 65536, "123", true, "", ""},
            {"test", -65536, "123", true, "", ""},
            {"test", -1, "123", true, "", ""},
            {"test", 0, "x", false, "", ""},
            {"test", 10, "", false, "test", "test"},
            {"test", 65535, "x", false, x65531 + "test", "test" + x65531},
            {"test", 65535, x65531, false, x65531 + "test", "test" + x65531},
            {x65535, 65535, "y", false, x65535, x65535},
            {"", 10, "123", false, "1231231231", "1231231231"},
            {"test", 4, "abcde", false, "test", "test"},
            {"test", 5, "abcde", false, "atest", "testa"},
            {"test", 9, "abcde", false, "abcdetest", "testabcde"},
            {"test", 10, "abcde", false, "abcdeatest", "testabcdea"},
            {"test", 14, "abcde", false, "abcdeabcdetest", "testabcdeabcde"},
            {"test", 15, "abcde", false, "abcdeabcdeatest", "testabcdeabcdea"},
            {"博学笃志", 10, "壹贰叁", false, "壹贰叁壹贰叁博学笃志", "博学笃志壹贰叁壹贰叁"},
            {"博学笃志", 0, "子", false, "", ""},
            {"博学笃志", 10, "", false, "博学笃志", "博学笃志"},

            {"博学笃志", 21845, "子", false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {"博学笃志", 21845, zi21845, false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {zi21845, 21845, "丑", false, zi21845, zi21845},

            {"", 10, "壹贰叁", false, "壹贰叁壹贰叁壹贰叁壹", "壹贰叁壹贰叁壹贰叁壹"},
            {"博学笃志", 4, "甲乙丙丁戊", false, "博学笃志", "博学笃志"},
            {"博学笃志", 5, "甲乙丙丁戊", false, "甲博学笃志", "博学笃志甲"},
            {"博学笃志", 9, "甲乙丙丁戊", false, "甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊"},
            {"博学笃志", 10, "甲乙丙丁戊", false, "甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲"},
            {"博学笃志", 14, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊"},
            {"博学笃志", 15, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊甲"},
    };

    for (auto& c : cases) {
        test_const_pad(1, c);
        test_const_pad(20, c);
        test_const_pad(200, c);
    }
}

void test_const_len_and_pad(size_t num_rows, TestCaseType& c) {
    ASSERT_TRUE(num_rows > 0);
    auto columns = prepare_data(num_rows - 1);
    auto [str, len, fill, is_null, lpad_expect, rpad_expect] = c;
    auto str_column = down_cast<BinaryColumn*>(columns[0].get());
    auto len_column = down_cast<Int32Column*>(columns[1].get());
    auto fill_columns = down_cast<BinaryColumn*>(columns[2].get());
    str_column->append(str);
    len_column->append(len);
    fill_columns->append(fill);
    auto state = std::make_unique<PadState>();
    std::shared_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    auto const_len_col = ColumnHelper::create_const_column<TYPE_INT>(len, 1);
    auto const_pad_col = ColumnHelper::create_const_column<TYPE_VARCHAR>(Slice{fill.data(), fill.size()}, 1);
    ctx->impl()->set_constant_columns({nullptr, const_len_col, const_pad_col});
    columns[1] = const_len_col;
    columns[2] = const_pad_col;
    StringFunctions::pad_prepare(ctx.get(), FunctionContext::FRAGMENT_LOCAL);
    auto lpad_result = StringFunctions::lpad(ctx.get(), columns);
    auto rpad_result = StringFunctions::rpad(ctx.get(), columns);
    StringFunctions::pad_close(ctx.get(), FunctionContext::FRAGMENT_LOCAL);
    if (lpad_result->is_constant()) {
        ASSERT_EQ(lpad_result->size(), 1);
        ASSERT_EQ(rpad_result->size(), 1);
        if (is_null) {
            ASSERT_TRUE(lpad_result->only_null());
            ASSERT_TRUE(rpad_result->only_null());
        } else {
            ASSERT_FALSE(lpad_result->only_null());
            ASSERT_FALSE(rpad_result->only_null());
            auto lslice = ColumnHelper::get_const_value<TYPE_VARCHAR>(lpad_result);
            auto rslice = ColumnHelper::get_const_value<TYPE_VARCHAR>(rpad_result);
            ASSERT_EQ(lslice.size, 0);
            ASSERT_EQ(rslice.size, 0);
        }
    } else {
        ASSERT_EQ(lpad_result->size(), num_rows);
        ASSERT_EQ(rpad_result->size(), num_rows);
        auto lpad_actual = lpad_result->get(num_rows - 1);
        auto rpad_actual = rpad_result->get(num_rows - 1);
        if (is_null) {
            ASSERT_TRUE(lpad_actual.is_null());
            ASSERT_TRUE(rpad_actual.is_null());
        } else {
            ASSERT_FALSE(lpad_actual.is_null());
            ASSERT_FALSE(rpad_actual.is_null());
            ASSERT_EQ(lpad_actual.get_slice().to_string(), lpad_expect);
            ASSERT_EQ(rpad_actual.get_slice().to_string(), rpad_expect);
        }
    }
}

TEST_F(StringFunctionPadTest, padConstLenAndPadTest) {
    std::string zi21845 = repeat(21845, "子");
    std::string zi21841 = repeat(21841, "子");
    std::string x65535(65535, 'x');
    std::string x65531(65531, 'x');
    TestCaseArray cases = {
            {"test", 10, "123", false, "123123test", "test123123"},
            {"test", 65536, "123", true, "", ""},
            {"test", -65536, "123", true, "", ""},
            {"test", -1, "123", true, "", ""},
            {"test", 0, "x", false, "", ""},
            {"test", 10, "", false, "test", "test"},
            {"test", 65535, "x", false, x65531 + "test", "test" + x65531},
            {"test", 65535, x65531, false, x65531 + "test", "test" + x65531},
            {x65535, 65535, "y", false, x65535, x65535},
            {"", 10, "123", false, "1231231231", "1231231231"},
            {"test", 4, "abcde", false, "test", "test"},
            {"test", 5, "abcde", false, "atest", "testa"},
            {"test", 9, "abcde", false, "abcdetest", "testabcde"},
            {"test", 10, "abcde", false, "abcdeatest", "testabcdea"},
            {"test", 14, "abcde", false, "abcdeabcdetest", "testabcdeabcde"},
            {"test", 15, "abcde", false, "abcdeabcdeatest", "testabcdeabcdea"},
            {"博学笃志", 10, "壹贰叁", false, "壹贰叁壹贰叁博学笃志", "博学笃志壹贰叁壹贰叁"},
            {"博学笃志", 0, "子", false, "", ""},
            {"博学笃志", 10, "", false, "博学笃志", "博学笃志"},

            {"博学笃志", 21845, "子", false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {"博学笃志", 21845, zi21845, false, zi21841 + "博学笃志", "博学笃志" + zi21841},
            {zi21845, 21845, "丑", false, zi21845, zi21845},

            {"", 10, "壹贰叁", false, "壹贰叁壹贰叁壹贰叁壹", "壹贰叁壹贰叁壹贰叁壹"},
            {"博学笃志", 4, "甲乙丙丁戊", false, "博学笃志", "博学笃志"},
            {"博学笃志", 5, "甲乙丙丁戊", false, "甲博学笃志", "博学笃志甲"},
            {"博学笃志", 9, "甲乙丙丁戊", false, "甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊"},
            {"博学笃志", 10, "甲乙丙丁戊", false, "甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲"},
            {"博学笃志", 14, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊"},
            {"博学笃志", 15, "甲乙丙丁戊", false, "甲乙丙丁戊甲乙丙丁戊甲博学笃志", "博学笃志甲乙丙丁戊甲乙丙丁戊甲"},
    };

    for (auto& c : cases) {
        test_const_len_and_pad(1, c);
        test_const_len_and_pad(20, c);
        test_const_len_and_pad(200, c);
    }
}

TEST_F(StringFunctionPadTest, lpadNullTest) {
    std::unique_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    Columns columns;
    auto str = BinaryColumn::create();
    auto len = Int32Column::create();
    auto fill = BinaryColumn::create();
    auto null = NullColumn::create();

    str->append("test");
    len->append(8);
    fill->append("123");
    null->append(false);

    str->append("test");
    len->append(2);
    fill->append("123");
    null->append(true);

    columns.emplace_back(str);
    columns.emplace_back(len);
    columns.emplace_back(NullableColumn::create(fill, null));

    ColumnPtr result = StringFunctions::lpad(ctx.get(), columns);
    ASSERT_EQ(2, result->size());
    ASSERT_TRUE(result->is_nullable());
    auto v = ColumnHelper::cast_to<TYPE_VARCHAR>(ColumnHelper::as_raw_column<NullableColumn>(result)->data_column());

    ASSERT_FALSE(result->is_null(0));
    ASSERT_EQ("1231test", v->get_data()[0].to_string());

    ASSERT_TRUE(result->is_null(1));
    ASSERT_EQ("", v->get_data()[1].to_string());
}

TEST_F(StringFunctionPadTest, rpadTest) {
    std::unique_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    Columns columns;
    auto str = BinaryColumn::create();
    auto len = Int32Column::create();
    auto fill = BinaryColumn::create();

    str->append("test");
    len->append(11);
    fill->append("123");

    str->append("test");
    len->append(0);
    fill->append("123");

    columns.emplace_back(str);
    columns.emplace_back(len);
    columns.emplace_back(fill);

    ColumnPtr result = StringFunctions::rpad(ctx.get(), columns);
    ASSERT_EQ(2, result->size());

    auto v = ColumnHelper::cast_to<TYPE_VARCHAR>(result);

    ASSERT_EQ("test1231231", v->get_data()[0].to_string());
    ASSERT_EQ("", v->get_data()[1].to_string());
}

TEST_F(StringFunctionPadTest, rpadChineseTest) {
    std::unique_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    Columns columns;
    auto str = BinaryColumn::create();
    auto len = Int32Column::create();
    auto fill = BinaryColumn::create();

    str->append("我是中文字符串");
    len->append(15);
    fill->append("不，我不是");

    str->append("我是中文字符串");
    len->append(2);
    fill->append("不，我没有");

    columns.emplace_back(str);
    columns.emplace_back(len);
    columns.emplace_back(fill);

    ColumnPtr result = StringFunctions::rpad(ctx.get(), columns);
    ASSERT_EQ(2, result->size());

    auto v = ColumnHelper::cast_to<TYPE_VARCHAR>(result);

    ASSERT_EQ(Slice("我是中文字符串不，我不是不，我"), v->get_data()[0]);
    ASSERT_EQ(Slice("我是"), v->get_data()[1]);
}

TEST_F(StringFunctionPadTest, rpadConstTest) {
    std::unique_ptr<FunctionContext> ctx(FunctionContext::create_test_context());
    Columns columns;
    auto str = BinaryColumn::create();
    auto len = Int32Column::create();
    auto fill = BinaryColumn::create();

    str->append("test");
    len->append(8);
    fill->append("123");

    str->append("test");
    len->append(-1);

    columns.emplace_back(str);
    columns.emplace_back(len);
    columns.emplace_back(ConstColumn::create(fill));

    ColumnPtr result = StringFunctions::rpad(ctx.get(), columns);
    ASSERT_EQ(2, result->size());
    ASSERT_TRUE(result->is_nullable());
    auto v = ColumnHelper::cast_to<TYPE_VARCHAR>(ColumnHelper::as_raw_column<NullableColumn>(result)->data_column());

    ASSERT_FALSE(result->is_null(0));
    ASSERT_EQ("test1231", v->get_data()[0].to_string());

    ASSERT_TRUE(result->is_null(1));
    ASSERT_EQ("", v->get_data()[1].to_string());
}

} // namespace vectorized
} // namespace starrocks
