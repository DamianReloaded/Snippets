#include <array>

struct b_params
{
    float paramc;
};

struct a_params
{
    int param1;
    int param2;
};

union params
{
    b_params b;
    a_params a;
};

void doom_a_function(int param1, int param2)
{
//    std::cout << param1<< " " << param2 << std::endl;
}

void doom_b_function(float paramc)
{
//    std::cout << paramc << std::endl;
}

void a_function (params params)
{
    doom_a_function(params.a.param1, params.a.param2);
}

void b_function (params params)
{
    doom_b_function(params.b.paramc);
}

typedef void(*func)(params);
std::array<func,2> v  { &a_function, &b_function };

int main()
{
	v[0](params { .a {3,4} });
	v[1](params { .b {123.4f} });
}

/*
with -O3 compiles to

a_function(params):
        ret
b_function(params):
        ret
doom_a_function(int, int):
        ret
doom_b_function(float):
        ret
main:
        movabs  rdi, 17179869187
        sub     rsp, 8
        call    [QWORD PTR v[rip]]
        mov     edi, 1123470541
        call    [QWORD PTR v[rip+8]]
        xor     eax, eax
        add     rsp, 8
        ret
v:
        .quad   a_function(params)
        .quad   b_function(params)
*/