// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.
package com.starrocks.sql.plan;

import com.starrocks.analysis.DescriptorTable;
import com.starrocks.analysis.Expr;
import com.starrocks.planner.PlanFragment;
import com.starrocks.planner.PlannerContext;
import com.starrocks.planner.ScanNode;
import com.starrocks.qe.ConnectContext;
import com.starrocks.sql.optimizer.operator.scalar.ColumnRefOperator;
import com.starrocks.thrift.TExplainLevel;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ExecPlan {
    private final PlannerContext planCtx;
    private final ConnectContext connectContext;
    private final List<String> colNames;
    private final List<ScanNode> scanNodes = new ArrayList<>();
    private final List<Expr> outputExprs = new ArrayList<>();
    private final DescriptorTable descTbl = new DescriptorTable();
    private final Map<ColumnRefOperator, Expr> colRefToExpr = new HashMap<>();
    private final ArrayList<PlanFragment> fragments = new ArrayList<>();

    public ExecPlan(PlannerContext planCtx, ConnectContext connectContext, List<String> colNames) {
        this.planCtx = planCtx;
        this.connectContext = connectContext;
        this.colNames = colNames;
    }

    public ConnectContext getConnectContext() {
        return connectContext;
    }

    public List<ScanNode> getScanNodes() {
        return scanNodes;
    }

    public List<Expr> getOutputExprs() {
        return outputExprs;
    }

    public ArrayList<PlanFragment> getFragments() {
        return fragments;
    }

    public DescriptorTable getDescTbl() {
        return descTbl;
    }

    public List<String> getColNames() {
        return colNames;
    }

    public PlannerContext getPlanCtx() {
        return planCtx;
    }

    public Map<ColumnRefOperator, Expr> getColRefToExpr() {
        return colRefToExpr;
    }

    public String getExplainString(TExplainLevel level) {
        StringBuilder str = new StringBuilder();
        for (int i = 0; i < fragments.size(); ++i) {
            PlanFragment fragment = fragments.get(i);
            if (i > 0) {
                // a blank line between plan fragments
                str.append("\n");
            }
            if (level.equals(TExplainLevel.NORMAL)) {
                str.append("PLAN FRAGMENT ").append(i).append("\n");
                str.append(fragment.getExplainString(TExplainLevel.NORMAL));
            } else if (level.equals(TExplainLevel.COSTS)) {
                str.append("PLAN FRAGMENT ").append(i).append("(").append(fragment.getFragmentId()).append(")\n");
                str.append(fragment.getCostExplain());
            } else {
                str.append("PLAN FRAGMENT ").append(i).append("(").append(fragment.getFragmentId()).append(")\n");
                str.append(fragment.getVerboseExplain());
            }
        }
        return str.toString();
    }
}