#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"

#if __has_include(<memory_resource>)
#include <memory_resource>
#endif

namespace ac = async_simple::coro;

#if __has_include(<memory_resource>)
class print_new_delete_memory_resource : public std::pmr::memory_resource {
    void* do_allocate(std::size_t bytes, size_t /*alignment*/) override {
        std::cout << "allocate " << bytes << " bytes\n";
        return ::operator new(bytes);
    }

    void do_deallocate(void* p, std::size_t bytes,
                       size_t /*alignment*/) override {
        std::cout << "deallocate " << bytes << " bytes\n";
        ::operator delete(p);
    }

    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }
};
#endif

ac::Lazy<int> get_43() {
    std::cout << "run with ::operator new/delete" << '\n';
    co_return 43;
}

#if __has_include(<memory_resource>)
ac::Lazy<int> get_43_plus_i(
    std::allocator_arg_t /*unused*/,
    const std::pmr::polymorphic_allocator<>& /*coroutine state allocator*/,
    int i = 0) {
    std::cout << "run with async_simple::coro::pmr::memory_resource" << '\n';
    int test{};
    test = co_await get_43();
    std::cout << "a pointer on coroutine frame: " << &test << '\n';
    co_return test + i;
}
#endif

std::array<std::byte, 1024> global_buffer;

int main() {
    int i = 0;

    std::cout << "###############################################\n";
    // run without pmr argument, use global new/delete to allocate Lazy's
    // coroutine state
    i += syncAwait(get_43());

#if __has_include(<memory_resource>)
    std::cout << "\n###############################################\n";
    std::cout << "global buffer address: " << global_buffer.data() << '\n';
    std::pmr::monotonic_buffer_resource pool(global_buffer.data(),
                                             global_buffer.size(),
                                             std::pmr::null_memory_resource());
    std::pmr::polymorphic_allocator alloc(&pool);
    // the first argument std::allocator_arg indicates that the second argument
    // is an custom allocator, which will be used to allocate the coroutine
    // state
    i += syncAwait(get_43_plus_i(std::allocator_arg, alloc));

    // allocate Lazy's coroutine state on stack
    std::cout << "\n###############################################\n";
    std::array<std::byte, 1024> stack_buffer;
    std::cout << "stack buffer address: " << stack_buffer.data() << '\n';
    std::pmr::monotonic_buffer_resource stack_pool(
        stack_buffer.data(), stack_buffer.size(),
        std::pmr::null_memory_resource());
    std::pmr::polymorphic_allocator stack_alloc(&stack_pool);
    // additional argument can be passed to the coroutine
    i += syncAwait(get_43_plus_i(std::allocator_arg, stack_alloc, 4));

    std::cout << "\n###############################################\n";
    // run with a custom memory resource which prints the allocation and
    // deallocation process
    print_new_delete_memory_resource res;
    std::pmr::polymorphic_allocator<std::byte> print_alloc(&res);
    i += syncAwait(get_43_plus_i(std::allocator_arg, print_alloc));
#endif

    std::cout << '\n';
    if (i == 43
#if __has_include(<memory_resource>)
                 + 43 + 43 + 43 + 4
#endif
    ) {
        std::cout << "test passed\n";
    } else {
        std::cout << "test failed\n";
    }
    return 0;
}
