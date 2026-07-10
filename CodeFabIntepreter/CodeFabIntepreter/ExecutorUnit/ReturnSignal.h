#pragma once

#include "../AssemblerUnit/Tokenizer/Value.h"

#include <utility>

using std::move;

// return문 실행 시 함수 호출 스택을 즉시 빠져나가기 위해 던지는 제어 흐름 신호.
// 에러가 아니므로 CodeFabException과 별도의 타입으로 둔다.
class ReturnSignal {
public:
    explicit ReturnSignal(Value value) : value{ move(value) } {}

    const Value& getValue() const {
        return value;
    }

private:
    Value value;
};
