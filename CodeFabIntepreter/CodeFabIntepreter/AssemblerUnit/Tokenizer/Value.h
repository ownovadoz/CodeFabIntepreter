#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

class Interpreter;
class Callable;
class CodeFabArray;

// CodeFab 런타임 값 표현: nil(monostate) / boolean / number(double) / string /
// 호출 가능한 값(Callable) / 고정 크기 배열(CodeFabArray)
using Value = std::variant<std::monostate, bool, double, std::string, std::shared_ptr<Callable>, std::shared_ptr<CodeFabArray>>;

// 함수/메서드처럼 CodeFab 코드에서 호출 가능한 값이 구현해야 하는 인터페이스.
class Callable
{
public:
    virtual ~Callable() = default;

    virtual int arity() const = 0;
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments) const = 0;
    virtual std::string toString() const = 0;
};

// `Array(size)`로 생성되는 고정 크기 배열. 원소는 nil(monostate)로 초기화된다.
// 대입 시 값이 복사되지 않고 참조가 공유되도록(다른 함수 인자로 배열을 전달해도
// 같은 배열을 가리키도록) shared_ptr로만 Value에 담긴다.
class CodeFabArray
{
public:
    explicit CodeFabArray(int size) : elements(size) {}

    int size() const {
        return static_cast<int>(elements.size());
    }

    const Value& get(int index) const {
        return elements[index];
    }

    void set(int index, const Value& value) {
        elements[index] = value;
    }

private:
    std::vector<Value> elements;
};

inline bool isTruthy(const Value& value)
{
    if (std::holds_alternative<std::monostate>(value))
    {
        return false;
    }

    if (std::holds_alternative<bool>(value))
    {
        return std::get<bool>(value);
    }

    return true;
}

inline bool isNumber(const Value& value)
{
    return std::holds_alternative<double>(value);
}

inline bool isString(const Value& value)
{
    return std::holds_alternative<std::string>(value);
}

inline bool isCallable(const Value& value)
{
    return std::holds_alternative<std::shared_ptr<Callable>>(value);
}

inline bool isArray(const Value& value)
{
    return std::holds_alternative<std::shared_ptr<CodeFabArray>>(value);
}

inline std::string numberToString(double number)
{
    if (number == static_cast<long long>(number))
    {
        return std::to_string(static_cast<long long>(number));
    }

    std::ostringstream stream;
    stream << number;
    return stream.str();
}

inline std::string stringify(const Value& value)
{
    if (std::holds_alternative<std::monostate>(value))
    {
        return "nil";
    }

    if (std::holds_alternative<bool>(value))
    {
        return std::get<bool>(value) ? "true" : "false";
    }

    if (std::holds_alternative<double>(value))
    {
        return numberToString(std::get<double>(value));
    }

    if (std::holds_alternative<std::shared_ptr<Callable>>(value))
    {
        return std::get<std::shared_ptr<Callable>>(value)->toString();
    }

    if (std::holds_alternative<std::shared_ptr<CodeFabArray>>(value))
    {
        return "<array>";
    }

    return std::get<std::string>(value);
}
