#include <iostream>
#include <array>

namespace owl {
    template <typename T, size_t Tsize=50>
    class textfixed
    {
        protected:
            std::array<T,Tsize> m_array;
            size_t m_length;

        public:
            textfixed() : m_length(0) {}
            operator const T*() const { return &m_array[0]; }

            void operator = (const T* _value)
            {
                m_length = 0;
                T* c = (T*)_value;
                while (*c!=L'\0' && m_length < Tsize-1)
                {
                    m_array[m_length]=*c;
                    m_length++; c++;
                }
                m_array[(m_length+1<Tsize-1)?m_length+1:Tsize-1]=0;
            }

            T* cstring() { return &m_array[0]; }

            size_t size () {return Tsize;}
            size_t bytesize () {return Tsize*sizeof(T);}
            const size_t& length () {return m_length;}
    };

    namespace c8 {
        template <size_t Tsize=50> class text : public textfixed<char, Tsize>
        {
            public:
                text() { }

                text(const char* _value)
                {
                    textfixed<char,Tsize>::operator=(_value);
                }

                void operator = (const char* _value) { textfixed<char,Tsize>::operator = (_value); }

                const char* cstring() { return (const char*)&textfixed<char,Tsize>::m_array[0]; }

                std::string string()
                {
                    return std::string(&textfixed<char,Tsize>::m_array[0], textfixed<char,Tsize>::length());
                }

                std::string_view string_view()
                {
                    return std::string_view(&textfixed<char,Tsize>::m_array[0], textfixed<char,Tsize>::length());
                }
        };
    }
    namespace c16 {
        template <size_t Tsize=50> class text : public textfixed<char16_t, Tsize>
        {
            public:
                text() { }
                text(const char16_t* _value) { textfixed<char16_t,Tsize>::operator=(_value); }
                text(const wchar_t* _value) { textfixed<char16_t,Tsize>::operator=((const char16_t*)_value); }
                void operator = (const char16_t* _value) { textfixed<char16_t,Tsize>::operator=(_value); }
                void operator = (const wchar_t* _value) { textfixed<char16_t,Tsize>::operator=((const char16_t*)_value); }

                const wchar_t* cstring() { return (const wchar_t*)&textfixed<char16_t,Tsize>::m_array[0]; }

                std::wstring wstring()
                {
                    size_t size = this->length();
                    return std::wstring((wchar_t*)&textfixed<char16_t,Tsize>::m_array[0],textfixed<char16_t,Tsize>::length());
                }

                std::wstring_view wstring_view()
                {
                    return std::wstring_view((wchar_t*)&textfixed<char16_t,Tsize>::m_array[0], textfixed<char16_t,Tsize>::length());
                }
        };
    }
}

int main()
{
    owl::c16::text<20> text16 = L"Hello World 0123456789";
    owl::c16::text<20> text162 = text16;
    std::wcout << text162.cstring() << std::endl;
    std::cout << "16bit Byte size: " << text162.bytesize() <<std::endl;
    std::cout << "16bit Length: " << text162.length() <<std::endl;
    std::cout << "16bit size: " << text162.size() <<std::endl<<std::endl;

    owl::c8::text<20> text8 = "Hello World 0123456789";
    std::cout << text8.cstring() << std::endl;
    std::cout << "8bit Byte size: " << text8.bytesize() <<std::endl;
    std::cout << "8bit Length: " << text8.length() <<std::endl;
    std::cout << "8bit size: " << text8.size() <<std::endl;
    return 0;
}
