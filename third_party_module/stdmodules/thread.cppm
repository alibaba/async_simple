module;
#include <thread>
export module std:thread;
export namespace std {
    using std::thread;
    namespace this_thread {
        using std::this_thread::sleep_for;
        using std::this_thread::yield;
        using std::this_thread::get_id;
    }
}
