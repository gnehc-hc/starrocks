// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/test/java/org/apache/doris/analysis/ShowVariablesStmtTest.java

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

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

public class ShowVariablesStmtTest {
    @Test
    public void testNormal() {
        ShowVariablesStmt stmt = new ShowVariablesStmt(null, null);
        stmt.analyze((Analyzer) null);
        assertEquals("SHOW DEFAULT VARIABLES", stmt.toString());
        assertEquals(2, stmt.getMetaData().getColumnCount());
        assertEquals("Variable_name", stmt.getMetaData().getColumn(0).getName());
        assertEquals("Value", stmt.getMetaData().getColumn(1).getName());
        assertNull(stmt.getPattern());
        assertEquals(SetType.DEFAULT, stmt.getType());

        stmt = new ShowVariablesStmt(SetType.GLOBAL, "abc");
        stmt.analyze((Analyzer) null);
        assertEquals("SHOW GLOBAL VARIABLES LIKE 'abc'", stmt.toString());
        assertEquals(2, stmt.getMetaData().getColumnCount());
        assertEquals("Variable_name", stmt.getMetaData().getColumn(0).getName());
        assertEquals("Value", stmt.getMetaData().getColumn(1).getName());
        assertEquals("abc", stmt.getPattern());
        assertEquals(SetType.GLOBAL, stmt.getType());
    }
}