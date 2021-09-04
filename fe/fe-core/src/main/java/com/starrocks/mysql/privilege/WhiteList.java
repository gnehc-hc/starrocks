// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/fe/fe-core/src/main/java/org/apache/doris/mysql/privilege/WhiteList.java

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

package com.starrocks.mysql.privilege;

import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import com.starrocks.analysis.TablePattern;
import com.starrocks.analysis.UserIdentity;
import com.starrocks.catalog.Catalog;
import com.starrocks.common.DdlException;
import com.starrocks.common.FeMetaVersion;
import com.starrocks.common.io.Text;
import com.starrocks.common.io.Writable;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Map;
import java.util.Set;

// grant privs.. on db.tbl to user@['domain.name']
// revoke privs on db.tbl from user@['domain.name']
public class WhiteList implements Writable {
    private static final Logger LOG = LogManager.getLogger(WhiteList.class);

    private Map<String, byte[]> passwordMap = Maps.newConcurrentMap();

    // Domain name to resolved IPs, generated by DomainResolver
    private Map<String, Set<String>> resolvedIPsMap = Maps.newConcurrentMap();

    // this 2 variables are deprecated and only be used for converting
    @Deprecated
    private Map<String, Map<TablePattern, PrivBitSet>> oldDomainPrivsMap = Maps.newHashMap();
    @Deprecated
    private byte[] oldPassword;

    public void removeDomain(String domain) {
        passwordMap.remove(domain);
        resolvedIPsMap.remove(domain);
    }

    public void setPassword(String domain, byte[] password) {
        this.passwordMap.put(domain, password);
    }

    // handle new resolved IPs.
    // it will only modify password entry of these resolved IPs. All other privileges are binded
    // to the domain, so no need to modify.
    public void addUserPrivEntriesByResovledIPs(String user, Map<String, Set<String>> resolvedIPsMap) {
        // the parameter "resolvedIPsMap" contains all resolved domains.
        // "newResolvedIPsMap" will only save the domains contained in this white list.
        Map<String, Set<String>> newResolvedIPsMap = Maps.newHashMap();
        for (Map.Entry<String, Set<String>> entry : resolvedIPsMap.entrySet()) {
            if (!containsDomain(entry.getKey())) {
                continue;
            }

            newResolvedIPsMap.put(entry.getKey(), entry.getValue());

            // this user ident will be saved along with each resolved "IP" user ident, so that when checking
            // password, this "domain" user ident will be returned as "current user".
            UserIdentity domainUserIdent = UserIdentity.createAnalyzedUserIdentWithDomain(user, entry.getKey());
            for (String newIP : entry.getValue()) {
                UserIdentity userIdent = UserIdentity.createAnalyzedUserIdentWithIp(user, newIP);
                byte[] password = passwordMap.get(entry.getKey());
                Preconditions.checkNotNull(password, entry.getKey());
                // set password
                try {
                    Catalog.getCurrentCatalog().getAuth()
                            .setPasswordInternal(userIdent, new Password(password), domainUserIdent,
                                    false /* err on non exist */, true /* set by resolver */, true /* is replay */);
                } catch (DdlException e) {
                    // this may happen when this user ident is already set by user, so that resolver can not
                    // overwrite it. just add a debug log to observer.
                    LOG.debug("failed to set password for user ident: {}, {}", userIdent, e.getMessage());
                }
            }
        }

        // set new resolved IPs
        this.resolvedIPsMap = newResolvedIPsMap;
    }

    public Map<String, Set<String>> getResolvedIPs() {
        return resolvedIPsMap;
    }

    public boolean containsDomain(String domain) {
        return passwordMap.containsKey(domain);
    }

    public Set<String> getAllDomains() {
        return Sets.newHashSet(passwordMap.keySet());
    }

    public boolean hasPassword(String domain) {
        return passwordMap.containsKey(domain) && passwordMap.get(domain).length > 0;
    }

    @Override
    public String toString() {
        return Joiner.on(", ").join(passwordMap.keySet());
    }

    @Override
    public void write(DataOutput out) throws IOException {
        out.writeInt(passwordMap.size());
        for (Map.Entry<String, byte[]> entry : passwordMap.entrySet()) {
            Text.writeString(out, entry.getKey());
            byte[] password = entry.getValue();
            out.writeInt(password.length);
            out.write(password);
        }
    }

    public void readFields(DataInput in) throws IOException {
        if (Catalog.getCurrentCatalogJournalVersion() >= FeMetaVersion.VERSION_43) {
            if (Catalog.getCurrentCatalogJournalVersion() < FeMetaVersion.VERSION_69) {
                // db priv map
                int size = in.readInt();
                for (int i = 0; i < size; i++) {
                    String domain = Text.readString(in);
                    Map<TablePattern, PrivBitSet> privsMap = Maps.newConcurrentMap();
                    oldDomainPrivsMap.put(domain, privsMap);

                    int count = in.readInt();
                    for (int j = 0; j < count; j++) {
                        TablePattern tablePattern = TablePattern.read(in);
                        PrivBitSet privs = PrivBitSet.read(in);
                        privsMap.put(tablePattern, privs);
                    }
                }

                // password
                if (in.readBoolean()) {
                    int passwordLen = in.readInt();
                    oldPassword = new byte[passwordLen];
                    in.readFully(oldPassword);
                } else {
                    oldPassword = new byte[0];
                }

                // convert to password map
                for (String domain : oldDomainPrivsMap.keySet()) {
                    passwordMap.put(domain, oldPassword);
                }
            } else {
                // meta version >= FeMetaVersion.VERSION_66
                int size = in.readInt();
                for (int i = 0; i < size; i++) {
                    String domain = Text.readString(in);
                    int passLen = in.readInt();
                    byte[] password = new byte[passLen];
                    in.readFully(password);
                    passwordMap.put(domain, password);
                }
            }
        }
    }

    // in previous implementation(before meta version 67), we save privs in the domainPrivsMap.
    // and now, these privs which corresponding to the domains should be saved directly in priv tables.
    // so we need to convert them.
    public void convertOldDomainPrivMap(String user) {
        Preconditions.checkState(Catalog.getCurrentCatalogJournalVersion() < FeMetaVersion.VERSION_69);
        for (Map.Entry<String, Map<TablePattern, PrivBitSet>> domainEntry : oldDomainPrivsMap.entrySet()) {
            String domain = domainEntry.getKey();
            for (Map.Entry<TablePattern, PrivBitSet> privEntry : domainEntry.getValue().entrySet()) {
                TablePattern tablePattern = privEntry.getKey();
                PrivBitSet privBitSet = privEntry.getValue();
                UserIdentity userIdent = UserIdentity.createAnalyzedUserIdentWithDomain(user, domain);
                try {
                    Catalog.getCurrentCatalog().getAuth().grantPrivs(userIdent, tablePattern, privBitSet,
                            false /* err on non exist */);
                } catch (DdlException e) {
                    // this may happen if priv entry is already set by user. just print a log here.
                    LOG.warn("failed to grant privs {} on {} to user {} when convert old domain priv map.",
                            privBitSet, tablePattern, userIdent, e);
                }
            }
        }
    }
}
