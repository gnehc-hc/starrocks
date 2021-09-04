// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "exprs/vectorized/is_null_predicate.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <memory>

#include "column/column_helper.h"
#include "column/fixed_length_column.h"
#include "exprs/vectorized/mock_vectorized_expr.h"

namespace starrocks {
namespace vectorized {

class VectorizedIsNullExprTest : public ::testing::Test {
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

TEST_F(VectorizedIsNullExprTest, isNullTest) {
    expr_node.fn.name.function_name = "is_null_pred";
    auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

    MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    col1.all_null = true;

    expr->_children.push_back(&col1);

    ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
    ASSERT_TRUE(ptr->is_numeric());

    auto v = ColumnHelper::cast_to_raw<TYPE_BOOLEAN>(ptr);
    for (int j = 0; j < ptr->size(); ++j) {
        ASSERT_TRUE(v->get_data()[j]);
    }
}

TEST_F(VectorizedIsNullExprTest, onlyNullTest) {
    expr_node.fn.name.function_name = "is_null_pred";
    auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

    MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
    col1.only_null = true;

    expr->_children.push_back(&col1);

    {
        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_constant());

        auto v = ColumnHelper::get_const_value<TYPE_BOOLEAN>(ptr);
        ASSERT_TRUE(v);
    }
}

TEST_F(VectorizedIsNullExprTest, isNullFalseTest) {
    expr_node.fn.name.function_name = "is_null_pred";
    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BOOLEAN>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            if (j % 2) {
                ASSERT_TRUE(v->get_data()[j]);
            } else {
                ASSERT_FALSE(v->get_data()[j]);
            }
        }
    }

    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_constant());

        auto v = ColumnHelper::get_const_value<TYPE_BOOLEAN>(ptr);
        ASSERT_FALSE(v);
    }
}

TEST_F(VectorizedIsNullExprTest, isNotNullTest) {
    expr_node.fn.name.function_name = "is_not_null_pred";

    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
        col1.all_null = true;

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BOOLEAN>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_FALSE(v->get_data()[j]);
        }
    }

    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);
        col1.only_null = true;

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_constant());

        auto v = ColumnHelper::get_const_value<TYPE_BOOLEAN>(ptr);
        ASSERT_FALSE(v);
    }

    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockNullVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_numeric());

        auto v = ColumnHelper::cast_to_raw<TYPE_BOOLEAN>(ptr);
        for (int j = 0; j < ptr->size(); ++j) {
            if (j % 2 == 0) {
                ASSERT_TRUE(v->get_data()[j]);
            } else {
                ASSERT_FALSE(v->get_data()[j]);
            }
        }
    }

    {
        auto expr = std::unique_ptr<Expr>(VectorizedIsNullPredicateFactory::from_thrift(expr_node));

        MockVectorizedExpr<TYPE_BIGINT> col1(expr_node, 10, 10);

        expr->_children.push_back(&col1);

        Chunk chunk;
        ColumnPtr ptr = expr->evaluate(nullptr, &chunk);
        ASSERT_TRUE(ptr->is_constant());

        auto v = ColumnHelper::get_const_value<TYPE_BOOLEAN>(ptr);
        ASSERT_TRUE(v);
    }
}
} // namespace vectorized
} // namespace starrocks
