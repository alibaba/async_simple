#include <array>
#include <cstddef>
#include <memory>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/test/unittest.h"

#if __has_include(<memory_resource>)
#include <memory_resource>
#endif

class PMRLazyTest : public FUTURE_TESTBASE {
public:
    void caseSetUp() override {}
    void caseTearDown() override {}
};

namespace ac = async_simple::coro;
ac::Lazy<int> get_43() { co_return 43; }
ac::Lazy<int> get_43_plus_i(int i = 0) { co_return 43 + i; }

#if __has_include(<memory_resource>)
ac::Lazy<int> get_43_plus_i(
    std::allocator_arg_t /*unused*/,
    std::pmr::polymorphic_allocator<> /*coroutine state allocator*/,
    int i = 0) {
    int test{};
    test = co_await get_43();
    co_return test + i;
}
#endif

TEST_F(PMRLazyTest, testStdAllocator) {
    auto lazy = get_43();
    int result = ac::syncAwait(lazy);
    EXPECT_EQ(result, 43);

    auto lazy2 = get_43_plus_i(1);
    int result2 = ac::syncAwait(lazy2);
    EXPECT_EQ(result2, 44);
}

#if __has_include(<memory_resource>)
TEST_F(PMRLazyTest, testPMRAllocator) {
    std::array<std::byte, 1024> stack_buffer {};
    std::pmr::monotonic_buffer_resource stack_memory_resource(
        stack_buffer.data(), stack_buffer.size());
    std::pmr::polymorphic_allocator stack_allocator(&stack_memory_resource);
    auto lazy = get_43_plus_i(std::allocator_arg, stack_allocator, 1);
    int result = ac::syncAwait(lazy);
    EXPECT_EQ(result, 44);
}
#endif