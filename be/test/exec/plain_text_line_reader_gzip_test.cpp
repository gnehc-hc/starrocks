// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/be/test/exec/plain_text_line_reader_gzip_test.cpp

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

#include <gtest/gtest.h>

#include "exec/decompressor.h"
#include "exec/local_file_reader.h"
#include "exec/plain_text_line_reader.h"
#include "util/runtime_profile.h"

namespace starrocks {

class PlainTextLineReaderGZipTest : public testing::Test {
public:
    PlainTextLineReaderGZipTest() : _profile("TestProfile") {}

protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

private:
    RuntimeProfile _profile;
};

TEST_F(PlainTextLineReaderGZipTest, gzip_normal_use) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/test_file.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, -1, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;

    // 1,2
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(3, size);
    ASSERT_FALSE(eof);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(0, size);
    ASSERT_FALSE(eof);

    // 1,2,3,4
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(7, size);
    ASSERT_FALSE(eof);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_TRUE(eof);
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, uncompressed_no_newline) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/no_newline.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, -1, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;

    // 1,2,3
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    ASSERT_STREQ("1,2,3", std::string((char*)ptr, size).c_str());
    ASSERT_FALSE(eof);

    // 4,5
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(3, size);
    ASSERT_STREQ("4,5", std::string((char*)ptr, size).c_str());
    ASSERT_FALSE(eof);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_TRUE(eof);
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, gzip_test_limit) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/limit.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, 8, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    ASSERT_FALSE(eof);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);
    ASSERT_EQ(0, size);

    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    ASSERT_FALSE(eof);
    LOG(INFO) << std::string((const char*)ptr, size);

    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, gzip_test_limit2) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/limit.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, 6, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, gzip_test_limit3) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/limit.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, 7, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);
    ASSERT_EQ(0, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, gzip_test_limit4) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/limit.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, 7, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_EQ(5, size);
    LOG(INFO) << std::string((const char*)ptr, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    ASSERT_FALSE(eof);
    ASSERT_EQ(0, size);

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    delete decompressor;
}

TEST_F(PlainTextLineReaderGZipTest, gzip_test_limit5) {
    LocalFileReader file_reader("./be/test/exec/test_data/plain_text_line_reader/limit.csv.gz", 0);
    auto st = file_reader.open();
    ASSERT_TRUE(st.ok());

    Decompressor* decompressor;
    st = Decompressor::create_decompressor(CompressionTypePB::GZIP, &decompressor);
    ASSERT_TRUE(st.ok());

    PlainTextLineReader line_reader(&_profile, &file_reader, decompressor, 0, '\n');
    const uint8_t* ptr;
    size_t size;
    bool eof;

    // Empty
    st = line_reader.read_line(&ptr, &size, &eof);
    ASSERT_TRUE(st.ok());
    delete decompressor;
}

} // end namespace starrocks
