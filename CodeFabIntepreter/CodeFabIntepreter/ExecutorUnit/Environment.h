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

    // Resolver가 미리 계산해둔 거리(몇 단계 바깥 스코프인지)로 곧장 해당 스코프에
    // 접근한다. enclosing 체인을 이름으로 훑지 않으므로, 거리가 정적으로 정해지는
    // 지역 변수 접근은 이 경로로 O(distance)에 끝난다(체인 전체를 이름으로
    // 재탐색하는 get/assign과 달리 실패할 수 없는 탐색이라 매 단계 hashmap 조회
    // 실패를 겪지 않는다).
    Value getAt(int distance, const string& name);
    void assignAt(int distance, const string& name, const Value& value);

    bool isGlobal() const { return enclosing == nullptr; }
    const unordered_map<string, Value>& getOwnVariables() const { return values; }
    const shared_ptr<Environment>& getEnclosing() const { return enclosing; }

private:
    Environment* ancestor(int distance);

    shared_ptr<Environment> enclosing;
    unordered_map<string, Value> values;
};