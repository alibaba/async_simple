// There unhandled macro uses found in the body:
//	'CPU_SETSIZE' defined in /usr/include/sched.h:82:10
//	'CPU_ISSET' defined in /usr/include/sched.h:85:10
//	'CPU_ZERO' defined in /usr/include/sched.h:87:10
//	'CPU_SET' defined in /usr/include/sched.h:83:10
module;
// WARNING: Detected unhandled non interesting includes.
// It is not suggested mix includes and imports from the compiler's
// perspective. Since it may introduce redeclarations within different
// translation units and the compiler is not able to handle such patterns
// efficiently.
//
// See
// https://clang.llvm.org/docs/StandardCPlusPlusModules.html#performance-tips
#include <stdio.h>
#include <cassert>
#include <climits>
#include <version>

#ifdef __linux__
#include <sched.h>
#endif

export module async_simple;
import std;
#define ASYNC_SIMPLE_USE_MODULES
extern "C++" {
#include "coro/Traits.h"
#include "experimental/coroutine.h"
#include "util/move_only_function.h"
}
export extern "C++" {
#include "Executor.h"
#include "MoveWrapper.h"
}
extern "C++" {
#include "Common.h"
#include "CommonMacros.h"
#include "Unit.h"
}
export extern "C++" {
#include "Try.h"
}
extern "C++" {
#include "FutureState.h"
#include "LocalState.h"
#include "Traits.h"
}
export extern "C++" {
#include "Future.h"
#include "Promise.h"
#include "coro/DetachedCoroutine.h"
#include "coro/Lazy.h"
#include "coro/ViaCoroutine.h"
}
extern "C++" {
#include "uthread/internal/thread.h"
#include "uthread/internal/thread_impl.h"
}
export extern "C++" {
#include "Collect.h"
#include "IOExecutor.h"
#include "coro/Collect.h"
#include "coro/ConditionVariable.h"
#include "coro/CountEvent.h"
#include "coro/Dispatch.h"
#include "coro/FutureAwaiter.h"
#include "coro/Generator.h"
#include "coro/Latch.h"
#include "coro/Mutex.h"
#include "coro/PromiseAllocator.h"
#include "coro/ResumeBySchedule.h"
#include "coro/Semaphore.h"
#include "coro/SharedMutex.h"
#include "coro/Sleep.h"
#include "coro/SpinLock.h"
#include "coro/SyncAwait.h"
#include "executors/SimpleExecutor.h"
#include "executors/SimpleIOExecutor.h"
#include "uthread/Async.h"
#include "uthread/Await.h"
#include "uthread/Collect.h"
#include "uthread/Latch.h"
#include "uthread/Uthread.h"
#include "util/Condition.h"
#include "util/Queue.h"
#include "util/ThreadPool.h"
}
