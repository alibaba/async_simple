module;
#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/Collect.h"
#include "async_simple/coro/SyncAwait.h"
#include "async_simple/coro/Latch.h"
#include "async_simple/coro/ConditionVariable.h"
#include "async_simple/coro/FutureAwaiter.h"
#include "async_simple/coro/Mutex.h"
#include "async_simple/coro/Semaphore.h"
#include "async_simple/coro/Sleep.h"

#include "async_simple/executors/SimpleExecutor.h"

#include "async_simple/uthread/Async.h"
#include "async_simple/uthread/Await.h"
#include "async_simple/uthread/Collect.h"
#include "async_simple/uthread/Latch.h"
#include "async_simple/uthread/Uthread.h"

#include "async_simple/Future.h"
#include "async_simple/Collect.h"

#include "async_simple/util/Condition.h"
export module async_simple;
namespace async_simple {
    export using async_simple::Future;
    export using async_simple::Promise;
    export using async_simple::Try;
    export using async_simple::makeReadyFuture;

    export using async_simple::collectAll;

    export using async_simple::CurrentExecutor;
    export using async_simple::Executor;

    export using async_simple::operator co_await;

    export using async_simple::Unit;

namespace executors {
    export using async_simple::executors::SimpleExecutor;
}

namespace coro {
    export using async_simple::coro::Lazy;
    export using async_simple::coro::RescheduleLazy;

    export using async_simple::coro::Yield;

    export using async_simple::coro::collectAll;
    export using async_simple::coro::collectAny;
    export using async_simple::coro::collectAllPara;
    export using async_simple::coro::collectAllWindowed;
    export using async_simple::coro::collectAllWindowedPara;

    export using async_simple::coro::Latch;
    export using async_simple::coro::ConditionVariable;
    export using async_simple::coro::Mutex;
    export using async_simple::coro::CountingSemaphore;
    export using async_simple::coro::SpinLock;

    export using async_simple::coro::sleep;
    export using async_simple::coro::syncAwait;
}

namespace uthread {
    export using async_simple::uthread::collectAll;

    export using async_simple::uthread::Launch;
    export using async_simple::uthread::async;
    export using async_simple::uthread::await;
    export using async_simple::uthread::Latch;
    export using async_simple::uthread::Uthread;
}

namespace util {
    export using async_simple::util::Condition;
}
}

// export coroutine related components to ease the use.
namespace std {
    export using std::coroutine_handle;
    export using std::coroutine_traits;
}
