template <typename T>
class property {
public:
    property() {value={};}
    virtual ~property() {}
    virtual T& operator= (const T& f) { return value = f; }
    virtual operator T () const { return value; }
    virtual explicit operator const T& () const { return value; }
    virtual T* operator->() { return &value; }
protected:
    T value;
};

template <typename T>
class readonly {
public:
    virtual ~readonly() {}
    virtual operator T const & () const { return value; }
protected:
    T value;
};

struct test
{
    test()  { num2.value=3; }
    property<int> num;
    class : public readonly<int> { friend class test; } num2;
};

int main ()
{
    test t;
    t.num = 3;
    return t.num2+t.num; 
}