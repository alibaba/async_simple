import async_simple;
import std;


using namespace async_simple;
using namespace async_simple::coro;

using FileName = std::string;

/////////////////////////////////////////
/////////// Synchronous Part ////////////
/////////////////////////////////////////

// The *Impl is not the key point for this example, code readers
// could ignore this.
std::uint64_t CountCharInFileImpl(const FileName &File, char c) {
    std::uint64_t Ret = 0;
    std::ifstream infile(File);
    std::string line;
    while (std::getline(infile, line))
        Ret += std::count(line.begin(), line.end(), c);
    return Ret;
}

std::uint64_t CountCharInFiles(const std::vector<FileName> &Files, char c) {
    std::uint64_t ReadSize = 0;
    for (const auto &File : Files)
        ReadSize += CountCharInFileImpl(File, c);
    return ReadSize;
}

/////////////////////////////////////////
/////////// Asynchronous Part ///////////
/////////////////////////////////////////

// It's possible that user's toolchain is not sufficient to support ranges yet.
template <typename Iterator>
struct SubRange {
    Iterator B, E;
    SubRange(Iterator begin, Iterator end) 
        : B(begin), E(end) {}

    Iterator begin() { return B; }
    Iterator end() { return E; }
};

template< class T >
concept Range = requires(T& t) {
  t.begin();
  t.end();
};

// It is not travial to implement an asynchronous do_for_each.
template <Range RangeTy, typename Callable>
Future<Unit> do_for_each(RangeTy &&Range, Callable &&func) {
    auto Begin = Range.begin();
    auto End = Range.end();

    if (Begin == End)
        return makeReadyFuture(Unit());

    while (true) {
        auto F = std::invoke(func, *(Begin++));
        // If we met an error, return early.
        if (F.result().hasError() || Begin == End)
            return F;

        if (!F.hasResult())
            return std::move(F).thenTry(
                [func = std::forward<Callable>(func), Begin = std::move(Begin), End = std::move(End)](auto&&) mutable {
                    return do_for_each(SubRange(Begin, End), std::forward<Callable>(func));
                });
    }
}

// The *Impl is not the key point for this example, code readers
// could ignore this.
Future<std::uint64_t> CountCharInFileAsyncImpl(const FileName &File, char c) {
    std::uint64_t Ret = 0;
    std::ifstream infile(File);
    std::string line;
    while (std::getline(infile, line))
        Ret += std::count(line.begin(), line.end(), c);
    return makeReadyFuture(std::move(Ret));
}

Future<std::uint64_t> CountCharInFilesAsync(const std::vector<FileName> &Files, char c) {
    // std::shared_ptr is necessary here. Since ReadSize may be used after
    // CountCharInFilesAsync function ends.
    auto ReadSize = std::make_shared<std::uint64_t>(0);
    // We need to introduce another API `do_for_each` here.
    return do_for_each(std::move(Files), [ReadSize, c](auto &&File){
        return CountCharInFileAsyncImpl(File, c).thenValue([ReadSize](auto &&Size) {
            *ReadSize += Size;
            return makeReadyFuture(Unit());
        });
    }).thenTry([ReadSize] (auto&&) { return makeReadyFuture<std::uint64_t>(*ReadSize); });;
}

/////////////////////////////////////////
/////////// Corotuine Part //////////////
/////////////////////////////////////////

// The *Impl is not the key point for this example, code readers
// could ignore this.
Lazy<std::uint64_t> CountCharFileCoroImpl(const FileName &File, char c) {
    std::uint64_t Ret = 0;
    std::ifstream infile(File);
    std::string line;
    while (std::getline(infile, line))
        Ret += std::count(line.begin(), line.end(), c);
    co_return Ret;
}

Lazy<std::uint64_t> CountCharInFilesCoro(const std::vector<FileName> &Files, char c) {
    std::uint64_t ReadSize = 0;
    for (const auto &File : Files)
        ReadSize += co_await CountCharFileCoroImpl(File, c);
    co_return ReadSize;
}

int main() {
    std::vector<FileName> Files = {
        "Input/1.txt",
        "Input/2.txt",
        "Input/3.txt",
        "Input/4.txt",
        "Input/5.txt",
        "Input/6.txt",
        "Input/7.txt",
        "Input/8.txt",
        "Input/9.txt",
        "Input/10.txt",
    };

    std::cout << "Calculating char counts synchronously.\n";
    auto ResSync = CountCharInFiles(Files, 'x');
    std::cout << "Files contain " << ResSync << " 'x' chars.\n";

    executors::SimpleExecutor executor(4);
    Promise<Unit> p;
    auto future = p.getFuture();

    std::cout << "Calculating char counts asynchronously by future.\n";
    auto f = std::move(future).via(&executor).thenTry([&Files](auto&&){
        return CountCharInFilesAsync(Files, 'x').thenValue([](auto &&Value) {
            std::cout << "Files contain " << Value << " 'x' chars.\n";
        });
    });

    p.setValue(Unit());
    f.wait();

    std::cout << "Calculating char counts asynchronously by coroutine.\n";
    auto Task = CountCharInFilesCoro(Files, 'x').via(&executor);
    auto ResCoro = syncAwait(std::move(Task));
    std::cout << "Files contain " << ResCoro << " 'x' chars.\n";

    return 0;
}
