// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.
package com.starrocks.sql.optimizer.operator.logical;

import com.starrocks.sql.optimizer.ExpressionContext;
import com.starrocks.sql.optimizer.OptExpression;
import com.starrocks.sql.optimizer.OptExpressionVisitor;
import com.starrocks.sql.optimizer.base.ColumnRefSet;
import com.starrocks.sql.optimizer.operator.OperatorType;
import com.starrocks.sql.optimizer.operator.OperatorVisitor;
import com.starrocks.sql.optimizer.operator.scalar.ColumnRefOperator;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Set;

public class LogicalRepeatOperator extends LogicalOperator {
    private final List<ColumnRefOperator> outputGrouping;
    private final List<Set<ColumnRefOperator>> repeatColumnRefList;
    private final List<List<Long>> groupingIds;

    public LogicalRepeatOperator(List<ColumnRefOperator> outputGrouping,
                                 List<Set<ColumnRefOperator>> repeatColumnRefList,
                                 List<List<Long>> groupingIds) {
        super(OperatorType.LOGICAL_REPEAT);
        this.outputGrouping = outputGrouping;
        this.repeatColumnRefList = repeatColumnRefList;
        this.groupingIds = groupingIds;
    }

    public List<ColumnRefOperator> getOutputGrouping() {
        return outputGrouping;
    }

    public List<Set<ColumnRefOperator>> getRepeatColumnRef() {
        return repeatColumnRefList;
    }

    public List<List<Long>> getGroupingIds() {
        return groupingIds;
    }

    @Override
    public ColumnRefSet getOutputColumns(ExpressionContext expressionContext) {
        ColumnRefSet outputColumns = new ColumnRefSet(outputGrouping);
        for (Set<ColumnRefOperator> refSets : repeatColumnRefList) {
            outputColumns.union(new ArrayList<>(refSets));
        }

        return outputColumns;
    }

    @Override
    public <R, C> R accept(OperatorVisitor<R, C> visitor, C context) {
        return visitor.visitLogicalRepeat(this, context);
    }

    @Override
    public <R, C> R accept(OptExpressionVisitor<R, C> visitor, OptExpression optExpression, C context) {
        return visitor.visitLogicalRepeat(optExpression, context);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        if (!super.equals(o)) {
            return false;
        }
        LogicalRepeatOperator that = (LogicalRepeatOperator) o;
        return Objects.equals(outputGrouping, that.outputGrouping) &&
                Objects.equals(repeatColumnRefList, that.repeatColumnRefList);
    }

    @Override
    public int hashCode() {
        return Objects.hash(super.hashCode(), outputGrouping, repeatColumnRefList);
    }
}
