#pragma once

#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <unordered_map>

using std::enable_shared_from_this;
using std::shared_ptr;
using std::string;
using std::unordered_map;

// import한 파일의 최상위 스코프에서 정의된 var/Func/Class를 이름으로 담아두는
// "가방"이다. `import "a.txt" alias a;`의 a가 평가되는 값이 바로 이 객체이고,
// `a.func()`/`a.name`은 CodeFabInstance의 필드 접근과 동일하게 GetExpr로 여기서
// 이름을 찾는다. CodeFabInstance와 마찬가지로 Callable을 구현하지만(Value가
// shared_ptr<Callable> 하나만 객체 슬롯으로 갖기 때문), 네임스페이스 자체는
// 호출할 수 없으므로 Interpreter가 evaluateCallExpr에서 먼저 걸러낸다.
class CodeFabNamespace : public Callable, public enable_shared_from_this<CodeFabNamespace> {
public:
    Value get(const Token& name) const;
    void define(const string& name, const Value& value);

    int arity() const override;
    Value call(Interpreter& interpreter, const vector<Value>& arguments) const override;
    string toString() const override;

private:
    unordered_map<string, Value> members;
};
