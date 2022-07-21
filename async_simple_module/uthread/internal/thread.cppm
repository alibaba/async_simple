/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (C) 2015 Cloudius Systems, Ltd.
 */

export module async_simple:uthread.thread;

import :Future;
import std;

namespace async_simple {
namespace uthread {
namespace internal {

constexpr inline std::size_t default_base_stack_size = 512 * 1024;
std::size_t get_base_stack_size();

typedef void* fcontext_t;
struct transfer_t {
    fcontext_t fctx;
    void* data;
};

extern "C" __attribute__((__visibility__("default"))) transfer_t
_fl_jump_fcontext(fcontext_t const to, void* vp);
extern "C" __attribute__((__visibility__("default"))) fcontext_t
_fl_make_fcontext(void* sp, std::size_t size, void (*fn)(transfer_t));

class thread_context;

struct jmp_buf_link {
    fcontext_t fcontext = nullptr;
    jmp_buf_link* link = nullptr;
    thread_context* thread = nullptr;

public:
    void switch_in();
    void switch_out();
    void initial_switch_in_completed() {}
};

inline thread_local jmp_buf_link* g_current_context = nullptr;
inline thread_local jmp_buf_link g_unthreaded_context;

namespace thread_impl {

inline thread_context* get() { return g_current_context->thread; }
inline void switch_in(thread_context* to);
inline void switch_out(thread_context* from);
inline bool can_switch_out() { return g_current_context && g_current_context->thread; }

}  // namespace thread_impl

class thread_context {
    struct stack_deleter {
        void operator()(char* ptr) const noexcept {
            delete[] ptr;
        }
    };
    using stack_holder = std::unique_ptr<char[], stack_deleter>;

    const std::size_t stack_size_;
    stack_holder stack_{make_stack()};
    std::function<void()> func_;
    jmp_buf_link context_;

public:
    bool joined_ = false;
    Promise<bool> done_;

private:
    static void s_main(transfer_t t) {
        auto q = reinterpret_cast<thread_context*>(t.data);
        q->context_.link->fcontext = t.fctx;
        q->main();
    }
    void setup() {
        context_.fcontext =
            _fl_make_fcontext(stack_.get() + stack_size_, stack_size_, thread_context::s_main);
        context_.thread = this;
        context_.switch_in();
    }
    void main() {
        #ifdef __x86_64__
        // There is no caller of main() in this context. We need to annotate this frame like this so that
        // unwinders don't try to trace back past this frame.
        // See https://github.com/scylladb/scylla/issues/1909.
            asm(".cfi_undefined rip");
        #elif defined(__PPC__)
            asm(".cfi_undefined lr");
        #elif defined(__aarch64__)
            asm(".cfi_undefined x30");
        #else
            #warning "Backtracing from uthreads may be broken"
        #endif
        context_.initial_switch_in_completed();
        try {
            func_();
            done_.setValue(true);
        } catch (...) {
            done_.setException(std::current_exception());
        }

        context_.switch_out();
    }
    stack_holder make_stack() {
        auto stack = stack_holder(new char[stack_size_]);
        return stack; 
    }

    void switch_in() { context_.switch_in(); }
    void switch_out() { context_.switch_out(); }

public:
    explicit thread_context(std::function<void()> func)
        : stack_size_(get_base_stack_size())
        , func_(std::move(func)) {
            setup();
        }

    ~thread_context() {}

    friend void thread_impl::switch_in(thread_context*);
    friend void thread_impl::switch_out(thread_context*);
};

static const std::string uthread_stack_size = "UTHREAD_STACK_SIZE_KB";
std::size_t get_base_stack_size() {
  static std::size_t stack_size = 0;
  if (stack_size) {
    return stack_size;
  }
  auto env = std::getenv(uthread_stack_size.data());
  if (env) {
    auto kb = std::strtoll(env, nullptr, 10);
    if (kb > 0 && kb < std::numeric_limits<std::size_t>::max()) {
      stack_size = 1024 * kb;
      return stack_size;
    }
  }
  stack_size = default_base_stack_size;
  return stack_size;
}

inline void jmp_buf_link::switch_in() {
  link = std::exchange(g_current_context, this);
  if (!link) [[unlikely]] {
      link = &g_unthreaded_context;
  }
  fcontext = _fl_jump_fcontext(fcontext, thread).fctx;
}

inline void jmp_buf_link::switch_out() {
  g_current_context = link;
  auto from = _fl_jump_fcontext(link->fcontext, nullptr).fctx;
  link->fcontext = from;
}

inline void thread_impl::switch_in(thread_context* to) {
    to->switch_in();
}
inline void thread_impl::switch_out(thread_context* from) {
    from->switch_out();
}

}  // namespace internal
}  // namespace uthread
}  // namespace async_simple
