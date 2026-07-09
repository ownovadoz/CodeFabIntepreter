# CodeFabIntepreter

CodeFab이라는 간단한 스크립트 언어를 파싱하고 실행하는 C++ 인터프리터 프로젝트입니다.
콘솔에서 한 줄씩 코드를 입력받아 즉시 실행하는 REPL(`PromptShell`) 형태로 동작합니다.

## 요구사항

아래는 1일차/3일차 과제 문서에서 도출한 통합(End-to-End) 요구사항 체크리스트입니다.
유닛별 구현 현황은 [문법 지원 범위](#문법-지원-범위) 표를 참고하세요.

### 1일차

**기능**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 변수 선언과 출력 | `var a = 3; print a;` → `3` 출력 | [ ] |
| 산술 연산과 우선순위 | `print 1 + 2 * 3;` → `7` 출력 (곱셈 먼저) | [ ] |
| 비교/논리 연산 | `a > 3 and b < 5` 형태의 조건식 평가 | [ ] |
| 조건 분기 실행 | `if (a > 3) { ... } else { ... }` 분기 실행 | [ ] |
| 반복 실행 | `for (var i = 0; i < 3; i = i + 1) { print i; }` → 0,1,2 순차 출력 | [ ] |
| 블록 스코프 | 중첩 `{ }` 안 변수가 바깥에서 안 보임, 바깥 변수는 안에서 조회 가능 | [ ] |
| Prompt Shell 동작 | 콘솔에서 한 줄씩 입력 → 즉시 결과 확인, `exit`로 종료 | [ ] |

**예외처리**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 변수 중복 선언 오류 | 같은 스코프에서 같은 이름 재선언 시 오류 발생 | [ ] |
| 자기 참조 선언 오류 | `var a = a + 1;` 입력 시 오류 발생 | [ ] |
| 타입 불일치 런타임 오류 | `true * false`, `3 - "hello"` 등 입력 시 오류 발생 | [ ] |
| 미정의 변수 참조 오류 | 선언 안 한 변수 사용 시 오류 발생 | [ ] |
| 0으로 나누기 오류 | `3 / 0` 입력 시 오류 발생 | [ ] |
| 오류 후 세션 유지 | 한 줄에서 오류가 나도 Shell이 죽지 않고 다음 입력을 계속 받음 | [ ] |

**기타**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 제공된 테스트 스크립트 | gist 예시 스크립트 전체 통과 | [ ] |
| 팀명/코딩 컨벤션 정의 | | [ ] |
| TDD 개발 (1~2일차) | | [ ] |

### 3일차

**기능**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 함수 정의/호출/재귀 | `Func fact(n) { if(n<=1) return 1; return n*fact(n-1); }` 정상 동작 | [ ] |
| 클래스 인스턴스/필드 | `Class Robot{} var r=Robot(); r.speed=10; print r.speed;` 동작 | [ ] |
| 메서드/this | 메서드 안에서 `this`로 필드 읽고 쓰기 | [ ] |
| 생성자(init) | 생성 시 인자 전달·필드 초기화 | [ ] |
| 상속/Super/instanceof | 메서드 상속·오버라이딩, `Super.move()`, `instanceof` 판별 | [ ] |
| 정적 배열 | `Array(3)` 생성 후 `arr[i]` 읽기/쓰기 | [ ] |
| 실행 전 최적화 - 정적 바인딩 | 변수 접근이 스코프 탐색 없이 O(1)로 동작 (테스트로 검증) | [ ] |
| 실행 전 최적화 - 상수 폴딩 | 리터럴로만 이뤄진 연산식이 실행 전에 계산됨 (테스트로 검증) | [ ] |
| import | `import "a.txt" alias a;`로 함수 가져와 `a.func()` 호출 | [ ] |
| 파일 모드 | `factory run <경로>`로 스크립트 실행 | [ ] |
| 디버그 모드 | `factory debug <경로>`로 진입, step/break/watch/inspect 동작 | [ ] |

**예외처리**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 함수 오류 처리 | 함수 밖 return, 파라미터 중복, 비함수 호출, 인자 개수 불일치 | [ ] |
| 클래스 오류 처리 | 클래스 밖 this/Super, init에서 return, 자기 상속, 없는 필드 접근 등 | [ ] |
| 배열 오류 처리 | 범위 초과, 인덱스 타입 오류, 비배열 인덱싱, 크기 타입 오류 | [ ] |
| import 오류 처리 | 순환 import, 같은 scope 중복 import, 반복문 내 import | [ ] |
| 파일 모드 오류 처리 | 런타임 오류 발생 시 줄 번호와 함께 출력 후 즉시 종료, 파일 없음 오류 | [ ] |

**기타**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 기존 테스트 유지보수 | 새 기능 추가 후에도 기존 테스트 전부 통과 | [ ] |
| README 사용법 문서화 | 문법 사용법 + 특이사항 명시 | [ ] |
| (가산점) 디자인 패턴 적용 | Visitor/Interpreter/Command/Strategy 등 | [ ] |

## 아키텍처

전체 실행 흐름은 `CodeFabFacade`를 통해 아래 4단계 파이프라인으로 이뤄집니다.

```
입력 코드 문자열
      │
      ▼
┌─────────────────┐   문자열을 토큰으로 분리(Lexer)하고
│  AssemblerUnit   │   문법에 맞춰 AST(Statement/Expression)로 구성(Parser)
└─────────────────┘
      │
      ▼
┌─────────────────┐   변수 선언/스코프 등 정적 규칙 검사
│   CheckerUnit    │
└─────────────────┘
      │
      ▼
┌─────────────────┐   AST를 순회하며 실제로 실행
│   ExecutorUnit   │
└─────────────────┘
```

- **AssemblerUnit** (`AssemblerUnit/`)
  - `Tokenizer/Lexer` — 소스 코드를 `Token` 시퀀스로 변환합니다.
  - `Parser/Parser` — 토큰을 `Statement`/`Expression` AST로 구성합니다.
- **CheckerUnit** (`CheckerUnit/Checker`)
  - AST를 순회하며 변수 선언 중복, 스코프 규칙 등 정적 검사를 수행합니다.
  - 각 스코프는 변수 이름을 `declare`(선언됨) → `define`(정의 완료) 2단계 상태로 추적하는
    Resolver 패턴을 사용합니다. 초기화식을 검사하는 동안 자신의 이름이 아직 `define`되지
    않은 상태로 스코프에 남아있으면 자기참조로 판단해 오류를 발생시킵니다.
- **ExecutorUnit** (`ExecutorUnit/`)
  - `Interpreter` — AST를 실행하며 `Environment`(변수 스코프 체인)를 통해 값을 읽고 씁니다.
- **CodeFabFacade**
  - 위 세 유닛을 조합해 한 줄의 코드를 어셈블 → 검사 → 실행까지 한 번에 처리하는 진입점입니다.
- **PromptShell**
  - 표준 입력으로부터 코드를 한 줄씩 읽어 `CodeFabFacade`에 전달하는 REPL 셸입니다.

## 프로젝트 구조

```
CodeFabIntepreter/
└── CodeFabIntepreter/
    ├── AssemblerUnit/
    │   ├── Tokenizer/     # Lexer, Token, Value
    │   └── Parser/        # Parser, Statement, Expression
    ├── CheckerUnit/       # Checker
    ├── ExecutorUnit/      # Interpreter, Environment
    ├── CodeFabFacade.*    # 파이프라인 진입점
    ├── PromptShell.*      # REPL 셸
    └── main.cpp
```

각 유닛에는 대응하는 `*Test.cpp`가 함께 위치하며, gtest/gmock으로 작성되어 있습니다.

## 빌드 및 실행

Visual Studio에서 `CodeFabIntepreter.slnx` 솔루션을 열어 빌드합니다. NuGet(`packages.config`)을 통해 gmock/gtest 패키지가 자동으로 복원됩니다.

- **Debug 빌드**: `main.cpp`에서 `_DEBUG`가 정의되어 있으면 REPL 대신 gtest 기반 전체 테스트 스위트(`RUN_ALL_TESTS`)가 실행됩니다.
- **Release 빌드**: `PromptShell`이 실행되어 표준 입력으로 CodeFab 코드를 한 줄씩 입력받아 실행합니다.

## 테스트

이 프로젝트는 gtest/gmock 기반으로 각 유닛(Tokenizer, Parser, Checker, Interpreter, Environment, Facade 등)별 단위 테스트를 갖추고 있습니다. Debug 구성으로 빌드/실행하면 전체 테스트가 수행됩니다.

## 사용법

Release 구성으로 실행하면 `PromptShell`이 콘솔에 `>` 프롬프트를 띄우고, 한 줄씩 입력받은 코드를 `CodeFabFacade`에 전달해 처리합니다.

```
CodeFab Interpreter - Ctrl+D / Ctrl+Z / exit / EXIT 후 enter로 종료
> var a = 10;
> exit
CodeFab Interpreter Exit
```

- 입력은 한 줄 단위로 처리되며, 각 문장은 `;`(세미콜론)으로 끝나야 합니다.
- 한 줄 안에 이스케이프된 `\n` 문자열이 있으면 그 이전까지만 코드로 처리합니다.
- `exit`, `EXIT`를 입력하거나 EOF(Ctrl+D / Ctrl+Z)를 보내면 REPL이 종료됩니다.
- 파싱/검사 과정에서 오류가 발생하면 예외 메시지가 표준 에러로 출력되고, REPL은 계속 다음 줄을 입력받습니다.

> **참고**: `CodeFabFacade`는 어셈블(토크나이즈+파싱) → 검사(Checker) → 실행(Executor) 순으로 실제로 연결되어 동작합니다. `Checker`는 현재 문법이 지원하는 모든 문장/표현식을 순회하며 변수 중복 선언과 초기화식 자기참조를 검사하고, PromptShell처럼 한 줄씩 나뉘어 들어오는 입력에서도 전역 스코프를 유지합니다. `Executor`는 아직 일부 문장/표현식만 실행 가능하며, 실제 구현은 단위 테스트(`InterpreterTest.cpp`)를 통해 독립적으로 검증되고 있습니다.

## 문법 지원 범위

CodeFab 언어는 아직 개발 초기 단계이며, 문법 요소마다 파이프라인의 어느 단계까지 실제로 구현되어 있는지가 다릅니다. 아래 표는 각 유닛(`Lexer` → `Parser` → `Checker` → `Interpreter`)이 해당 문법을 실제로 처리하는지를 기준으로 정리한 것입니다.

**상태 범례**

| 기호 | 의미 |
| --- | --- |
| ✅ | 해당 유닛에서 완전히 처리됨 |
| ⚠️ | 부분적으로만 동작하거나 제약이 있음 |
| 🚧 | 스텁만 존재 (구현부가 비어있거나 `nullptr` 반환) |
| ❌ | 아직 처리 로직 없음 |
| ➖ | 해당 유닛과 무관 (검사/실행 대상이 아님) |

### 토큰 / 리터럴

| 문법 | 예시 | Lexer | Parser | Checker | Interpreter |
| --- | --- | --- | --- | --- | --- |
| 숫자 리터럴 | `10`, `3.14` | ✅ | ✅ | ➖ | ✅ |
| 문자열 리터럴 | `"text"` | ✅ | ✅ | ➖ | ✅ |
| 불리언 리터럴 | `true`, `false` | ✅ | ✅ | ➖ | ✅ |
| 한 줄 주석 | `// comment` | ✅ (무시) | ➖ | ➖ | ➖ |

### 문장 (Statement)

| 문법 | 예시 | Lexer | Parser | Checker | Interpreter |
| --- | --- | --- | --- | --- | --- |
| 변수 선언 | `var a = 10;` | ✅ | ✅ | ✅ (동일 스코프 중복 선언 및 초기화식 자기참조 검사) | ⚠️ (초기값이 리터럴일 때만 정상 동작) |
| 블록 | `{ ... }` | ✅ | 🚧 (스텁, `nullptr` 반환) | ✅ (블록마다 독립된 스코프 생성/소멸) | ⚠️ (로직은 있으나 Parser 미완성으로 도달 불가) |
| 조건문 | `if (...) { ... } else { ... }` | ✅ | 🚧 | ✅ (조건식 순회 및 then/else 분기별 독립 스코프) | ❌ |
| 반복문 | `for (...) { ... }` | ✅ | 🚧 | ✅ (초기화 변수를 위한 자체 스코프 포함 전체 순회) | ❌ |
| 출력문 | `print ...;` | ✅ | 🚧 | ✅ (출력 표현식 순회) | ❌ |
| 단독 표현식 문장 | `a + b;` | ✅ | 🚧 | ✅ (표현식 순회) | ❌ |

### 표현식 (Expression)

| 문법 | 예시 | Lexer | Parser | Checker | Interpreter |
| --- | --- | --- | --- | --- | --- |
| 리터럴 | `10`, `"text"`, `true` | ✅ | ✅ | ➖ (순회만 수행, 별도 규칙 없음) | ✅ |
| 단항 연산자 | `-10`, `!true` | ✅ | ✅ | ➖ (순회만 수행, 별도 규칙 없음) | ❌ (리터럴 외 표현식은 실행 시 예외 발생) |
| 변수 참조 | `var a = b;` | ✅ | 🚧 (개발 중) | ➖ (자기참조 검사를 위한 참조 수집 대상) | ❌ |
| 그룹핑 | `(10 + 1)` | ✅ | 🚧 (개발 중) | ➖ (순회만 수행, 별도 규칙 없음) | ❌ |
| 대입 | `a = 10` | ✅ | ❌ | ➖ (순회만 수행, 별도 규칙 없음) | ❌ |
| 이항 연산자 | `1 + 2`, `1 == 2` | ✅ | ❌ | ➖ (순회만 수행, 별도 규칙 없음) | ❌ |
| 논리 연산자 | `a and b`, `a or b` | ✅ | ❌ | ➖ (순회만 수행, 별도 규칙 없음) | ❌ |

지원 범위는 빠르게 변경될 수 있으므로, 최신 상태는 `AssemblerUnit/Parser/Parser.cpp`, `CheckerUnit/Checker.cpp`, `ExecutorUnit/Interpreter.cpp`와 각 단위 테스트를 함께 참고해 주세요.

## 기여 가이드

- 코드 스타일 및 컨벤션: [code_convention.md](code_convention.md)
- PR 작성 시 [PR 템플릿](.github/pull_request_template.md)에 따라 요구사항, 주요 변경점, 테스트 결과, 체크리스트를 작성해 주세요.
