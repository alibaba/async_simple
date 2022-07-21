import std;
 
namespace detail { 
 
void* voidify(const volatile void* ptr) noexcept { return const_cast<void*>(ptr); }
 
}
 
// #1, enabled via the return type
template<class T>
typename std::enable_if<std::is_trivially_default_constructible<T>::value>::type 
    construct(T*) 
{
    std::cout << "default constructing trivially default constructible T\n";
}
 
// same as above
template<class T>
typename std::enable_if<!std::is_trivially_default_constructible<T>::value>::type 
    construct(T* p) 
{
    std::cout << "default constructing non-trivially default constructible T\n";
    ::new(detail::voidify(p)) T;
}
 
// #2
template<class T, class... Args>
std::enable_if_t<std::is_constructible<T, Args&&...>::value> // Using helper type
    construct(T* p, Args&&... args) 
{
    std::cout << "constructing T with operation\n";
    ::new(detail::voidify(p)) T(static_cast<Args&&>(args)...);
}
 
// #3, enabled via a parameter
template<class T>
void destroy(
    T*, 
    typename std::enable_if<
        std::is_trivially_destructible<T>::value
    >::type* = 0
){
    std::cout << "destroying trivially destructible T\n";
}
 
// #4, enabled via a non-type template parameter
template<class T,
         typename std::enable_if<
             !std::is_trivially_destructible<T>{} &&
             (std::is_class<T>{} || std::is_union<T>{}),
            bool>::type = true>
void destroy(T* t)
{
    std::cout << "destroying non-trivially destructible T\n";
    t->~T();
}
 
// #5, enabled via a type template parameter
template<class T,
	typename = std::enable_if_t<std::is_array<T>::value> >
void destroy(T* t) // note: function signature is unmodified
{
    for(std::size_t i = 0; i < std::extent<T>::value; ++i) {
        destroy((*t)[i]);
    }
}
/*
template<class T,
	typename = std::enable_if_t<std::is_void<T>::value> >
void destroy(T* t){} // error: has the same signature with #5
*/
 
// the partial specialization of A is enabled via a template parameter
template<class T, class Enable = void>
class A {}; // primary template
 
template<class T>
class A<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
}; // specialization for floating point types
 
int main()
{
    std::aligned_union_t<0,int,std::string> u;
 
    construct(reinterpret_cast<int*>(&u));
    destroy(reinterpret_cast<int*>(&u));
 
    construct(reinterpret_cast<std::string*>(&u),"Hello");
    destroy(reinterpret_cast<std::string*>(&u));
 
    A<int>{}; // OK: matches the primary template
    A<double>{}; // OK: matches the partial specialization
}