// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/analysis/UserDesc.java

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

// Description of user in SQL statement
public class UserDesc {
    private UserIdentity userIdent;
    private String password;
    private boolean isPasswordPlain;
    private String authPlugin;
    private String authString;

    public UserDesc(UserIdentity userIdent) {
        this(userIdent, "", false);
    }

    public UserDesc(UserIdentity userIdent, String password, boolean isPasswordPlain) {
        this.userIdent = userIdent;
        this.password = password;
        this.isPasswordPlain = isPasswordPlain;
    }

    public UserDesc(UserIdentity userIdent, String authPlugin) {
        this.userIdent = userIdent;
        this.authPlugin = authPlugin;
    }

    public UserDesc(UserIdentity userIdent, String authPlugin, String authString, boolean isPasswordPlain) {
        this.userIdent = userIdent;
        this.authPlugin = authPlugin;
        this.authString = authString;
        this.isPasswordPlain = isPasswordPlain;
    }

    public UserIdentity getUserIdent() {
        return userIdent;
    }

    public String getPassword() {
        return password;
    }

    public boolean isPasswordPlain() {
        return isPasswordPlain;
    }

    public String getAuthPlugin() {
        return authPlugin;
    }

    public String getAuthString() {
        return authString;
    }
}
