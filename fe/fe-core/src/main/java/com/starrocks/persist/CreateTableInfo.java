// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/persist/CreateTableInfo.java

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

package com.starrocks.persist;

import com.starrocks.catalog.Catalog;
import com.starrocks.catalog.Table;
import com.starrocks.cluster.ClusterNamespace;
import com.starrocks.common.FeMetaVersion;
import com.starrocks.common.io.Text;
import com.starrocks.common.io.Writable;
import com.starrocks.system.SystemInfoService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

public class CreateTableInfo implements Writable {
    public static final Logger LOG = LoggerFactory.getLogger(CreateTableInfo.class);

    private String dbName;
    private Table table;

    public CreateTableInfo() {
        // for persist
    }

    public CreateTableInfo(String dbName, Table table) {
        this.dbName = dbName;
        this.table = table;
    }

    public String getDbName() {
        return dbName;
    }

    public Table getTable() {
        return table;
    }

    public void write(DataOutput out) throws IOException {
        Text.writeString(out, dbName);
        table.write(out);
    }

    public void readFields(DataInput in) throws IOException {
        if (Catalog.getCurrentCatalogJournalVersion() < FeMetaVersion.VERSION_30) {
            dbName = ClusterNamespace.getFullName(SystemInfoService.DEFAULT_CLUSTER, Text.readString(in));
        } else {
            dbName = Text.readString(in);
        }

        table = Table.read(in);
    }

    public static CreateTableInfo read(DataInput in) throws IOException {
        CreateTableInfo createTableInfo = new CreateTableInfo();
        createTableInfo.readFields(in);
        return createTableInfo;
    }

    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof CreateTableInfo)) {
            return false;
        }

        CreateTableInfo info = (CreateTableInfo) obj;

        return (dbName.equals(info.dbName))
                && (table.equals(info.table));
    }
}
