#include <xmem/shared_ptr.hpp>
#include <iostream>

struct handle {};

struct shape {
    virtual ~shape() = default;
    virtual int area() const = 0;
};

struct printable {
    virtual ~printable() = default;
    virtual void print() const = 0;
};

struct square final : public shape, public printable, public handle {
    int side = 7;
    virtual int area() const override {
        return side * side;
    }
    virtual void print() const override {
        std::cout << "Square with side " << side << " has area " << area() << std::endl;
    }
};

int main() {
    auto sq = xmem::make_shared<square>();

    xmem::shared_ptr<shape> s = sq;
    xmem::shared_ptr<void> vs = s;

    xmem::shared_ptr<printable> p = sq;
    xmem::shared_ptr<void> vp = p;

    xmem::shared_ptr<handle> h = sq;
    xmem::shared_ptr<void> vh = h;

    xmem::shared_ptr<void> vsq = sq;

    std::cout
        << "\nsq : " << sq.get()
        << "\ns  : " << s.get()
        << "\nvs : " << vs.get()
        << "\np  : " << p.get()
        << "\nvp : " << vp.get()
        << "\nh  : " << h.get()
        << "\nvh : " << vh.get()
        << "\nvsq: " << vsq.get()
        << "\n";

    auto sq2 = xmem::static_pointer_cast<square>(h);
    sq2->print();

}
