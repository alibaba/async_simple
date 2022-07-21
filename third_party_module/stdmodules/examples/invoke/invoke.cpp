import std;
 
struct Foo {
    Foo(int num) : num_(num) {}
    void print_add(int i) const { std::cout << num_+i << '\n'; }
    int num_;
};
 
void print_num(int i)
{
    std::cout << i << '\n';
}
 
struct PrintNum {
    void operator()(int i) const
    {
        std::cout << i << '\n';
    }
};
 
int main()
{
    // invoke a free function
    std::invoke(print_num, -9);
 
    // invoke a lambda
    std::invoke([]() { print_num(42); });
 
    // invoke a member function
    const Foo foo(314159);
    std::invoke(&Foo::print_add, foo, 1);
 
    // invoke (access) a data member
    std::cout << "num_: " << std::invoke(&Foo::num_, foo) << '\n';
 
    // invoke a function object
    std::invoke(PrintNum(), 18);
}