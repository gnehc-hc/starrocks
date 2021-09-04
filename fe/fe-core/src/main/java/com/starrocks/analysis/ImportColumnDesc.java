// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/analysis/ImportColumnDesc.java

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

public class ImportColumnDesc {
    private String columnName;
    private Expr expr;

    public ImportColumnDesc(ImportColumnDesc other) {
        this.columnName = other.columnName;
        if (other.expr != null) {
            this.expr = other.expr.clone();
        }
    }

    public ImportColumnDesc(String column) {
        this.columnName = column;
    }

    public ImportColumnDesc(String column, Expr expr) {
        this.columnName = column;
        this.expr = expr;
    }

    public void reset(String column, Expr expr) {
        this.columnName = column;
        this.expr = expr;
    }

    public String getColumnName() {
        return columnName;
    }

    public Expr getExpr() {
        return expr;
    }

    public boolean isColumn() {
        return expr == null;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(columnName);
        if (expr != null) {
            sb.append("=").append(expr.toSql());
        }
        return sb.toString();
    }
}
