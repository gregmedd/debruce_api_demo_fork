#include "Transport.h"
#include <iostream>
#include <sstream>

using namespace std;

struct Transport::Impl {
    string data;
    size_t  counter;

    inline Impl(string& fail_desc, const string& name) {
        if (name == "fail") {
            fail_desc = "got fail for name";
            return;
        }

        data = name;
        counter = 0;
        cout << __PRETTY_FUNCTION__ << ' ' << data << ' ' << counter << endl;
    }

    inline ~Impl()
    {
        cout << __PRETTY_FUNCTION__ << ' ' << data << ' ' << counter << endl;
    }

    inline string process(const string& arg)
    {
        stringstream ss;
        ss << data << '+' << arg << '+' << counter;
        counter++;
        return ss.str();
    }

    inline void process_with_callable(function<Uuid (size_t)> func)
    {
        cout << __PRETTY_FUNCTION__ << " got " << func(345) << " from callable" << endl;
    }
};

Transport::Transport(const string& name) : pImpl(new Impl(open_fail_desc, name))
{
}


bool Transport::is_open() const
{
    return nullptr != pImpl;
}

string Transport::open_error() const
{
    return open_fail_desc;
}

size_t Transport::use_count() const
{
    if (pImpl) return pImpl.use_count();
    return 0;
}

bool Transport::is_same(const Transport& other)
{
    return this->pImpl == other.pImpl;
}

string Transport::process(const string& arg)
{
    return pImpl->process(arg);
}

void Transport::process_with_callable(function<Uuid (size_t)> func)
{
    pImpl->process_with_callable(func);
}