#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include <meta>

namespace Linq
{
    // ============================================================
    // EXPRESSIONS
    // ============================================================

    struct Expression
    {
    };


    template<typename T>
    struct ValueExpression : Expression
    {
        T value;


        constexpr ValueExpression(T value)
            :
            value(value)
        {
        }


        template<typename Value>
        constexpr auto operator()(Value&&) const
        {
            return value;
        }
    };



    struct RowExpression : Expression
    {
        template<typename Value>
        constexpr decltype(auto) operator()(Value&& value) const
        {
            return std::forward<Value>(value);
        }
    };


    inline constexpr RowExpression row{};



    template<typename T>
    struct IsExpression :
        std::is_base_of<
            Expression,
            std::remove_cvref_t<T>>
    {
    };


    template<typename T>
    inline constexpr bool IsExpressionV =
        IsExpression<T>::value;



    template<typename T>
    constexpr auto ToExpression(T&& value)
    {
        using Type =
            std::remove_cvref_t<T>;


        if constexpr(IsExpressionV<Type>)
        {
            return std::forward<T>(value);
        }
        else
        {
            return ValueExpression<Type>
            {
                std::forward<T>(value)
            };
        }
    }



    // ============================================================
    // MEMBER ACCESS
    // ============================================================


    template<typename Type, typename Member>
    struct MemberExpression : Expression
    {
        Member Type::* member;


        constexpr MemberExpression(Member Type::* value)
            :
            member(value)
        {
        }


        template<typename Object>
        constexpr decltype(auto) operator()(Object&& object) const
        {
            return std::forward<Object>(object).*member;
        }
    };



    template<typename Type, typename Member>
    constexpr auto field(Member Type::* member)
    {
        return MemberExpression<Type, Member>
        {
            member
        };
    }



    // ============================================================
    // OPERATORS
    // ============================================================


    template<typename Left, typename Right>
    struct EqualExpression : Expression
    {
        Left left;
        Right right;


        constexpr EqualExpression(
            Left left,
            Right right)
            :
            left(left),
            right(right)
        {
        }


        template<typename Value>
        constexpr auto operator()(Value&& value) const
        {
            return left(value) ==
                   right(value);
        }
    };



    template<typename Left, typename Right>
    requires (
        IsExpressionV<Left> ||
        IsExpressionV<Right>)
    constexpr auto operator==(
        Left&& left,
        Right&& right)
    {
        using LeftExpression =
            decltype(ToExpression(
                std::forward<Left>(left)));


        using RightExpression =
            decltype(ToExpression(
                std::forward<Right>(right)));


        return EqualExpression<
            LeftExpression,
            RightExpression>
        {
            ToExpression(std::forward<Left>(left)),
            ToExpression(std::forward<Right>(right))
        };
    }



    template<typename Left, typename Right>
    struct GreaterExpression : Expression
    {
        Left left;
        Right right;


        constexpr GreaterExpression(
            Left left,
            Right right)
            :
            left(left),
            right(right)
        {
        }


        template<typename Value>
        constexpr auto operator()(Value&& value) const
        {
            return left(value) >
                   right(value);
        }
    };



    template<typename Left, typename Right>
    requires (
        IsExpressionV<Left> ||
        IsExpressionV<Right>)
    constexpr auto operator>(
        Left&& left,
        Right&& right)
    {
        using LeftExpression =
            decltype(ToExpression(
                std::forward<Left>(left)));


        using RightExpression =
            decltype(ToExpression(
                std::forward<Right>(right)));


        return GreaterExpression<
            LeftExpression,
            RightExpression>
        {
            ToExpression(std::forward<Left>(left)),
            ToExpression(std::forward<Right>(right))
        };
    }



    template<typename Left, typename Right>
    struct MultiplyExpression : Expression
    {
        Left left;
        Right right;


        constexpr MultiplyExpression(
            Left left,
            Right right)
            :
            left(left),
            right(right)
        {
        }


        template<typename Value>
        constexpr auto operator()(Value&& value) const
        {
            return left(value) *
                   right(value);
        }
    };



    template<typename Left, typename Right>
    requires (
        IsExpressionV<Left> ||
        IsExpressionV<Right>)
    constexpr auto operator*(
        Left&& left,
        Right&& right)
    {
        using LeftExpression =
            decltype(ToExpression(
                std::forward<Left>(left)));


        using RightExpression =
            decltype(ToExpression(
                std::forward<Right>(right)));


        return MultiplyExpression<
            LeftExpression,
            RightExpression>
        {
            ToExpression(std::forward<Left>(left)),
            ToExpression(std::forward<Right>(right))
        };
    }



    // ============================================================
    // QUERY
    // ============================================================


    struct TrueExpression : Expression
    {
        template<typename Value>
        constexpr bool operator()(Value&&) const
        {
            return true;
        }
    };



    template<typename Container, typename Condition = TrueExpression>
    class Query
    {
    public:

        Query(
            const Container& source,
            Condition condition = {})
            :
            source(source),
            condition(condition)
        {
        }



        template<typename NewCondition>
        auto where(NewCondition expression) const
        {
            return Query<
                Container,
                decltype(ToExpression(expression))>
            {
                source,
                ToExpression(expression)
            };
        }



        template<typename Projection>
        auto select(Projection expression) const
        {
            auto projection =
                ToExpression(expression);


            using Result =
                std::remove_cvref_t<
                    decltype(
                        projection(
                            *source.begin()))>;


            std::vector<Result> results;


            for(auto&& value : source)
            {
                if(condition(value))
                {
                    results.push_back(
                        projection(value));
                }
            }


            return results;
        }



    private:

        const Container& source;
        Condition condition;
    };



    template<typename Container>
    auto from(const Container& container)
    {
        return Query<Container>
        {
            container
        };
    }



    // ============================================================
    // OUTPUT
    // ============================================================


    template<typename Container>
    void print(const Container& container)
    {
        for(auto&& value : container)
        {
            std::cout
                << value
                << " ";
        }

        std::cout << "\n";
    }

}

// ============================================================
// USER CODE
// ============================================================

// ============================================================
// USER DATA
// ============================================================

struct record
{
    int a;
    float b;
};



// ============================================================
// MAIN
// ============================================================

int main()
{
    using namespace Linq;


    std::vector<record> records
    {
        {1, 1.123f},
        {2, 2.234f},
        {3, 3.345f}
    };



    // ============================================================
    // Select integer members
    // ============================================================

    auto keys =
        from(records)
            .where(field(&record::a) == 2)
            .select(field(&record::a));


    print(keys);



    // ============================================================
    // Select floating point members
    // ============================================================

    auto magnitudes =
        from(records)
            .where(field(&record::b) > 2.0f)
            .select(field(&record::b));


    print(magnitudes);



    // ============================================================
    // Computed projection
    // ============================================================

    auto calculated =
        from(records)
            .where(field(&record::a) > 1)
            .select(
                field(&record::b) * 10
            );


    print(calculated);



    // ============================================================
    // Multiple operations
    // ============================================================

    auto doubled =
        from(records)
            .where(field(&record::b) > 1.0f)
            .select(
                field(&record::b) *
                field(&record::a)
            );


    print(doubled);


    return 0;
}