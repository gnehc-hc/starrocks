// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.
package com.starrocks.sql.optimizer.operator.physical;

import com.starrocks.sql.optimizer.OptExpression;
import com.starrocks.sql.optimizer.OptExpressionVisitor;
import com.starrocks.sql.optimizer.base.ColumnRefSet;
import com.starrocks.sql.optimizer.operator.OperatorType;
import com.starrocks.sql.optimizer.operator.OperatorVisitor;
import com.starrocks.sql.optimizer.operator.scalar.ColumnRefOperator;
import com.starrocks.sql.optimizer.operator.scalar.ScalarOperator;

import java.util.List;
import java.util.Objects;

public class PhysicalValues extends PhysicalOperator {
    private final List<ColumnRefOperator> columnRefSet;
    private final List<List<ScalarOperator>> rows;

    public PhysicalValues(List<ColumnRefOperator> columnRefSet, List<List<ScalarOperator>> rows) {
        super(OperatorType.PHYSICAL_VALUES);
        this.columnRefSet = columnRefSet;
        this.rows = rows;
    }

    public List<ColumnRefOperator> getColumnRefSet() {
        return columnRefSet;
    }

    public List<List<ScalarOperator>> getRows() {
        return rows;
    }

    @Override
    public <R, C> R accept(OperatorVisitor<R, C> visitor, C context) {
        return visitor.visitPhysicalValues(this, context);
    }

    @Override
    public <R, C> R accept(OptExpressionVisitor<R, C> visitor, OptExpression optExpression, C context) {
        return visitor.visitPhysicalValues(optExpression, context);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        PhysicalValues empty = (PhysicalValues) o;
        return Objects.equals(columnRefSet, empty.columnRefSet);
    }

    @Override
    public int hashCode() {
        return Objects.hash(columnRefSet);
    }

    @Override
    public ColumnRefSet getUsedColumns() {
        ColumnRefSet set = super.getUsedColumns();
        rows.forEach(r -> r.forEach(s -> set.union(s.getUsedColumns())));
        return set;
    }
}
