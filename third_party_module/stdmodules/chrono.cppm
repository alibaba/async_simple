module;
#include <chrono>
#include <ratio>
export module std:chrono;
export namespace std {
    namespace chrono {
        using std::chrono::seconds;
        using std::chrono::nanoseconds;
        using std::chrono::milliseconds;
        using std::chrono::microseconds;
        using std::chrono::duration;
        using std::chrono::duration_cast;

        using std::chrono::operator>;
        using std::chrono::operator>=;
        using std::chrono::operator<;
        using std::chrono::operator<=;
        using std::chrono::operator==;

        using std::chrono::operator+;
        using std::chrono::operator-;

        using std::chrono::weeks;
        using std::chrono::years;
        using std::chrono::minutes;

        using std::chrono::time_point;

        using std::chrono::high_resolution_clock;
        using std::chrono::system_clock;
    }
}
export namespace std {
    using std::deca;
    using std::nano;
    using std::centi;
    using std::micro;
    using std::ratio;
    using std::hecto;
    using std::ratio_multiply;
}

export namespace std {
    inline namespace literals {
        inline namespace chrono_literals {
            using std::literals::chrono_literals::operator""s;
            using std::literals::chrono_literals::operator""ms;
            using std::literals::chrono_literals::operator""us;
        }
    }
}
