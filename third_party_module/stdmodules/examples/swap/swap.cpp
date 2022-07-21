import std;
 
namespace Ns {
class A {
    int id{};
 
    friend void swap(A& lhs, A& rhs) {
        std::cout << "swap(" << lhs << ", " << rhs << ")\n";
        std::swap(lhs.id, rhs.id);
    }
 
    friend std::ostream& operator<< (std::ostream& os, A const& a) {
        return os << "A::id=" << a.id;
    }
 
public:
    A(int i) : id{i} { }
    A(A const&) = delete;
    A& operator = (A const&) = delete;
};
}
 
int main()
{
    int a = 5, b = 3;
    std::cout << a << ' ' << b << '\n';
    std::swap(a,b);
    std::cout << a << ' ' << b << '\n';
 
    Ns::A p{6}, q{9};
    std::cout << p << ' ' << q << '\n';
//  std::swap(p, q);  // error, type requirements are not satisfied
    swap(p, q);  // OK, ADL finds the appropriate friend `swap`
    std::cout << p << ' ' << q << '\n';
}
