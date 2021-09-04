// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

package com.starrocks.sql.plan;

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class AggTableTest extends PlanTestBase {
    public static void assertTestAggOFF(String sql, String reason) {
        if (!sql.contains("TABLE: test_agg")) {
            fail();
        }
        String result = sql.split("TABLE: test_agg")[1].trim().split("\n")[0].trim();
        assertEquals(result, "PREAGGREGATION: OFF. Reason: " + reason);
    }

    public static void assertTestAggON(String sql) {
        if (!sql.contains("TABLE: test_agg")) {
            fail();
        }
        String result = sql.split("TABLE: test_agg")[1].trim().split("\n")[0].trim();
        assertEquals(result, "PREAGGREGATION: ON");
    }

    @Test
    public void testSingleAgg1() throws Exception {
        String sql = getFragmentPlan("select * from test_agg");
        assertTestAggOFF(sql, "None aggregate function");
    }

    @Test
    public void testSingleAgg2() throws Exception {
        String sql = getFragmentPlan("select MAX(k1), MIN(k2) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg3() throws Exception {
        String sql = getFragmentPlan("select MAX(v1), MIN(v2), SUM(v3) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg4() throws Exception {
        String sql = getFragmentPlan("select BITMAP_UNION(b1), hll_union(h1) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg5() throws Exception {
        String sql = getFragmentPlan("select SUM(k1), MIN(k2) from test_agg");
        assertTestAggOFF(sql, "The key column don't support aggregate function: SUM");
    }

    @Test
    public void testSingleAgg6() throws Exception {
        String sql = getFragmentPlan("select SUM(v3 + 1) from test_agg");
        assertTestAggOFF(sql, "The parameter of aggregate function isn't value column or CAST/CASE-WHEN expression");
    }

    @Test
    public void testSingleAgg7() throws Exception {
        String sql = getFragmentPlan("select SUM(cast(v3 as float)) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg8() throws Exception {
        String sql = getFragmentPlan("select SUM(cast(v4 as int)) from test_agg");
        assertTestAggOFF(sql, "The parameter of aggregate function isn't numeric type");
    }

    @Test
    public void testSingleAgg9() throws Exception {
        String sql = getFragmentPlan("select SUM(case k1 when k2 then v3 when k3 then v5 else v6 end) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg10() throws Exception {
        String sql = getFragmentPlan("select SUM(case k1 when k2 then v3 when 2 then v5 else v6 end) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg11() throws Exception {
        String sql = getFragmentPlan("select SUM(case k1 when k2 then v3 when k3 then v2 else v6 end) from test_agg");
        assertTestAggOFF(sql, "Aggregate Operator not match: SUM <--> MIN");
    }

    @Test
    public void testSingleAgg12() throws Exception {
        String sql = getFragmentPlan("select SUM(v3) from test_agg where k1 = 1");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg13() throws Exception {
        String sql = getFragmentPlan("select SUM(v3) from test_agg where v1 = 1");
        assertTestAggOFF(sql, "Predicates include the value column");
    }

    @Test
    public void testSingleAgg14() throws Exception {
        String sql = getFragmentPlan("select SUM(v3) from test_agg group by k1");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg15() throws Exception {
        String sql = getFragmentPlan("select SUM(v3) from test_agg group by v1");
        assertTestAggOFF(sql, "Group columns isn't Key column");
    }

    @Test
    public void testSingleAgg16() throws Exception {
        String sql = getFragmentPlan("select SUM(v3) from test_agg, t1 group by k1");
        assertTestAggOFF(sql, "Has Join");
    }

    @Test
    public void testSingleAgg17() throws Exception {
        String sql = getFragmentPlan("select NDV(k2) from test_agg");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg18() throws Exception {
        String sql = getFragmentPlan("select HLL_UNION_AGG(h1) from test_agg group by k1");
        assertTestAggON(sql);
    }

    @Test
    public void testSingleAgg19() throws Exception {
        String sql = getFragmentPlan("select NDV(v2) from test_agg group by k1");
        assertTestAggOFF(sql, "Aggregation function NDV just work on key column");
    }
}
