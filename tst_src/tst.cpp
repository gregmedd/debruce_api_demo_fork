#include "Transport.h"

#include <iostream>
#include <sstream>

using namespace std;

void func(Transport handle)
{
    cout << handle.process("e") << endl;
    cout << handle.process("f") << endl;
}

struct MyCallable {
    size_t data;

    MyCallable(size_t data) : data(data) {}

    Transport::Uuid operator()(size_t arg)
    {
        using namespace std;

        stringstream ss;
        ss << "MyCallable data=" << data << " arg=" << arg;
        return ss.str();
    }
};

int main(int argc, char *argv[])
{
    cout << "top of main" << endl;

    {
        Transport handle1("handle1");
        cout << handle1.process("a") << endl;
        cout << handle1.process("b") << endl;

        auto handle2 = handle1;
        cout << handle1.process("c") << endl;
        cout << handle1.process("d") << endl;

        cout << "inside use_count=" << handle1.use_count() << endl;
        cout << "is handle1 == handle2 = " << handle1.is_same(handle2) << endl;

        Transport handle3("handle3");
        cout << "is handle1 == handle3 = " << handle1.is_same(handle3) << endl;

        handle3.process_with_callable([&](size_t arg) { stringstream ss; ss << "lambda" << arg; return ss.str(); });

        MyCallable policy1(1), policy2(2);
        handle3.process_with_callable(policy1);
        handle3.process_with_callable(policy2);
    }

    cout << "bottom of main" << endl;
}