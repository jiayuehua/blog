#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define TRIVIAL_ABI __attribute__((trivial_abi))

template<class Ptr>
Ptr incr(Ptr p) {
    p += 1;
    return p;
}

template<class T>
class offset_ptr {
    intptr_t value_;
public:
    offset_ptr(T *p) : value_((const char*)p - (const char*)this) {}
    offset_ptr(const offset_ptr& rhs) : value_((const char*)rhs.get() - (const char*)this) {}
    T *get() const { return (T *)((const char *)this + value_); }
    offset_ptr& operator=(const offset_ptr& rhs) {
        value_ = ((const char*)rhs.get() - (const char*)this);
        return *this;
    }
    offset_ptr& operator+=(int diff) {
        value_ += (diff * sizeof (T));
        return *this;
    }
};

template<class T>
class TRIVIAL_ABI trivial_offset_ptr {
    intptr_t value_;
public:
    trivial_offset_ptr(T *p) : value_((const char*)p - (const char*)this) {}
    trivial_offset_ptr(const trivial_offset_ptr& rhs) : value_((const char*)rhs.get() - (const char*)this) {}
    T *get() const { return (T *)((const char *)this + value_); }
    trivial_offset_ptr& operator=(const trivial_offset_ptr& rhs) {
        value_ = ((const char*)rhs.get() - (const char*)this);
        return *this;
    }
    trivial_offset_ptr& operator+=(int diff) {
        value_ += (diff * sizeof (T));
        return *this;
    }
};

template offset_ptr<int> incr(offset_ptr<int>);
template trivial_offset_ptr<int> incr(trivial_offset_ptr<int>);

int main()
{
    int a[10];
    offset_ptr<int> op = &a[4];
    op = incr(op);
    assert(op.get() == &a[5]);

    trivial_offset_ptr<int> top = &a[4];
    top = incr(top);
    assert(top.get() == &a[5]);
}

