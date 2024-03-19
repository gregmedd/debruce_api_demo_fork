#include "Transport.h"
#include <iostream>
#include <sstream>

using namespace std;

struct MyImpl : public Transport::Impl {
    string data;
    size_t  counter;

    MyImpl(string& fail_desc, const string& name) {
        if (name == "fail") {
            fail_desc = "got fail for name";
            return;
        }

        data = name;
        counter = 0;
        cout << __PRETTY_FUNCTION__ << ' ' << __FILE__ << ' ' << data << ' ' << counter << endl;
    }

    ~MyImpl()
    {
        cout << __PRETTY_FUNCTION__ << ' ' << __FILE__ << ' ' << data << ' ' << counter << endl;
    }

    string process(const string& arg)
    {
        stringstream ss;
        ss << data << '+' << arg << '+' << counter;
        counter++;
        return ss.str();
    }

    void process_with_callable(function<string (size_t)> func)
    {
        cout << __PRETTY_FUNCTION__ << ' ' << __FILE__ << " got " << func(345) << " from callable" << endl;
    }
};

shared_ptr<Transport::Impl> TransportImpl_factory(string& fail_desc, const string& name)
{
    return make_shared<MyImpl>(fail_desc, name);
}