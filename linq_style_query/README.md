# C++26 Compile-Time Query DSL

A small experimental query framework built with **C++26 reflection**, **expression templates**, and **static metaprogramming**.

The goal of this project is to provide a strongly typed, SQL/LINQ-like query syntax directly in C++ while keeping the query structure known at compile time.

Example:

```cpp
auto result =
    from(records)
        .where(
            q.name.contains("a")
            &&
            (
                q.age == 32
                ||
                q.weight > 60.0f
            ));
```

Instead of writing manual loops:

```cpp
for(auto& record : records)
{
    if(record.name.find("a") != std::string::npos &&
       (record.age == 32 || record.weight > 60.0f))
    {
        result.push_back(record);
    }
}
```

the query is described declaratively and evaluated by the generated expression tree.

---

# Features

- C++26 compile-time reflection
- Automatically generated query fields
- Expression template based query construction
- Strong compile-time type checking
- No runtime query parsing
- No string-based field lookup
- Lazy filtering through iterators
- LINQ/SQL-inspired syntax
- Zero virtual dispatch
- Extensible operation system

---

# Requirements

The project requires a compiler implementing the C++26 reflection proposal.

Currently this means using an experimental compiler implementation with support for:

- `std::meta`
- `^^T` reflection syntax
- `template for`
- splice expressions (`[: ... :]`)

The code is experimental and intended for exploring future C++ metaprogramming capabilities.

---

# Basic Usage

## Define a Data Type

The query system operates on normal C++ structures.

```cpp
struct Record
{
    std::string name;
    int age;
    float weight;
};
```

No additional metadata or registration is required.

---

## Create Data

```cpp
std::vector<Record> records
{
    {"John",32,75.5f},
    {"Mary",28,61.0f},
    {"Peter",41,83.2f},
    {"Jane",32,59.0f}
};
```

---

## Create a Query

```cpp
auto q = from(records);
```

The query object uses reflection to inspect `Record` and generates fields automatically.

The generated interface becomes:

```cpp
q.name
q.age
q.weight
```

---

## Filter Data

Queries are created using normal C++ operators:

```cpp
auto result =
    q.where(
        q.name.contains("a")
        &&
        (
            q.age == 32
            ||
            q.weight > 60.0f
        ));
```

The expression does not execute immediately.

Instead, the framework creates an expression tree:

```
                 AND
                /   \
          Contains    OR
             /       / \
          name    Equal Greater
                  / \    / \
                age 32 weight 60
```

The tree is evaluated when rows are accessed.

---

# Architecture

The framework is composed of several layers.

---

# Reflection Layer

The framework uses C++26 reflection to inspect data members.

Example:

```cpp
struct Record
{
    std::string name;
    int age;
    float weight;
};
```

Reflection discovers:

```
name
age
weight
```

and generates corresponding field expressions.

This removes the need for manually writing:

```cpp
Field<Record, &Record::name>
```

or maintaining registration tables.

---

# Expression System

Every query component is an expression.

The base expression type:

```cpp
template<typename Derived>
struct Expression
{
};
```

uses CRTP (Curiously Recurring Template Pattern).

This allows operators to be implemented once:

```cpp
q.age == 32
q.weight > 60
q.name.contains("John")
```

while preserving the exact expression type.

---

# Value Expressions

Constants are converted into expressions.

Example:

```cpp
q.age == 32
```

becomes:

```
BinaryExpression
(
    FieldExpression(age),
    ValueExpression(32),
    Equal
)
```

The value is stored inside the expression tree.

---

# Binary Expressions

Operations are represented as types:

```cpp
template<typename Left,
         typename Right,
         typename Operation>
struct BinaryExpression;
```

An expression stores:

- left operand
- right operand
- operation type

For example:

```cpp
q.age == 32
```

creates:

```
             Equal
            /     \
          age      32
```

No computation happens until evaluation.

---

# Evaluation

Expression evaluation is performed through:

```cpp
Evaluate(expression,row);
```

Example:

```cpp
Evaluate(
    q.age == 32,
    record);
```

The evaluation process:

1. Extract the field value
2. Obtain constant values
3. Execute the operation

Conceptually:

```cpp
return record.age == 32;
```

---

# Operations

Operations are separated from expressions.

Example:

```cpp
struct Equal
{
    template<typename L, typename R>
    static constexpr auto Apply(
        L&& left,
        R&& right)
    {
        return left == right;
    }
};
```

The expression system does not know what comparison is performed.

It only stores the operation type.

New operations can be added independently.

Examples:

```cpp
Less
GreaterEqual
StartsWith
EndsWith
Between
```

---

# String Literal Handling

The framework normalizes values before storing them.

A string literal:

```cpp
"John"
```

is converted into:

```cpp
std::string
```

This prevents lifetime problems caused by storing references to temporary arrays.

---

# Query Execution

Filtering is lazy.

The query does not immediately create a new container.

Instead, it provides an iterator.

The iterator advances until a matching row is found:

```cpp
while(current != end &&
      !Evaluate(condition,*current))
{
    ++current;
}
```

This means:

- no unnecessary allocations
- no intermediate result container
- rows are evaluated only when requested

---

# Projection

The framework also supports selecting values:

```cpp
auto names =
    q.where(q.age > 30)
     .select(q.name);
```

The projection is converted into an expression and evaluated against matching rows.

---

# Design Philosophy

The framework follows several principles:

## Describe, Then Execute

Queries are represented as compile-time structures before execution.

## Prefer Types Over Runtime Metadata

Instead of:

```text
"name == 'John'"
```

the compiler sees:

```cpp
FieldExpression<&Record::name>
```

## Let The Compiler Optimize

Because the entire query structure is visible during compilation, the optimizer can remove abstraction overhead.

---

# Possible Extensions

The current implementation provides the foundation for a larger query system.

Possible additions:

- `select`
- `orderBy`
- `groupBy`
- aggregation functions
- joins
- arithmetic expressions
- nullable fields
- database backends
- query optimization passes

Example future syntax:

```cpp
from(records)
    .where(q.age > 18)
    .orderBy(q.name)
    .select(q.name);
```

---

# Project Status

This project is an exploration of what is possible with modern C++26 metaprogramming.

It demonstrates how reflection and expression templates can be combined to create a statically typed query language embedded inside C++.

The implementation focuses on architecture and experimentation rather than production database functionality.

---

# License

Use and modify freely for experimentation and learning.