#include <exception>
#include <functional>
#include <iostream>
#include <type_traits>
#include "async_simple/Common.h"
#include "async_simple/executors/SimpleExecutor.h"
#include "async_simple/test/unittest.h"
#include "async_simple/uthread/Await.h"
#include "async_simple/uthread/Uthread.h"

using namespace std;

static std::atomic<unsigned> get_stack_holder_count = 0;
static std::atomic<unsigned> delete_stack_holder_count = 0;

namespace async_simple {
namespace uthread {
namespace internal {

stack_holder get_stack_holder(unsigned stack_size) {
    get_stack_holder_count++;
    return stack_holder(new char[stack_size]);
}

void stack_deleter::operator()(char* ptr) const noexcept {
    delete_stack_holder_count++;
    delete[] ptr;
}
}
}
}

namespace async_simple {
namespace uthread {
class UthreadAllocSwapTest : public FUTURE_TESTBASE {
public:
    UthreadAllocSwapTest() : _executor(4) {}
    void caseSetUp() override {}
    void caseTearDown() override {}

    template <class Func>
    void delayedTask(Func&& func, std::size_t ms) {
        std::thread(
            [f = std::move(func), ms](Executor* ex) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                ex->schedule(std::move(f));
            },
            &_executor)
            .detach();
    }

    template <class T>
    struct Awaiter {
        Executor* ex;
        T value;

        Awaiter(Executor* e, T v) : ex(e), value(v) {}

        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> handle) noexcept {
            auto ctx = ex->checkout();
            std::thread([handle, e = ex, ctx]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Executor::Func f = [handle]() mutable { handle.resume(); };
                e->checkin(std::move(f), ctx);
            }).detach();
        }
        T await_resume() noexcept { return value; }
    };
    template <class T>
    coro::Lazy<T> lazySum(T x, T y) {
        co_return co_await Awaiter{&_executor, x + y};
    }

protected:
    executors::SimpleExecutor _executor;
};

TEST_F(UthreadAllocSwapTest, testSwitch) {
    Executor* ex = &_executor;
    auto show = [&](const std::string& message) mutable {
        std::cout << message << "\n";
    };

    auto ioJob = [&]() -> Future<int> {
        Promise<int> p;
        auto f = p.getFuture().via(&_executor);
        delayedTask(
            [p = std::move(p)]() mutable {
                auto value = 1024;
                p.setValue(value);
            },
            100);
        return f;
    };

    std::atomic<int> running = 1;
    _executor.schedule([ex, &running, &show, &ioJob]() mutable {
        Uthread task1(Attribute{ex}, [&show, &ioJob]() {
            show("task1 start");
            auto value = await(ioJob());
            EXPECT_EQ(1024, value);
            show("task1 done");
        });
        task1.join([&]() { 
            running--;
        });
    });

    while (running) {
    }

    EXPECT_NE(get_stack_holder_count, 0);
    while (!delete_stack_holder_count) {
    }

    EXPECT_EQ(delete_stack_holder_count, get_stack_holder_count);
}
}
}
