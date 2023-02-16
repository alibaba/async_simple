set(SRCS ./Executor.cppm
         ./Common.cppm
         ./util/ThreadPool.cppm
         ./util/Queue.cppm
         ./util/Condition.cppm
         ./IOExecutor.cppm
         ./uthread/Collect.cppm
         ./uthread/Latch.cppm
         ./uthread/Uthread.cppm
         ./uthread/Async.cppm
         ./uthread/Await.cppm
         ./uthread/internal/thread.cppm
         ./executors/SimpleExecutor.cppm
         ./executors/SimpleIOExecutor.cppm
         ./async_simple.cppm
         ./Traits.cppm
         ./Unit.cppm
         ./Try.cppm
         ./FutureState.cppm
         ./MoveWrapper.cppm
         ./coro/Collect.cppm
         ./coro/Util.cppm
         ./coro/CoAwait.cppm
         ./coro/Event.cppm
         ./coro/Traits.cppm
         ./coro/Lazy.cppm
         ./coro/Task.cppm
         ./coro/Sleep.cppm
         ./Invoke.cppm
         ./LocalState.cppm
         ./Future.cppm)
         
add_library(async_simple)
target_sources(async_simple
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ${SRCS}
)
