# CodeFabIntepreter

CodeFab이라는 간단한 스크립트 언어를 파싱하고 실행하는 C++ 인터프리터 프로젝트입니다.
콘솔에서 한 줄씩 코드를 입력받아 즉시 실행하는 REPL(`PromptShell`) 형태로 동작합니다.

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

> **참고**: 현재 `CodeFabFacade`는 코드를 어셈블(토크나이즈+파싱)하는 단계까지 실제로 동작하며, 검사(Checker)와 실행(Executor) 단계는 아직 실제 로직과 연결되지 않은 상태입니다. `Checker`/`Interpreter`의 실제 구현은 각각의 단위 테스트(`CheckerTest.cpp`, `InterpreterTest.cpp`)를 통해 독립적으로 검증되고 있으며, 전체 파이프라인 연결은 진행 중인 작업입니다.

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
| 블록 | `{ ... }` | ✅ | 🚧 (스텁, `nullptr` 반환) | ⚠️ (로직은 있으나 Parser 미완성으로 도달 불가) | ⚠️ (로직은 있으나 Parser 미완성으로 도달 불가) |
| 조건문 | `if (...) { ... } else { ... }` | ✅ | 🚧 | ❌ | ❌ |
| 반복문 | `for (...) { ... }` | ✅ | 🚧 | ❌ | ❌ |
| 출력문 | `print ...;` | ✅ | 🚧 | ❌ | ❌ |
| 단독 표현식 문장 | `a + b;` | ✅ | 🚧 | ❌ | ❌ |

### 표현식 (Expression)

| 문법 | 예시 | Lexer | Parser | Checker | Interpreter |
| --- | --- | --- | --- | --- | --- |
| 리터럴 | `10`, `"text"`, `true` | ✅ | ✅ | ➖ | ✅ |
| 단항 연산자 | `-10`, `!true` | ✅ | ✅ | ➖ | ❌ (리터럴 외 표현식은 실행 시 예외 발생) |
| 변수 참조 | `var a = b;` | ✅ | 🚧 (개발 중) | ❌ | ❌ |
| 그룹핑 | `(10 + 1)` | ✅ | 🚧 (개발 중) | ❌ | ❌ |
| 대입 | `a = 10` | ✅ | ❌ | ❌ | ❌ |
| 이항 연산자 | `1 + 2`, `1 == 2` | ✅ | ❌ | ❌ | ❌ |
| 논리 연산자 | `a and b`, `a or b` | ✅ | ❌ | ❌ | ❌ |

지원 범위는 빠르게 변경될 수 있으므로, 최신 상태는 `AssemblerUnit/Parser/Parser.cpp`, `CheckerUnit/Checker.cpp`, `ExecutorUnit/Interpreter.cpp`와 각 단위 테스트를 함께 참고해 주세요.

## 기여 가이드

- 코드 스타일 및 컨벤션: [code_convention.md](code_convention.md)
- PR 작성 시 [PR 템플릿](.github/pull_request_template.md)에 따라 요구사항, 주요 변경점, 테스트 결과, 체크리스트를 작성해 주세요.
