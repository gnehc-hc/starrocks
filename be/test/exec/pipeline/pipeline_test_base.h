// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "column/chunk.h"
#include "column/vectorized_fwd.h"
#include "gen_cpp/InternalService_types.h"
#include "gtest/gtest.h"
#include "runtime/descriptors.h"
#include "runtime/exec_env.h"
#include "runtime/runtime_state.h"
#include "storage/storage_engine.h"

namespace starrocks::pipeline {

class PipelineTestBase : public ::testing::Test {
public:
    static vectorized::ChunkPtr _create_and_fill_chunk(const std::vector<SlotDescriptor*>& slots, size_t row_num);
    static vectorized::ChunkPtr _create_and_fill_chunk(size_t row_num);

public:
    virtual void SetUp() final;
    virtual void TearDown() final;

protected:
    ExecEnv* _exec_env = nullptr;
    QueryContext* _query_ctx = nullptr;
    FragmentContext* _fragment_ctx = nullptr;
    FragmentFuture _fragment_future;
    RuntimeState* _runtime_state = nullptr;
    ObjectPool* _obj_pool = nullptr;

    TExecPlanFragmentParams _request;

    // Entry of test, subclass should call this method to start test
    void start_test();

    // SubClass can init request in this method
    virtual void _prepare_request() {}

    // lambda used to init _pipelines
    std::function<void()> _pipeline_builder;
    Pipelines _pipelines;

private:
    // Prepare execution context of pipeline
    void _prepare();

    // execute pipeline
    void _execute();
};
}; // namespace starrocks::pipeline
