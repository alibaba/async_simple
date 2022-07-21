module;
#include <stdexcept>
#include <system_error>
export module std:stdexcept;
export namespace std {
    using std::runtime_error;
    using std::logic_error;
    using std::rethrow_exception;
    using std::current_exception;
    using std::invalid_argument;
    using std::error_code;
    using std::operator==;
}
