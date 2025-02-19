//
// Created by xmh on 25-2-16.
//

#ifndef CURRENTCOROUTINE_H
#define CURRENTCOROUTINE_H

#include <coroutine>
struct CurrentCoroutine{
    struct CoroutineAwaiter{
        bool await_ready() const noexcept {return false;}
        auto await_suspend(std::coroutine_handle<> handle) noexcept {
            h_ = handle;
            return handle;
        }
        auto await_resume() const noexcept{
            return h_;
        }
        std::coroutine_handle<> h_;
    };
    auto coAwait(){
        return CoroutineAwaiter{};
    }
};

#endif //CURRENTCOROUTINE_H
