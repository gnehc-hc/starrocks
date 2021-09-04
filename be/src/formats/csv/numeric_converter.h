// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#pragma once

#include "formats/csv/converter.h"

namespace starrocks::vectorized::csv {

template <typename T>
class NumericConverter final : public Converter {
    static_assert(std::is_arithmetic_v<T> && !std::is_floating_point_v<T>, "Not numeric type");

public:
    using DataType = T;

    Status write_string(OutputStream* os, const Column& column, size_t row_num, const Options& options) const override;
    Status write_quoted_string(OutputStream* os, const Column& column, size_t row_num,
                               const Options& options) const override;
    bool read_string(Column* column, Slice s, const Options& options) const override;
    bool read_quoted_string(Column* column, Slice s, const Options& options) const override;
};

} // namespace starrocks::vectorized::csv
