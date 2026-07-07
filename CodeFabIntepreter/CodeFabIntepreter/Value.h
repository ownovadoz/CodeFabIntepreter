#pragma once

#include <sstream>
#include <string>
#include <variant>

using Value = std::variant<std::monostate, bool, double, std::string>;

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

    return std::get<std::string>(value);
}
