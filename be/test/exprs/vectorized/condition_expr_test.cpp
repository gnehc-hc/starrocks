// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "exprs/vectorized/condition_expr.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "column/column_helper.h"
#include "column/fixed_length_column.h"
#include "exprs/vectorized/mock_vectorized_expr.h"

namespace starrocks {
namespace vectorized {

class VectorizedConditionExprTest : public ::testing::Test {
public:
    void SetUp() {
        expr_node.opcode = TExprOpcode::ADD;
        expr_node.child_type = TPrimitiveType::INT;
        expr_node.node_type = TExprNodeType::BINARY_PRED;
        expr_node.num_children = 2;
        expr_node.__isset.opcode = true;
        expr_node.__isset.child_type = true;
        expr_node.type = gen_type_desc(TPrimitiveType::BIGINT);
    }

public:
    TExprNode expr_node;
};

TEST_F(VectorizedConditionExprTest, ifNullLNotNull) {
    auto expr = std::unique_ptr<Expr>(VectorizedConditionExprFactory::create_if_null_expr(expr_node));

    MockVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_BIGINT> col2(expr_node, 10, 20);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);
    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BIGINT>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_EQ(10, v->get_data()[j]);
        }
    }
}

TEST_F(VectorizedConditionExprTest, ifNullLAllNull) {
    auto expr = std::unique_ptr<Expr>(VectorizedConditionExprFactory::create_if_null_expr(expr_node));

    MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_BIGINT> col2(expr_node, 10, 20);

    col1.all_null = true;
    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);
    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BIGINT>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_EQ(20, v->get_data()[j]);
        }
    }
}

TEST_F(VectorizedConditionExprTest, ifNull) {
    auto expr = std::unique_ptr<Expr>(VectorizedConditionExprFactory::create_if_null_expr(expr_node));

    MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_BIGINT> col2(expr_node, 10, 20);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);
    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BIGINT>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            if (j % 2 == 0) {
                ASSERT_EQ(10, v->get_data()[j]);
            } else {
                ASSERT_EQ(20, v->get_data()[j]);
            }
        }
    }
}

TEST_F(VectorizedConditionExprTest, ifNullNull) {
    auto expr = std::unique_ptr<Expr>(VectorizedConditionExprFactory::create_if_null_expr(expr_node));

    MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    MockNullVectorizedExpr<TYPE_BIGINT> col2(expr_node, 10, 20);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);
    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
        ASSERT_TRUE(ptr->is_nullable());
        ASSERT_FALSE(ptr->is_numeric());

        auto v =
                ColumnHelper::cast_to_raw<TYPE_BIGINT>(ColumnHelper::as_raw_column<NullableColumn>(ptr)->data_column());
        for (int j = 0; j < ptr->size(); ++j) {
            if (j % 2 == 0) {
                ASSERT_EQ(10, v->get_data()[j]);
            } else {
                ASSERT_TRUE(ptr->is_null(j));
                ASSERT_EQ(20, v->get_data()[j]);
            }
        }
    }
}

} // namespace vectorized
} // namespace starrocks
