#include <boost/flyweight.hpp>
#include <iostream>
#include <string>

int main()
{
    std::cout << "start\n" << std::flush;

    boost::flyweight<std::string> a1("AAPL");
    boost::flyweight<std::string> a2("AAPL");

    std::cout << "a1=" << a1 << " a2=" << a2 << '\n';
    std::cout << "same ptr: " << (&a1.get() == &a2.get()) << '\n';

    return 0;
}
