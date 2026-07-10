# CodeFabIntepreter

CodeFab이라는 팀 전용 스크립트 언어를 파싱하고 실행하는 C++ 인터프리터 프로젝트입니다.
콘솔에서 한 줄씩 코드를 입력받아 즉시 실행하는 REPL(`PromptShell`), 파일 하나를 통째로
읽어 실행하는 파일 모드(`FileModeShell`), Stmt 단위로 멈춰가며 점검하는 디버그 모드
(`DebugModeShell`)를 함께 제공합니다.

## 진행 상황

기본기능 과제(Custom Language + Code Fab Interpreter + Prompt Shell)와, 추가기능 과제 중
함수(Chapter 2)·클래스(Chapter 3)·정적 배열·파일 모드·디버그 모드(Chapter 7) 요구사항이 모두
구현되어 단위 테스트로 검증되었습니다. 실행 전 최적화, import는 아직 착수 전입니다.

### 기본기능 (완료)

**기능**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 변수 선언과 출력 | `var a = 3; print a;` → `3` 출력 | ✅ |
| 산술 연산과 우선순위 | `print 1 + 2 * 3;` → `7` 출력 (곱셈 먼저) | ✅ |
| 비교/논리 연산 | `a > 3 and b < 5` 형태의 조건식 평가 | ✅ |
| 조건 분기 실행 | `if (a > 3) { ... } else { ... }` 분기 실행 | ✅ |
| 반복 실행 | `for (var i = 0; i < 3; i = i + 1) { print i; }` → 0,1,2 순차 출력 | ✅ |
| 블록 스코프 | 중첩 `{ }` 안 변수가 바깥에서 안 보임, 바깥 변수는 안에서 조회 가능 | ✅ |
| Prompt Shell 동작 | 콘솔에서 한 줄씩 입력 → 즉시 결과 확인, `exit`/`quit`로 종료 | ✅ |

**예외처리**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 변수 중복 선언 오류 | 같은 스코프에서 같은 이름 재선언 시 오류 발생 | ✅ |
| 자기 참조 선언 오류 | `var a = a + 1;` 입력 시 오류 발생 | ✅ |
| 타입 불일치 런타임 오류 | `true * false`, `3 - "hello"` 등 입력 시 오류 발생 | ✅ |
| 미정의 변수 참조 오류 | 선언 안 한 변수 사용 시 오류 발생 | ✅ |
| 0으로 나누기 오류 | `3 / 0` 입력 시 오류 발생 | ✅ |
| 오류 후 세션 유지 | 한 줄에서 오류가 나도 Shell이 죽지 않고 다음 입력을 계속 받음 | ✅ |

**기타**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 제공된 테스트 스크립트 | gist 예시 스크립트 전체 통과 | ✅ |
| 팀명/코딩 컨벤션 정의 | [code_convention.md](code_convention.md) | ✅ |
| TDD 개발 (1~2일차) | 각 유닛에 대응하는 `*Test.cpp`를 함께 커밋 | ✅ |

### 추가기능 (진행 중)

**기능**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 함수 정의/호출/재귀 | `Func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); }` | ✅ |
| 클래스 선언/인스턴스/필드 | `Class Robot {} var r = Robot(); r.speed = 10; print r.speed;` | ✅ |
| 메서드/this | `this.position = this.position + dist;` | ✅ |
| 생성자(init) | 인스턴스 생성 시 자동 호출, 인자 전달·필드 초기화 | ✅ |
| 상속/Super/instanceof | 메서드 상속·오버라이딩, `Super.move()`, `instanceof` 판별 | ✅ |
| 정적 배열 | `Array(3)` 생성 후 `arr[i]` 읽기/쓰기 | ✅ |
| 파일 모드 | `factory run <경로>`로 스크립트 실행 | ✅ |
| 디버그 모드 | `factory debug <경로>`로 진입, step/next/break/watch/inspect 동작 | ✅ |
| 실행 전 최적화 - 정적 바인딩 | 변수 접근이 스코프 탐색 없이 O(1)로 동작 (테스트로 검증) | ⬜ |
| 실행 전 최적화 - 상수 폴딩 | 리터럴로만 이뤄진 연산식이 실행 전에 계산됨 (테스트로 검증) | ⬜ |
| import | `import "a.txt" alias a;`로 함수 가져와 `a.func()` 호출 | ⬜ |

**예외처리**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 함수 오류 처리 | 함수 밖 return, 파라미터 중복, 비함수 호출, 인자 개수 불일치 | ✅ |
| 클래스 오류 처리 | 클래스 밖 this/Super, init에서 return, 자기 상속, 없는 필드 접근 등 | ✅ |
| 파일 모드 오류 처리 | 런타임 오류 발생 시 줄 번호와 함께 출력 후 즉시 종료, 파일 없음 오류 | ✅ |
| 배열 오류 처리 | 범위 초과, 인덱스 타입 오류, 비배열 인덱싱, 크기 타입 오류 | ✅ |
| import 오류 처리 | 순환 import, 같은 scope 중복 import, 반복문 내 import | ⬜ |

**기타**

| 항목 | 시나리오 | 완료 |
|---|---|---|
| 기존 테스트 유지보수 | 새 기능 추가 후에도 기존 테스트 전부 통과 | ✅ |
| README 사용법 문서화 | 문법 사용법 + 특이사항 명시 | ✅ |
| (가산점) 디자인 패턴 적용 | Visitor, Strategy(Callable), Command(DebugCommand) 등 [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md) 참고 | ✅ |

## 아키텍처

전체 실행 흐름은 `CodeFabFacade`를 통해 아래 3단계 파이프라인으로 이뤄집니다.

```
입력 코드 문자열
      │
      ▼
┌─────────────────┐   문자열을 토큰으로 분리(Lexer)하고
│  AssemblerUnit   │   문법에 맞춰 AST(Statement/Expression)로 구성(Parser)
└─────────────────┘
      │
      ▼
┌─────────────────┐   변수 중복 선언, 자기참조, 함수/클래스 관련 정적 규칙 검사
│   CheckerUnit    │
└─────────────────┘
      │
      ▼
┌─────────────────┐   AST를 순회하며 실제로 실행
│   ExecutorUnit   │
└─────────────────┘
```

- **AssemblerUnit** (`AssemblerUnit/`)
  - `Tokenizer/Lexer` — 소스 코드를 `Token` 시퀀스로 변환합니다. 키워드/식별자/숫자/문자열/연산자/주석을 처리합니다.
  - `Parser/Parser` — 토큰을 재귀 하향 파서로 `Statement`/`Expression` AST로 구성합니다. 대입 → 논리(or/and) →
    동등 → 비교 → 항(+/-) → 인자(*//) → 단항 → 호출/필드 접근(`()`, `.`) → 1차식 순으로 연산자 우선순위를
    반영합니다. `Func` 선언과 클래스 메서드 선언은 파라미터+본문 파싱 로직을 공유합니다.
- **CheckerUnit** (`CheckerUnit/Checker`)
  - AST를 순회하며 변수/함수/클래스 선언 중복, 초기화식 자기참조, 함수 외부 return, 클래스 외부 this/Super
    사용, init 내부 return, 자기 자신 상속 등 정적 검사를 수행합니다.
  - 각 스코프는 변수 이름을 `declare`(선언됨) → `define`(정의 완료) 2단계 상태로 추적하는 Resolver
    패턴을 사용합니다. 함수/클래스 이름은 선언과 동시에 `define`되어, 재귀 호출·전방 참조가 자기참조
    오류로 오검출되지 않습니다.
  - `FunctionGuard`/`ClassGuard`(RAII)로 함수 중첩 깊이와 클래스 문맥을 추적하며, 예외가 발생해도
    항상 이전 상태로 복원됩니다.
  - PromptShell처럼 한 줄씩 나뉘어 들어오는 입력에서도 전역 스코프를 유지해, 여러 번의 `check()`
    호출에 걸쳐서도 전역 변수 중복 선언을 검출합니다.
- **ExecutorUnit** (`ExecutorUnit/`)
  - `Interpreter` — Visitor 패턴으로 AST를 실행하며 `Environment`(변수 스코프 체인)를 통해 값을 읽고 씁니다.
    디버그 모드를 위해 문장 실행 직전 훅(`setBeforeStatementHook`)과 현재 보이는 모든 변수를 열거하는
    `inspectVariables()`도 제공합니다.
  - `LineResolver` — AST 노드의 소스 코드 줄 번호를 조회하는 전용 Visitor입니다. 예전에는 Interpreter가
    Expression/Statement의 모든 구체 타입을 `dynamic_cast`로 하나씩 확인했는데, `ExprVisitor`/`StmtVisitor`의
    세 번째 구현체로 분리해 새 노드 타입이 추가되면 컴파일러가 누락을 잡아주도록 했습니다. 예외 메시지와
    디버그 모드의 정지 위치 표시에 쓰입니다.
  - `Environment` — 블록 진입/종료마다 생성·소멸하는 지역 스코프 체인이며, 변수 조회는 가장 안쪽
    스코프부터 전역 스코프까지 거슬러 올라가며 탐색합니다.
  - `Callable`(`Value.h`) — 함수·클래스·인스턴스처럼 CodeFab 코드에서 호출 가능한(혹은 그 슬롯을
    공유하는) 값이 구현하는 Strategy 패턴 인터페이스입니다. Interpreter는 `arity()`/`call()`만으로
    실제로 무엇이 호출되는지 몰라도 호출을 위임할 수 있습니다.
  - `CodeFabFunction` — `Func` 선언과 클래스 메서드가 공유하는 `Callable` 구현체입니다. 선언 AST와
    정의 시점의 `Environment`(closure)를 들고 있다가 호출 시 새 스코프에서 본문을 실행합니다. `return`은
    예외(`ReturnSignal`)로 호출 경계까지 되감아 처리합니다. 클래스 메서드로 쓰일 때는 `bind()`로 `this`가
    바인딩된 복사본을 만들어 인스턴스별로 반환합니다.
  - `CodeFabClass`/`CodeFabInstance` — 클래스는 자신을 호출하면 인스턴스를 생성하는 `Callable`이고
    (`Robot()`), 인스턴스는 필드를 동적으로 저장합니다. 상속은 `CodeFabClass`가 부모를 가리키는 체인으로
    표현되며, `Super` 호출은 클래스 선언 시 그 클래스의 모든 메서드가 공유하는 environment에 `"super"`를
    바인딩해두고 실행 시점에 동적으로 해석합니다.
- **CodeFabFacade**
  - 위 세 유닛을 조합해 한 줄(혹은 한 파일)의 코드를 어셈블 → 검사 → 실행까지 한 번에 처리하는 진입점입니다.
  - 조립된 문장 전체를 먼저 전부 검사한 뒤에야 실행에 들어가므로, 뒤쪽 문장의 오류 때문에 앞쪽 문장이
    이미 실행되어 버리는 일이 없습니다.
  - 실행된 문장(AST)을 Facade가 살아있는 동안 계속 보관합니다. `Func`/`Class` 선언은 클로저가 원본
    AST를 가리키므로, PromptShell처럼 한 줄씩 나뉘어 실행되어도 이후 줄에서 계속 호출할 수 있어야
    하기 때문입니다.
- **FactoryShell**
  - `PromptShell` — 표준 입력으로부터 코드를 한 줄씩 읽어 `CodeFabFacade`에 전달하는 REPL 셸입니다.
  - `FileBackedShell` — 파일 로딩, 전체 소스 실행, 줄 번호 포함 오류 처리처럼 `FileModeShell`과
    `DebugModeShell`이 공유하는 로직을 모아둔 Template Method 베이스 클래스입니다.
  - `FileModeShell` — 파일 경로를 받아 전체 소스를 실행하고, 런타임 오류 발생 시 줄 번호와 함께
    출력한 뒤 즉시 종료합니다.
  - `DebugModeShell` / `DebugCommand` — Stmt 단위로 실행을 멈춰가며 점검하는 디버그 콘솔입니다.
    각 명령어(`step`/`next`/`continue`/`break`/`remove`/`breakpoints`/`watch`/`unwatch`/`watches`/
    `inspect`)는 Command 패턴으로 구현되어 있습니다.
  - `ArgumentParser` — 커맨드라인 인자를 해석해 Prompt/File/Debug 모드를 결정합니다(`run <경로>` →
    파일 모드, `debug <경로>` → 디버그 모드, 인자 없음 → 프롬프트 모드).
  - 적용된 디자인 패턴과 그 이유는 [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md)에 정리했습니다.

## 프로젝트 구조

```
CodeFabIntepreter/                 (레포 루트)
├── code_convention.md            # 팀 코드 컨벤션
├── DESIGN_PATTERNS.md            # 적용된 디자인 패턴과 그 이유
├── README.md
└── CodeFabIntepreter/             # Visual Studio 솔루션 디렉터리
    ├── CodeFabIntepreter.slnx
    └── CodeFabIntepreter/         # 프로젝트 디렉터리
        ├── AssemblerUnit/
        │   ├── Tokenizer/         # Lexer, Token, Value(+ Callable 인터페이스)
        │   └── Parser/            # Parser, Node, Statement, Expression (+ ParserTest)
        ├── CheckerUnit/           # Checker (+ CheckerTest)
        ├── ExecutorUnit/          # Interpreter, LineResolver, Environment, CodeFabFunction,
        │                          # CodeFabClass, CodeFabInstance (+ 각 Test)
        ├── FactoryShell/          # PromptShell, FileBackedShell, FileModeShell,
        │                          # DebugModeShell, DebugCommand, ArgumentParser (+ 각 Test)
        ├── CodeFabFacade.*        # 파이프라인 진입점 (+ Test)
        ├── CodeFabException.h     # 공용 예외 타입 (+ Test)
        ├── Visitor.*              # Expr/Stmt Visitor 인터페이스
        └── main.cpp
```

각 유닛에는 대응하는 `*Test.cpp`가 같은 디렉터리에 위치하며, gtest/gmock으로 작성되어 있습니다.

## 빌드 및 실행

Visual Studio에서 `CodeFabIntepreter/CodeFabIntepreter.slnx` 솔루션을 열어 빌드합니다.
NuGet(`packages.config`)을 통해 gmock/gtest 패키지가 자동으로 복원됩니다.

- **Debug 빌드**: `main.cpp`에서 `_DEBUG`가 정의되어 있으면 REPL 대신 gtest 기반 전체 테스트 스위트(`RUN_ALL_TESTS`)가 실행됩니다.
- **Release 빌드**: 커맨드라인 인자에 따라 아래 세 모드 중 하나로 동작합니다.
  - 인자 없음 → `PromptShell` 실행 (표준 입력으로 코드를 한 줄씩 입력받아 실행)
  - `run <파일경로>` → `FileModeShell` 실행 (해당 파일을 전체 실행, 파일이 없으면 오류)
  - `debug <파일경로>` → `DebugModeShell` 실행 (Stmt 단위로 멈춰가며 점검)

## 테스트

이 프로젝트는 gtest/gmock 기반으로 각 유닛(Tokenizer, Parser, Checker, Interpreter, Environment,
CodeFabFunction, CodeFabClass, CodeFabInstance, Facade, ArgumentParser, PromptShell, FileModeShell,
DebugModeShell 등)별 단위 테스트를 갖추고 있습니다. Debug 구성으로 빌드/실행하면 전체 테스트가
수행됩니다(현재 339개 테스트 전부 통과).

## 사용법

### Prompt 모드

Release 구성으로 인자 없이 실행하면 `PromptShell`이 콘솔에 `>` 프롬프트를 띄우고, 한 줄씩 입력받은
코드를 `CodeFabFacade`에 전달해 처리합니다.

```
CodeFab Interpreter - exit / quit로 종료
> var a = 10;
> print a + 5;
15
> exit
CodeFab Interpreter Exit
```

- 입력은 한 줄 단위로 처리되며, 각 문장은 `;`(세미콜론)으로 끝나야 합니다.
- 한 줄에 세미콜론으로 구분된 문장이 여러 개 있어도(예: `var a = 3; a = a + 4; { var b = 3; } print b;`) 전부
  순서대로 검사(Checker)를 통과한 뒤 순서대로 실행(Executor)됩니다.
- 한 줄 안에 이스케이프된 `\n` 문자열이 있으면 그 이전까지만 코드로 처리합니다.
- `exit` 또는 `quit`(소문자)을 입력하거나 EOF(Ctrl+D / Ctrl+Z)를 보내면 REPL이 종료됩니다.
- 파싱/검사/실행 과정에서 오류가 발생하면 예외 메시지가 표준 에러로 출력되고, REPL은 계속 다음 줄을 입력받습니다.
- `Func`/`Class` 선언은 한 줄에 나눠 입력해도(먼저 선언하고 다음 줄에서 호출) 세션 동안 계속 유효합니다.

### 파일 모드

```
CodeFabIntepreter run script.txt
```

파일 하나를 통째로 실행합니다. 파일이 없으면 오류 메시지를 출력하고, 실행 중 런타임 오류가 발생하면
오류가 난 줄 번호와 함께 출력한 뒤 즉시 종료합니다.

### 디버그 모드

```
CodeFabIntepreter debug script.txt
```

소스 코드를 Stmt 단위로 멈춰가며 점검합니다.

| 명령어 | 설명 |
|---|---|
| `step` | 현재 Stmt 실행 후 다음 Stmt에서 정지 |
| `next` | 현재 Stmt 실행(블록 내부로는 진입하지 않음) |
| `break <줄번호>` | 해당 줄에 breakpoint 설정 |
| `breakpoints` | 현재 설정된 breakpoint 목록 출력 |
| `remove <줄번호>` | breakpoint 해제 |
| `continue` | 다음 breakpoint까지 실행 |
| `watch <변수명>` | 해당 변수를 감시 목록에 추가 |
| `unwatch <변수명>` | 감시 목록에서 제거 |
| `watches` | 현재 감시 중인 변수 목록과 값 출력 |
| `inspect` | 현재 스코프의 모든 변수와 값 출력 |

### 함수와 클래스

```
Func add(a, b) {
    return a + b;
}
print add(3, 7);        // 10

Func fact(n) {
    if (n <= 1) return 1;
    return n * fact(n - 1);
}
print fact(5);           // 120

Class Robot {
    init(name, speed) {
        this.name = name;
        this.speed = speed;
    }
    move(dist) {
        this.speed = this.speed + dist;
    }
    report() {
        print this.name;
        print this.speed;
    }
}

var r = Robot("AndOr", 10);
r.move(5);
r.report();               // AndOr / 15

Class SpeedRobot : Robot {
    move(dist) {
        Super.move(dist);
        print "Speeeed!";
    }
}

var w = SpeedRobot("Sam", 1);
w.move(3);
print (w instanceof SpeedRobot);   // true
print (w instanceof Robot);        // true
```

- 함수/클래스 이름은 선언 즉시 사용 가능하므로 재귀 호출·전방 참조가 허용됩니다.
- `init`은 인스턴스 생성 시 자동 호출되며 `return`을 쓸 수 없고, 항상 생성된 인스턴스를 반환합니다.
- 메서드 안에서는 `this`로 자기 인스턴스를, `Super.method(...)`로 부모 클래스의 메서드를 호출할 수 있습니다.
- `instanceof`는 자기 자신의 클래스뿐 아니라 상속 체인상의 조상 클래스에 대해서도 `true`를 반환합니다.
- 문자열 리터럴은 반드시 직선 큰따옴표(`"`)를 사용해야 합니다. 편집기의 자동 교정으로 곡선따옴표(`“`/`”`)가
  들어가면 Lexer가 `Unexpected character` 오류를 냅니다.

### 정적 배열

```
var arr = Array(3);
arr[0] = 10;
arr[1] = 20;
arr[2] = 30;
print arr[0];        // 10

var i = 2;
arr[i - 1] = 7;
print arr[1];         // 7
```

- `Array(n)`은 크기가 `n`인 고정 크기 배열을 생성하며, 크기는 0 이상의 정수여야 합니다.
- 인덱스는 정수여야 하며 `0`부터 `size - 1`까지의 범위를 벗어나면 실행 시 예외가 발생합니다.
- 배열은 참조로 공유되므로, 다른 변수에 대입해도 같은 배열을 가리킵니다.

## 제약사항 
- **문법 제약**: {}는 한 line에 같이 있어야합니다. 예) { print "Hello, " + name + "!"; }
- **문법 제약**: Class에서 this 사용시 반드시 this를 사용해야합니다. 예) This (X)


## 문법 지원 범위

CodeFab 언어의 기본기능 문법과, 추가기능 함수(Chapter 2)·클래스(Chapter 3)·정적 배열 문법은 Lexer → Parser → Checker →
Interpreter 전 단계에 걸쳐 구현이 완료되어 단위 테스트로 검증되었습니다.

### 토큰 / 리터럴

| 문법 | 예시 |
| --- | --- |
| 숫자 리터럴 | `10`, `3.14` |
| 문자열 리터럴 | `"text"` |
| 불리언 리터럴 | `true`, `false` |
| 한 줄 주석 | `// comment` |

### 문장 (Statement)

| 문법 | 예시 |
| --- | --- |
| 변수 선언 | `var a = 10;` |
| 블록 | `{ ... }` (독립된 스코프 생성/소멸) |
| 조건문 | `if (...) { ... } else { ... }` |
| 반복문 | `for (var i = 0; i < 3; i = i + 1) { ... }` |
| 출력문 | `print ...;` |
| 단독 표현식 문장 | `a + b;` |
| 함수 선언 | `Func add(a, b) { ... }` |
| return | `return;` / `return a + b;` |
| 클래스 선언 | `Class Robot { ... }` / `Class SpeedRobot : Robot { ... }` |

### 표현식 (Expression)

| 문법 | 예시 |
| --- | --- |
| 리터럴 | `10`, `"text"`, `true` |
| 변수 참조 | `a` |
| 대입 | `a = 10` |
| 이항 연산자 | `1 + 2`, `a > 0`, `1 == 2` (연산자 우선순위 반영) |
| 단항 연산자 | `-10`, `!true` |
| 논리 연산자 (단락 평가) | `a and b`, `a or b` |
| 그룹핑 | `(1 + 2)` |
| 함수/메서드 호출 | `add(1, 2)`, `r.move(5)`, 연쇄 호출 `f()()` |
| 필드 읽기/쓰기 | `r.speed`, `r.speed = 10` |
| this / Super | `this.position`, `Super.move(dist)` |
| instanceof | `w instanceof Robot` |
| 배열 생성 | `Array(3)` |
| 배열 인덱스 읽기/쓰기 | `arr[i]`, `arr[i] = 7`, 연쇄/중첩 인덱싱 `arr[0][1]`, `f()[0]` |

최신 구현은 `AssemblerUnit/Parser/Parser.cpp`, `CheckerUnit/Checker.cpp`,
`ExecutorUnit/Interpreter.cpp`와 각 단위 테스트를 함께 참고해 주세요.

## 기여 가이드

- 코드 스타일 및 컨벤션: [code_convention.md](code_convention.md)
- 적용된 디자인 패턴과 그 이유: [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md)
- PR 작성 시 [PR 템플릿](.github/pull_request_template.md)에 따라 요구사항, 주요 변경점, 테스트 결과, 체크리스트를 작성해 주세요.
