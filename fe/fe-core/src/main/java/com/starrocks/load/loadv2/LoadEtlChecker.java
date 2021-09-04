// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/load/loadv2/LoadEtlChecker.java

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

import com.starrocks.common.Config;
import com.starrocks.common.util.MasterDaemon;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * LoadEtlChecker will update etl status for jobs that have etl state.
 * Now only for SparkLoadJob
 */
public class LoadEtlChecker extends MasterDaemon {
    private static final Logger LOG = LogManager.getLogger(LoadEtlChecker.class);

    private LoadManager loadManager;

    public LoadEtlChecker(LoadManager loadManager) {
        super("Load etl checker", Config.load_checker_interval_second * 1000);
        this.loadManager = loadManager;
    }

    @Override
    protected void runAfterCatalogReady() {
        try {
            loadManager.processEtlStateJobs();
        } catch (Throwable e) {
            LOG.warn("Failed to process one round of LoadEtlChecker with error message {}", e.getMessage(), e);
        }
    }
}
