import std;

class stream
{
  public:
 
   using flags_type = int;
 
  public:
 
    flags_type flags() const
    { return flags_; }
 
    // Replaces flags_ by newf, and returns the old value.
    flags_type flags(flags_type newf)
    { return std::exchange(flags_, newf); }
 
  private:
 
    flags_type flags_ = 0;
};
 
void f() { std::cout << "f()"; }
 
int main()
{
   stream s;
 
   std::cout << s.flags() << '\n';
   std::cout << s.flags(12) << '\n';
   std::cout << s.flags() << "\n\n";
 
   std::vector<int> v;
 
   // Since the second template parameter has a default value, it is possible
   // to use a braced-init-list as second argument. The expression below
   // is equivalent to std::exchange(v, std::vector<int>{1,2,3,4});
 
   std::exchange(v, {1,2,3,4});
 
   std::copy(begin(v),end(v), std::ostream_iterator<int>(std::cout,", "));
 
   std::cout << "\n\n";
 
   void (*fun)();
 
   // the default value of template parameter also makes possible to use a
   // normal function as second argument. The expression below is equivalent to
   // std::exchange(fun, static_cast<void(*)()>(f))
   std::exchange(fun,f);
   fun();
 
   std::cout << "\n\nFibonacci sequence: ";
   for (int a{0}, b{1}; a < 100; a = std::exchange(b, a + b))
       std::cout << a << ", ";
   std::cout << "...\n";
}
