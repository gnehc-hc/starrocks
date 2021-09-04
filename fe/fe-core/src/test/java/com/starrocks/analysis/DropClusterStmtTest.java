// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/test/java/org/apache/doris/analysis/DropClusterStmtTest.java

// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

package com.starrocks.analysis;

import com.starrocks.common.AnalysisException;
import com.starrocks.common.Config;
import com.starrocks.common.UserException;
import com.starrocks.mysql.privilege.Auth;
import com.starrocks.mysql.privilege.PrivPredicate;
import com.starrocks.qe.ConnectContext;
import mockit.Expectations;
import mockit.Mocked;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class DropClusterStmtTest {

    private static Analyzer analyzer;

    @Mocked
    private Auth auth;

    @Before
    public void setUp() {
        Config.disable_cluster_feature = false;
        analyzer = AccessTestUtil.fetchAdminAnalyzer(true);

        new Expectations() {
            {
                auth.checkGlobalPriv((ConnectContext) any, (PrivPredicate) any);
                minTimes = 0;
                result = true;

                auth.checkDbPriv((ConnectContext) any, anyString, (PrivPredicate) any);
                minTimes = 0;
                result = true;

                auth.checkTblPriv((ConnectContext) any, anyString, anyString, (PrivPredicate) any);
                minTimes = 0;
                result = true;
            }
        };
    }

    @Test
    public void testNormal() throws UserException, AnalysisException {
        final DropClusterStmt stmt = new DropClusterStmt(true, "testCluster");

        stmt.analyze(analyzer);
        Assert.assertEquals("testCluster", stmt.getName());
        Assert.assertEquals("DROP CLUSTER testCluster", stmt.toSql());
    }

    @Test(expected = AnalysisException.class)
    public void testFailed() throws UserException, AnalysisException {
        DropClusterStmt stmt = new DropClusterStmt(false, "");

        stmt.analyze(analyzer);
        Assert.fail("no exception");
    }
}
