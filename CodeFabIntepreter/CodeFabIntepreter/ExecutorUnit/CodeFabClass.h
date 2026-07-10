#pragma once

#include "CodeFabFunction.h"

#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <unordered_map>

using std::enable_shared_from_this;
using std::shared_ptr;
using std::string;
using std::unordered_map;

class Interpreter;

// 클래스 선언 자체도 "인스턴스를 만드는 방법을 아는 Callable"이다(Robot()처럼 함수 호출 문법으로 생성).
class CodeFabClass : public Callable, public enable_shared_from_this<CodeFabClass> {
public:
    CodeFabClass(string name, shared_ptr<CodeFabClass> superclass, unordered_map<string, shared_ptr<CodeFabFunction>> methods);

    const string& getName() const;
    const shared_ptr<CodeFabClass>& getSuperclass() const;

    // 자기 자신부터 시작해 상속 체인을 거슬러 올라가며 메서드를 찾는다(오버라이딩 지원).
    shared_ptr<CodeFabFunction> findMethod(const string& name) const;

    // instanceof 판별: other가 this 자신이거나, 상속 체인 어딘가의 조상이면 true.
    bool isSubclassOf(const CodeFabClass* other) const;

    int arity() const override;
    Value call(Interpreter& interpreter, const vector<Value>& arguments) const override;
    string toString() const override;

private:
    string name;
    shared_ptr<CodeFabClass> superclass;
    unordered_map<string, shared_ptr<CodeFabFunction>> methods;
};
