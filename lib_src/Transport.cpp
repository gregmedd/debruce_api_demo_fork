#include "Transport.h"
#include <iostream>
#include <sstream>

using namespace std;

// struct Transport::Impl {
//     Impl(string& fail_desc, const string& name) = 0;
//     virtual ~Impl() = 0;

//     virtual string process(const string& arg) = 0;
//     virtual void process_with_callable(function<Uuid (size_t)> func);
// };

// struct Transport::Impl {
//     string data;
//     size_t  counter;

//     inline Impl(string& fail_desc, const string& name) {
//         if (name == "fail") {
//             fail_desc = "got fail for name";
//             return;
//         }

//         data = name;
//         counter = 0;
//         cout << __PRETTY_FUNCTION__ << ' ' << data << ' ' << counter << endl;
//     }

//     inline ~Impl()
//     {
//         cout << __PRETTY_FUNCTION__ << ' ' << data << ' ' << counter << endl;
//     }

//     inline string process(const string& arg)
//     {
//         stringstream ss;
//         ss << data << '+' << arg << '+' << counter;
//         counter++;
//         return ss.str();
//     }

//     inline void process_with_callable(function<Uuid (size_t)> func)
//     {
//         cout << __PRETTY_FUNCTION__ << " got " << func(345) << " from callable" << endl;
//     }
// };

std::shared_ptr<Transport::Impl> TransportImpl_factory(string& fail_desc, const string& name);

Transport::Transport(const string& name) : pImpl(TransportImpl_factory(open_fail_desc, name))
{
}

string Transport::process(const string& arg)
{
    return pImpl->process(arg);
}

void Transport::process_with_callable(function<Uuid (size_t)> func)
{
    pImpl->process_with_callable(func);
}