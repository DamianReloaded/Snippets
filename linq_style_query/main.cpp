// Compile-time query DSL (Domain-Specific Language)

#include <iostream>
#include <meta>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T>
constexpr auto GetMembers()
{
    constexpr auto context =
        std::meta::access_context::current();

    return std::define_static_array(
        std::meta::nonstatic_data_members_of(
            ^^T,
            context));
}

// ============================================================
// DATA
// ============================================================

struct Record
{
    std::string name;
    int age;
    float weight;
};

// ============================================================
// NORMALIZATION
// ============================================================

template<typename T>
using CleanType =
    std::remove_cvref_t<T>;


template<typename T>
struct Normalize
{
    using Type =
        CleanType<T>;
};


template<size_t Size>
struct Normalize<const char(&)[Size]>
{
    using Type =
        std::string;
};


template<typename T>
using NormalizeT =
    typename Normalize<T>::Type;


// ============================================================
// FORWARD DECLARATIONS
// ============================================================

template<typename T>
constexpr auto ToExpression(T&& value);


template<typename Expression, typename Row>
constexpr decltype(auto) Evaluate(
    const Expression& expression,
    Row&& row);


// ============================================================
// CRTP BASE
// ============================================================

template<typename Derived>
struct Expression
{
    template<typename Value>
    constexpr auto operator==(Value&& value) const;

    template<typename Value>
    constexpr auto operator>(Value&& value) const;

    template<typename Value>
    constexpr auto operator&&(Value&& value) const;

    template<typename Value>
    constexpr auto operator||(Value&& value) const;

    template<typename Value>
    constexpr auto contains(Value&& value) const;
};



template<typename T>
concept ExpressionType =
    std::derived_from<
        CleanType<T>,
        Expression<CleanType<T>>>;


// ============================================================
// VALUE
// ============================================================

template<typename T>
struct ValueExpression :
    Expression<ValueExpression<T>>
{
    T value;


    constexpr ValueExpression() = default;


    template<typename U>
    constexpr ValueExpression(U&& value)
        :
        value(std::forward<U>(value))
    {
    }


    template<typename Row>
    constexpr decltype(auto) Evaluate(Row&&) const
    {
        return value;
    }
};


// ============================================================
// FIELD
// ============================================================

template<auto Member>
struct FieldExpression :
    Expression<FieldExpression<Member>>
{
    template<typename Row>
    constexpr decltype(auto) Evaluate(Row&& row) const
    {
        return std::forward<Row>(row).[:Member:];
    }
};


// ============================================================
// TO EXPRESSION
// ============================================================

template<typename T>
constexpr auto ToExpression(T&& value)
{
    using Type =
        CleanType<T>;


    if constexpr(ExpressionType<Type>)
    {
        return std::forward<T>(value);
    }
    else
    {
        using Stored =
            NormalizeT<T>;


        return ValueExpression<Stored>
        {
            std::forward<T>(value)
        };
    }
}


// ============================================================
// EVALUATE
// ============================================================

template<typename Expression, typename Row>
constexpr decltype(auto) Evaluate(
    const Expression& expression,
    Row&& row)
{
    return expression.Evaluate(
        std::forward<Row>(row));
}


// ============================================================
// OPERATIONS
// ============================================================

struct Equal
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& l,
        R&& r)
    {
        return l == r;
    }
};


struct Greater
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& l,
        R&& r)
    {
        return l > r;
    }
};


struct Contains
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& l,
        R&& r)
    {
        return l.find(r) !=
               std::string::npos;
    }
};


struct LogicalAnd
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& l,
        R&& r)
    {
        return l && r;
    }
};


struct LogicalOr
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& l,
        R&& r)
    {
        return l || r;
    }
};


// ============================================================
// BINARY EXPRESSION
// ============================================================

template<typename Left, typename Right, typename Operation>
struct BinaryExpression :
    Expression<
        BinaryExpression<Left, Right, Operation>>
{
    Left left;
    Right right;


    constexpr BinaryExpression(
        Left left,
        Right right)
        :
        left(std::move(left)),
        right(std::move(right))
    {
    }


    template<typename Row>
    constexpr auto Evaluate(Row&& row) const
    {
        return Operation::Apply(
            ::Evaluate(left, row),
            ::Evaluate(right, row));
    }
};



template<typename Operation, typename Left, typename Right>
constexpr auto MakeBinary(
    Left&& left,
    Right&& right)
{
    using LeftExpression =
        decltype(
            ToExpression(
                std::forward<Left>(left)));


    using RightExpression =
        decltype(
            ToExpression(
                std::forward<Right>(right)));


    return BinaryExpression
    <
        LeftExpression,
        RightExpression,
        Operation
    >
    (
        ToExpression(
            std::forward<Left>(left)),

        ToExpression(
            std::forward<Right>(right))
    );
}


// ============================================================
// CRTP OPERATORS
// ============================================================

template<typename Derived>
template<typename Value>
constexpr auto Expression<Derived>::operator==(Value&& value) const
{
    return MakeBinary<Equal>(
        static_cast<const Derived&>(*this),
        std::forward<Value>(value));
}



template<typename Derived>
template<typename Value>
constexpr auto Expression<Derived>::operator>(Value&& value) const
{
    return MakeBinary<Greater>(
        static_cast<const Derived&>(*this),
        std::forward<Value>(value));
}



template<typename Derived>
template<typename Value>
constexpr auto Expression<Derived>::operator&&(Value&& value) const
{
    return MakeBinary<LogicalAnd>(
        static_cast<const Derived&>(*this),
        std::forward<Value>(value));
}



template<typename Derived>
template<typename Value>
constexpr auto Expression<Derived>::operator||(Value&& value) const
{
    return MakeBinary<LogicalOr>(
        static_cast<const Derived&>(*this),
        std::forward<Value>(value));
}



template<typename Derived>
template<typename Value>
constexpr auto Expression<Derived>::contains(Value&& value) const
{
    return MakeBinary<Contains>(
        static_cast<const Derived&>(*this),
        std::forward<Value>(value));
}


// ============================================================
// REFLECTION FIELDS
// ============================================================

template<typename Row>
auto MakeFields()
{
    struct Fields;


    consteval
    {
        std::vector<std::meta::info> generated;


        template for
        (
            constexpr auto member :
            [: std::meta::reflect_constant_array(
                GetMembers<Row>()) :]
        )
        {
            generated.push_back(
                std::meta::data_member_spec(
                    ^^FieldExpression<member>,
                    {
                        .name =
                            std::meta::identifier_of(member)
                    }));
        }


        std::meta::define_aggregate(
            ^^Fields,
            generated);
    }


    return Fields{};
}


// ============================================================
// QUERY
// ============================================================


template<typename Container, typename Condition>
struct FilteredQuery
{
    using Row =
        typename Container::value_type;


    const Container& source;
    Condition condition;


    struct Iterator
    {
        typename Container::const_iterator current;
        typename Container::const_iterator end;
        const Condition& condition;


        void Advance()
        {
            while(current != end &&
                !Evaluate(condition, *current))
            {
                ++current;
            }
        }


        const Row& operator*() const
        {
            return *current;
        }


        Iterator& operator++()
        {
            ++current;
            Advance();

            return *this;
        }


        bool operator!=(const Iterator& other) const
        {
            return current != other.current;
        }
    };


    auto begin() const
    {
        Iterator iterator
        {
            source.begin(),
            source.end(),
            condition
        };

        iterator.Advance();

        return iterator;
    }


    auto end() const
    {
        return Iterator
        {
            source.end(),
            source.end(),
            condition
        };
    }


    template<typename Projection>
    auto select(Projection projection) const
    {
        auto expression =
            ToExpression(projection);


        using Result =
            CleanType<
                decltype(
                    Evaluate(
                        expression,
                        *source.begin()))>;


        std::vector<Result> result;


        for(auto&& row : *this)
        {
            result.push_back(
                Evaluate(
                    expression,
                    row));
        }


        return result;
    }
};



// ============================================================
// NEW QUERY WITH DIRECT FIELDS
// ============================================================


template<typename Container>
struct Query :
    decltype(
        MakeFields<
            typename Container::value_type>())
{
    using Row =
        typename Container::value_type;


    using Fields =
        decltype(
            MakeFields<Row>());


    const Container& source;



    Query(const Container& source)
        :
        source(source)
    {
    }



    template<typename Condition>
    auto where(Condition expression) const
    {
        return FilteredQuery
        <
            Container,
            decltype(
                ToExpression(expression))
        >
        {
            source,
            ToExpression(expression)
        };
    }
};



template<typename Container>
auto from(const Container& source)
{
    return Query<Container>
    {
        source
    };
}



// ============================================================
// MAIN TEST
// ============================================================


int main()
{
    std::vector<Record> records
    {
        {"John",32,75.5f},
        {"Mary",28,61.0f},
        {"Peter",41,83.2f},
        {"Jane",32,59.0f}
    };

    auto q = from(records);

    auto result =
        q.where(
            q.name.contains("a")
            &&
            (
                q.age == 32
                ||
                q.weight > 60.0f
            ));
        //.select(q.name);

    for(auto&& record : result)
    {
        std::cout
            << record.name
            << '\n';
    }

    std::cin.get();
}