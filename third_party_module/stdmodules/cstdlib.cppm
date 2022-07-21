module;
#include <cstdlib>
#include <ctime>
export module std:cstdlib;
export namespace std {
    using std::rand;
    using std::srand;
    using std::time;
    using std::getenv;
    using std::strtoll;
}
