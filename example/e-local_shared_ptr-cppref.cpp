// demo from https://en.cppreference.com/w/cpp/me xmem/local_shared_ptr
#include <xmem/local_shared_ptr.hpp>

#include <iostream>

struct MyObj
{
    MyObj()
    {
        std::cout << "MyObj construced" << std::endl;
    }

    ~MyObj()
    {
        std::cout << "MyObj destructed" << std::endl;
    }
};

struct Container : xmem::enable_local_shared_from_this<Container> // note: public inheritance
{
    void CreateMember()
    {
        memberObj = xmem::make_local_shared<MyObj>();
    }
    xmem::local_shared_ptr<MyObj> memberObj;

    xmem::local_shared_ptr<MyObj> GetAsMyObj()
    {
        // Use an alias shared ptr for member
        return xmem::local_shared_ptr<MyObj>(shared_from_this(), memberObj.get());
    }
};


void test()
{

    xmem::local_shared_ptr<Container> cont = xmem::make_local_shared<Container>();
    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';

    std::cout << "Creating member\n\n";
    cont->CreateMember();
    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';

    std::cout << "Creat xmemanother local_shared container\n\n";
    xmem::local_shared_ptr<Container> cont2 = cont;
    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';
    std::cout << "cont2.use_count() = " << cont2.use_count() << '\n';
    std::cout << "cont2.memberObj.use_count() = " << cont2->memberObj.use_count() << '\n';

    std::cout << "GetAsMyObj\n\n";
    xmem::local_shared_ptr<MyObj> myobj1 = cont->GetAsMyObj();
    std::cout << "myobj1.use_count() = " << myobj1.use_count() << '\n';
    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';
    std::cout << "cont2.use_count() = " << cont2.use_count() << '\n';
    std::cout << "cont2.memberObj.use_count() = " << cont2->memberObj.use_count() << '\n';

    std::cout << "copying alias obj\n\n";
    xmem::local_shared_ptr<MyObj> myobj2 = myobj1;
    std::cout << "myobj1.use_count() = " << myobj1.use_count() << '\n';
    std::cout << "myobj2.use_count() = " << myobj2.use_count() << '\n';

    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';
    std::cout << "cont2.use_count() = " << cont2.use_count() << '\n';
    std::cout << "cont2.memberObj.use_count() = " << cont2->memberObj.use_count() << '\n';

    std::cout << "Resetting cont2\n\n";
    cont2.reset();
    std::cout << "myobj1.use_count() = " << myobj1.use_count() << '\n';
    std::cout << "myobj2.use_count() = " << myobj2.use_count() << '\n';

    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';

    std::cout << "Resetting myobj2\n\n";
    myobj2.reset();
    std::cout << "myobj1.use_count() = " << myobj1.use_count() << '\n';
    std::cout << "cont.use_count() = " << cont.use_count() << '\n';
    std::cout << "cont.memberObj.use_count() = " << cont->memberObj.use_count() << '\n';

    std::cout << "Resetting cont\n\n";
    cont.reset();
    std::cout << "myobj1.use_count() = " << myobj1.use_count() << '\n';

    std::cout << "cont.use_count() = " << cont.use_count() << '\n';

}


int main()
{
    test();
}