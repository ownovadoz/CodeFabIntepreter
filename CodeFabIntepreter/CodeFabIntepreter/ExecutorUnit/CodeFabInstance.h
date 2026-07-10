#pragma once

#include "CodeFabClass.h"

#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <unordered_map>

using std::enable_shared_from_this;
using std::shared_ptr;
using std::string;
using std::unordered_map;

// 클래스로부터 생성된 인스턴스. 필드는 동적으로 추가/조회되고, 메서드는 klass에서 찾아 this로 bind된다.
// Callable을 구현하지만(Value가 shared_ptr<Callable> 하나만 객체 슬롯으로 갖기 때문), 인스턴스
// 자체는 호출할 수 없으므로 Interpreter가 evaluateCallExpr에서 먼저 걸러낸다.
class CodeFabInstance : public Callable, public enable_shared_from_this<CodeFabInstance> {
public:
    explicit CodeFabInstance(shared_ptr<CodeFabClass> klass);

    // 필드를 먼저 찾고, 없으면 메서드를(this가 바인딩된 상태로) 찾는다. 둘 다 없으면 런타임 오류.
    Value get(const Token& name);
    void set(const Token& name, const Value& value);

    const shared_ptr<CodeFabClass>& getClass() const;

    int arity() const override;
    Value call(Interpreter& interpreter, const vector<Value>& arguments) const override;
    string toString() const override;

private:
    shared_ptr<CodeFabClass> klass;
    unordered_map<string, Value> fields;
};
