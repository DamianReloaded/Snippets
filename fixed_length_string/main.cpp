#include <iostream>
#include <array>

namespace owl {
    template <typename T, size_t Tsize=50>
    class textfixed
    {
        std::array<T,Tsize> m_array;
        size_t m_length;
        public:
            textfixed() : m_length(0) {}
            operator const T*() const { return &m_array[0]; }

            void operator = (const T* _value)
            {
                m_length = 0;
                T* c = (T*)_value;
                while (*c!=L'\0' && m_length < Tsize)
                {
                    m_array[m_length]=*c;
                    m_length++; c++;
                }
            }

            std::string string()
            {
                return std::string(&m_array[0],Tsize);
            }

            std::wstring wstring()
            {
                return std::wstring((wchar_t*)&m_array[0],Tsize);
            }

            size_t size () {return Tsize;}
            size_t bytesize () {return Tsize*sizeof(T);}
            const size_t& length () {return m_length;}
    };

    namespace c8 {
        template <size_t Tsize=50> class text : public textfixed<char, Tsize>
        {
            public:
                void operator = (const char* _value) { textfixed<char,Tsize>::operator = (_value); }
        };
    }
    namespace c16 {
        template <size_t Tsize=50> class text : public textfixed<char16_t, Tsize>
        {
            public:
                void operator = (const char16_t* _value) { textfixed<char16_t,Tsize>::operator=(_value); }
        };
    }
}

int main()
{
    owl::c16::text<20> text;

    text = (const char16_t*)L"0123456789";
    std::wcout << text.wstring() << std::endl;
    std::cout << "Byte size: " << text.bytesize() <<std::endl;
    std::cout << "Length: " << text.length() <<std::endl;
    std::cout << "size: " << text.size() <<std::endl;
    return 0;
}
