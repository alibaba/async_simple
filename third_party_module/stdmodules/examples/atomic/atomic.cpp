import std;

std::atomic<int> atomic_count{0};
std::atomic<int> atomic_writes{0};
 
constexpr int global_max_count{72};
constexpr int writes_per_line{8};
constexpr int max_delay{100};
 
template<int Max> int random_value()
{
    static std::uniform_int_distribution<int> distr{1, Max};
    static std::random_device engine;
    static std::mt19937 noise{engine()};
    static std::mutex rand_mutex;
    std::lock_guard<std::mutex> μ{rand_mutex};
    return distr(noise);
}

int main()
{
    auto work = [](const char id)
    {
        for (int count{}; (count = ++atomic_count) <= global_max_count;) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(random_value<max_delay>()));
 
            bool new_line{false};
            if (++atomic_writes % writes_per_line == 0) {
                new_line = true;
            }
            // print thread `id` and `count` value
            {
                static std::mutex cout_mutex;
                std::lock_guard<std::mutex> m{cout_mutex};
                std::cout << "[" << id << "] " << std::setw(3) << count << " │ "
                          << (new_line ? "\n" : "") << std::flush;
            }
        }
    };
    
    // It looks like libcxx hasn't implemented jthread.
    std::thread j1(work, 'A'), j2(work, 'B'), j3(work, 'C'), j4(work, 'D');
    j1.join();
    j2.join();
    j3.join();
    j4.join();
}
