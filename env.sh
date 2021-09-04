# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# check STARROCKS_HOME
if [[ -z ${STARROCKS_HOME} ]]; then
    echo "Error: STARROCKS_HOME is not set"
    exit 1
fi

# check OS type
if [[ ! -z "$OSTYPE" ]]; then
    if [[ "$OSTYPE" != "linux-gnu" ]]; then
        echo "Error: Unsupported OS type: $OSTYPE"
        exit 1
    fi
fi

# include custom environment variables
if [[ -f ${STARROCKS_HOME}/custom_env.sh ]]; then
    . ${STARROCKS_HOME}/custom_env.sh
fi

# set STARROCKS_THIRDPARTY
if [[ -z ${STARROCKS_THIRDPARTY} ]]; then
    export STARROCKS_THIRDPARTY=${STARROCKS_HOME}/thirdparty
fi

# check python
export PYTHON=python
if ! ${PYTHON} --version; then
    export PYTHON=python2.7
    if ! ${PYTHON} --version; then
        echo "Error: python is not found"
        exit 1
    fi
fi

# set GCC HOME
if [[ -z ${STARROCKS_GCC_HOME} ]]; then
    export STARROCKS_GCC_HOME=$(dirname `which gcc`)/..
fi

gcc_ver=`${STARROCKS_GCC_HOME}/bin/gcc -dumpfullversion -dumpversion`
required_ver="5.3.1"
if [[ ! "$(printf '%s\n' "$required_ver" "$gcc_ver" | sort -V | head -n1)" = "$required_ver" ]]; then 
    echo "Error: GCC version (${gcc_ver}) must be greater than or equal to ${required_ver}"
    exit 1
fi

# export CLANG COMPATIBLE FLAGS
export CLANG_COMPATIBLE_FLAGS=`echo | ${STARROCKS_GCC_HOME}/bin/gcc -Wp,-v -xc++ - -fsyntax-only 2>&1 \
                | grep -E '^\s+/' | awk '{print "-I" $1}' | tr '\n' ' '`

# check java home
if [[ -z ${JAVA_HOME} ]]; then
    echo "Error: JAVA_HOME is not set"
    exit 1
fi

# check java version
export JAVA=${JAVA_HOME}/bin/java
JAVA_VER=$(${JAVA} -version 2>&1 | sed 's/.* version "\(.*\)\.\(.*\)\..*"/\1\2/; 1q' | cut -f1 -d " ")
if [[ $JAVA_VER -lt 18 ]]; then
    echo "Error: require JAVA with JDK version at least 1.8"
    exit 1
fi

# check maven
MVN_CMD=mvn
if [[ ! -z ${CUSTOM_MVN} ]]; then
    MVN_CMD=${CUSTOM_MVN}
fi
if ! ${MVN_CMD} --version; then
    echo "Error: mvn is not found"
    exit 1
fi
export MVN_CMD

CMAKE_CMD=cmake
if [[ ! -z ${CUSTOM_CMAKE} ]]; then
    CMAKE_CMD=${CUSTOM_CMAKE}
fi
if ! ${CMAKE_CMD} --version; then
    echo "Error: cmake is not found"
    exit 1
fi
export CMAKE_CMD
