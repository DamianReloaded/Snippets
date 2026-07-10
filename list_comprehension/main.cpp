#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <deque>
#include <string>
#include <utility>
#include <type_traits>

namespace ListComprehension
{
    // ============================================================
    // CORE EXPRESSIONS
    // ============================================================

    struct Expression
    {
    };

    template<typename T>
    struct ValueExpression : Expression
    {
        T value;

        template<typename Value>
        constexpr auto operator()(Value) const
        {
            return value;
        }
    };

    template<typename Function>
    struct FunctionExpression : Expression
    {
        Function function;

        template<typename Value>
        constexpr auto operator()(Value value) const
        {
            return function(value);
        }
    };

    template<typename Function>
    constexpr auto makeExpression(Function function)
    {
        return FunctionExpression<Function>
        {
            {},
            std::move(function)
        };
    }

    struct Placeholder : Expression
    {
        template<typename Value>
        constexpr auto operator()(Value value) const
        {
            return value;
        }
    };

    inline constexpr Placeholder x{};

    struct AlwaysTrue : Expression
    {
        template<typename Value>
        constexpr bool operator()(Value) const
        {
            return true;
        }
    };

    template<typename T>
    struct IsExpression : std::is_base_of<Expression, std::remove_cvref_t<T>>
    {
    };

    template<typename T>
    inline constexpr bool IsExpressionV = IsExpression<T>::value;

    template<typename T>
    constexpr auto ToExpression(T&& value)
    {
        using Type = std::remove_cvref_t<T>;

        if constexpr (IsExpressionV<Type>)
        {
            return std::forward<T>(value);
        }
        else
        {
            return ValueExpression<Type>
            {
                {},
                std::forward<T>(value)
            };
        }
    }


    // ============================================================
    // MEMBER ACCESS EXPRESSIONS
    // ============================================================

    struct FirstExpression : Expression
    {
        template<typename Pair>
        constexpr decltype(auto) operator()(Pair&& pair) const
        {
            return std::forward<Pair>(pair).first;
        }
    };


    struct SecondExpression : Expression
    {
        template<typename Pair>
        constexpr decltype(auto) operator()(Pair&& pair) const
        {
            return std::forward<Pair>(pair).second;
        }
    };


    inline constexpr FirstExpression first{};

    inline constexpr SecondExpression second{};


    // ============================================================
    // OPERATORS
    // ============================================================

    template<typename Left, typename Right>
    struct MultiplyExpression : Expression
    {
        Left left;
        Right right;

        template<typename Value>
        constexpr auto operator()(Value value) const
        {
            return left(value) * right(value);
        }
    };


    template<typename Left, typename Right>
    constexpr auto operator*(Left&& left, Right&& right)
    {
        using LeftExpression =
            decltype(ToExpression(std::forward<Left>(left)));

        using RightExpression =
            decltype(ToExpression(std::forward<Right>(right)));

        return MultiplyExpression<LeftExpression, RightExpression>
        {
            {},
            ToExpression(std::forward<Left>(left)),
            ToExpression(std::forward<Right>(right))
        };
    }


    template<typename Left, typename Right>
    struct LessExpression : Expression
    {
        Left left;
        Right right;

        template<typename Value>
        constexpr auto operator()(Value value) const
        {
            return left(value) < right(value);
        }
    };


    template<typename Left, typename Right>
    constexpr auto operator<(Left&& left, Right&& right)
    {
        using LeftExpression =
            decltype(ToExpression(std::forward<Left>(left)));

        using RightExpression =
            decltype(ToExpression(std::forward<Right>(right)));

        return LessExpression<LeftExpression, RightExpression>
        {
            {},
            ToExpression(std::forward<Left>(left)),
            ToExpression(std::forward<Right>(right))
        };
    }


    // ============================================================
    // QUERY
    // ============================================================

    template<typename Container, typename Condition = AlwaysTrue>
    struct Query
    {
        const Container& container;
        Condition condition;


        template<typename NewCondition>
        constexpr auto only(NewCondition newCondition) const
        {
            using ExpressionType =
                decltype(ToExpression(newCondition));

            return Query<Container, ExpressionType>
            {
                container,
                ToExpression(newCondition)
            };
        }


        template<typename ExpressionType>
        auto each(ExpressionType expression) const
        {
            auto expr = ToExpression(expression);

            using ResultType =
                decltype(expr(*container.begin()));

            std::vector<ResultType> result;

            for (auto&& value : container)
            {
                if (condition(value))
                {
                    result.push_back(expr(value));
                }
            }

            return result;
        }
    };


    template<typename Container>
    constexpr auto in(const Container& container)
    {
        return Query<Container>
        {
            container,
            {}
        };
    }


    // ============================================================
    // UTILITIES
    // ============================================================

    template<typename Container>
    void Print(const Container& container)
    {
        for (auto&& value : container)
        {
            std::cout << value << " ";
        }

        std::cout << "\n";
    }

}

// ============================================================
// USER CODE
// ============================================================
int main()
{
    using namespace ListComprehension;

    // ============================================================
    // std::vector
    // ============================================================

    std::vector<int> nums{ 1, 2, 3, 4 };

    auto squares =
        in(nums)
            .each(x * x);

    Print(squares);

    auto filtered =
        in(nums)
            .only(x < 3)
            .each(x * x);

    Print(filtered);


    // ============================================================
    // std::array
    // ============================================================

    std::array<int, 4> array{ 5, 6, 7, 8 };

    auto arraySquares =
        in(array)
            .each(x * x);

    Print(arraySquares);


    // ============================================================
    // std::list
    // ============================================================

    std::list<int> list{ 10, 20, 30 };

    auto listSquares =
        in(list)
            .each(x * x);

    Print(listSquares);


    // ============================================================
    // std::set
    // ============================================================

    std::set<int> set{ 3, 4, 5 };

    auto setSquares =
        in(set)
            .only(x < 5)
            .each(x * x);

    Print(setSquares);


    // ============================================================
    // std::map
    // ============================================================

    std::map<int, int> map
    {
        { 1, 10 },
        { 2, 20 },
        { 3, 30 }
    };


    auto keys =
        in(map)
            .each(first);

    Print(keys);


    auto values =
        in(map)
            .each(second);

    Print(values);


    auto products =
        in(map)
            .each(first * second);

    Print(products);


    auto filteredProducts =
        in(map)
            .only(first < 3)
            .each(first * second);

    Print(filteredProducts);


    // ============================================================
    // std::unordered_map
    // ============================================================

    std::unordered_map<int, int> unorderedMap
    {
        { 4, 40 },
        { 5, 50 },
        { 6, 60 }
    };


    auto unorderedProducts =
        in(unorderedMap)
            .each(first * second);

    Print(unorderedProducts);


    auto filteredUnorderedProducts =
        in(unorderedMap)
            .only(first < 6)
            .each(first * second);

    Print(filteredUnorderedProducts);


    // ============================================================
    // std::deque
    // ============================================================

    std::deque<int> deque{ 7, 8, 9, 10 };

    auto dequeSquares =
        in(deque)
            .each(x * x);

    Print(dequeSquares);


    // ============================================================
    // std::string
    // ============================================================

    std::string text = "Hello";

    auto characters =
        in(text)
            .each(x);

    Print(characters);


    return 0;
}