import std;
int main() {
    std::printf("Hello World.\n");
    // If you are linking libstdc++, then it would meet a segfault.
    // This is a known problem: https://github.com/llvm/llvm-project/issues/51873
    std::cout << "Hello World." << std::endl;
}
