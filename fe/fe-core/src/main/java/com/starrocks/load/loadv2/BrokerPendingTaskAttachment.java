// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/load/loadv2/BrokerPendingTaskAttachment.java

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

package com.starrocks.load.loadv2;

import com.google.common.collect.Maps;
import com.starrocks.load.BrokerFileGroupAggInfo.FileGroupAggKey;
import com.starrocks.thrift.TBrokerFileStatus;

import java.util.List;
import java.util.Map;

public class BrokerPendingTaskAttachment extends TaskAttachment {

    // FileGroupAggKey -> status of files group by FileGroup
    private Map<FileGroupAggKey, List<List<TBrokerFileStatus>>> fileStatusMap = Maps.newHashMap();
    // FileGroupAggKey -> total file num
    private Map<FileGroupAggKey, Integer> fileNumMap = Maps.newHashMap();

    public BrokerPendingTaskAttachment(long taskId) {
        super(taskId);
    }

    public void addFileStatus(FileGroupAggKey aggKey, List<List<TBrokerFileStatus>> fileStatusList) {
        fileStatusMap.put(aggKey, fileStatusList);
        fileNumMap.put(aggKey, fileStatusList.stream().mapToInt(entity -> entity.size()).sum());
    }

    public List<List<TBrokerFileStatus>> getFileStatusByTable(FileGroupAggKey aggKey) {
        return fileStatusMap.get(aggKey);
    }

    public int getFileNumByTable(FileGroupAggKey aggKey) {
        return fileNumMap.get(aggKey);
    }
}
