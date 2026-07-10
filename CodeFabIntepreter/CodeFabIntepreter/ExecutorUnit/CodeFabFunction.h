#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
using std::vector;

class Interpreter;
class CodeFabInstance;

// CodeFab에서 Func으로 선언된 사용자 정의 함수를 나타내는 호출 가능한 값.
// 선언 시점의 Environment를 closure로 캡처해, 호출마다 그 위에 파라미터
// 스코프를 새로 만들어 실행한다. 클래스 메서드 선언도 이 타입을 그대로 재사용한다.
class CodeFabFunction : public Callable {
public:
    CodeFabFunction(const FunctionStmt* declaration, shared_ptr<Environment> closure, bool is_initializer = false);

    int arity() const override;
    Value call(Interpreter& interpreter, const vector<Value>& arguments) const override;
    string toString() const override;

    // this가 바인딩된 새 closure를 갖는 메서드 복사본을 만든다(인스턴스별로 매번 새로 만들어짐).
    shared_ptr<CodeFabFunction> bind(shared_ptr<CodeFabInstance> instance) const;

private:
    const FunctionStmt* declaration;
    shared_ptr<Environment> closure;
    bool is_initializer;
};
