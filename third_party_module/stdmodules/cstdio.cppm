module;
#include <cstdio>
export module std:cstdio;
export namespace std {
    using std::fopen;
    using std::fclose;
    using std::fgetc;
    using std::FILE;

    using std::fprintf;
    using std::fflush;
}

export {
    using ::stdin;
    using ::stdout;
    using ::stderr;
}
