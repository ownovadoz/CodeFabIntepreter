#pragma once

#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;
using std::shared_ptr;

// 디버그 모드의 watch/inspect가 스코프를 거슬러 올라가며 변수를 열거할 때 쓰는
// 스냅샷 한 건. is_global은 해당 변수가 발견된 Environment가 최상위(globals)인지를 뜻한다.
struct VariableSnapshot {
    string name;
    Value value;
    bool is_global;
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment() = default;
    explicit Environment(shared_ptr<Environment> enclosing);

    void define(const string& name, const Value& value);
    Value get(const Token& name) const;
    void assign(const Token& name, const Value& value);

    // ScopeResolver가 미리 계산해둔 거리만큼 enclosing을 거슬러 올라가 곧바로
    // 접근한다. 스코프 탐색(get/assign의 재귀적 이름 검색) 없이 O(1)로 동작한다.
    Value getAt(int distance, const string& name) const;
    void assignAt(int distance, const string& name, const Value& value);

    bool isGlobal() const { return enclosing == nullptr; }
    const unordered_map<string, Value>& getOwnVariables() const { return values; }
    const shared_ptr<Environment>& getEnclosing() const { return enclosing; }

private:
    const Environment* ancestor(int distance) const;
    Environment* ancestor(int distance);

    shared_ptr<Environment> enclosing;
    unordered_map<string, Value> values;
};