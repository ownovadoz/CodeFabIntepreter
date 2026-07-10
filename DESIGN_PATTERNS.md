# 디자인 패턴 적용 현황

이 문서는 CodeFabIntepreter 프로젝트에 적용된 디자인 패턴과 그 이유를 정리합니다.
"패턴을 썼다"는 사실보다 "왜 이 패턴이 이 문제에 맞는지"를 기준으로 적었습니다.

## Visitor — AST를 순회하는 모든 로직

**어디**: `Visitor.h` (`ExprVisitor`/`StmtVisitor`), 구현체 `CheckerUnit::Checker`,
`ExecutorUnit::Interpreter`, `ExecutorUnit::LineResolver`, `ExecutorUnit::ScopeResolver`.

**문제**: `Expression`/`Statement`는 리터럴, 변수, 함수 호출, 배열 인덱싱 등 10여 종의
구체 타입을 가집니다. "이 노드가 정적 검사 대상인지", "실행하면 어떤 값이 나오는지",
"몇 번째 줄에 있는지"처럼 노드 종류마다 다르게 동작해야 하는 연산이 여러 개 있습니다.

**해결**: 각 AST 노드는 `accept(Visitor&)`만 가지고 있고, "무엇을 할지"는
`ExprVisitor`/`StmtVisitor`를 구현하는 별도 클래스(Checker, Interpreter,
LineResolver)에 둡니다. 새 노드 타입(`ArrayExpr`, `IndexExpr`, `IndexSetExpr`)이
추가되면 `ExprVisitor`에 순수 가상 함수가 하나 늘어나므로, 이를 구현하지 않은
Visitor는 **컴파일이 실패**합니다 — 노드 타입을 빠뜨리고 넘어가는 실수를 컴파일러가
잡아줍니다.

**이번에 정리한 것**: `Interpreter::resolveLine`/`resolveStatementLine`은 원래
`dynamic_cast`로 모든 구체 타입을 하나씩 확인하는 코드였습니다. Visitor가 이미
있는데도 새 노드 타입을 추가할 때마다 (1) `ExprVisitor`/`StmtVisitor`에 메서드 추가,
(2) Checker/Interpreter에 구현 추가, (3) 이 `dynamic_cast` 체인에도 case 추가라는
세 곳을 손대야 했습니다. `ExecutorUnit/LineResolver`를 `ExprVisitor`/`StmtVisitor`의
세 번째 구현체로 추가해 (3)을 없앴습니다 — 이제 새 노드 타입을 추가하면 Visitor가
누락을 컴파일 에러로 잡아주므로 갱신을 빠뜨릴 수 없습니다.

**정적 바인딩에도 재사용**: 변수 접근을 O(1)로 만들기 위해 `interpret()` 실행 직전에
AST를 한 번 훑어 변수마다 "몇 겹의 스코프를 거슬러 올라가야 하는지"를 계산해두는
`ExecutorUnit::ScopeResolver`도 같은 Visitor를 네 번째로 구현합니다. Checker도 스코프를
추적하지만 그 목적은 중복 선언/자기참조 검출이라, `if`문 분기마다 항상 스코프를 여는 등
Interpreter가 실제로 `Environment`를 만드는 시점과 스코프 경계가 다릅니다(Checker는 정적
검사 규칙 기준, ScopeResolver는 런타임 Environment 생성 시점 기준). 그래서 Checker의
스코프 로직을 재사용하지 않고 별도 Visitor로 분리했습니다.

## Command — 디버그 콘솔 명령어

**어디**: `FactoryShell/DebugCommand.h/.cpp` (`StepCommand`, `NextCommand`,
`BreakCommand`, `WatchCommand` 등), `DebugModeShell::createCommandTable()`.

**문제**: 디버그 REPL은 `step`, `next`, `break`, `watch` 등 10가지 명령어를 입력받고,
각 명령어는 `DebugModeShell`의 상태를 다르게 조작합니다.

**해결**: 명령어마다 `execute(DebugModeShell&, istringstream&) const` 하나만 구현하는
`DebugCommand` 구현체로 분리하고, 명령어 이름 → `DebugCommand` 매핑을 테이블로
구성합니다. `if/else` 사슬이 아니라 조회(lookup)로 명령어를 찾으므로, 새 명령어를
추가할 때 기존 분기 로직을 건드릴 필요가 없습니다.

## Template Method — 파일을 읽어 실행하는 셸

**어디**: `FactoryShell/FileBackedShell.h/.cpp` (기반 클래스),
`FactoryShell/FileModeShell`, `FactoryShell/DebugModeShell` (파생 클래스).

**문제**: "파일 모드"와 "디버그 모드"는 둘 다 파일 존재 확인 → 줄 단위로 읽어 하나의
소스로 합치기 → 예외를 줄 번호와 함께 보고하는 과정이 동일하고, 다른 점은 "읽은 뒤
무엇을 하는지"(디버그 모드는 훅을 걸고 멈춰서 명령을 기다림)뿐입니다.

**해결**: 공통 흐름은 `FileBackedShell::enter()`가 한 번만 구현하고, 달라지는 지점만
`afterLoad()`/`beforeExecute()` 가상 함수(기본 구현은 빈 몸통)로 갈라 하위 클래스가
필요한 것만 오버라이드합니다. 파일 읽기/예외 보고 로직이 두 클래스에 중복되지 않습니다.

## Strategy — 실행 모드 선택 (Prompt / File / Debug)

**어디**: `FactoryShell/IShellMode.h` (인터페이스), `PromptShell`, `FileModeShell`,
`DebugModeShell` (구현체), `main.cpp`에서 `ArgumentParser`가 고른 모드에 따라 하나를
생성해 `enter()`를 호출.

**문제**: 실행 방식(REPL / 파일 실행 / 디버깅)이 서로 다르지만, `main`은 "어떤
모드인지"를 몰라도 되게 하고 싶습니다.

**해결**: `IShellMode::enter()` 하나로 통일해, `main.cpp`는 구체 타입을 몰라도
`shell->enter()`만 호출하면 됩니다. 모드를 추가해도 `main`의 호출부는 바뀌지 않습니다.

## Facade — 파이프라인 진입점

**어디**: `CodeFabFacade.h/.cpp`.

**문제**: 코드 한 줄을 실행하려면 어셈블(Lexer+Parser) → 검사(Checker) → 실행
(Interpreter) 세 단계를 항상 같은 순서로, 그리고 "검사를 전부 통과해야만 실행"이라는
규칙을 지키며 호출해야 합니다.

**해결**: `CodeFabFacade::execute(code_line)` 하나가 이 세 단계를 대신 조율합니다.
`PromptShell`/`FileModeShell`/`DebugModeShell`은 각자 세 유닛을 직접 알 필요 없이
`CodeFabFacade`만 알면 됩니다.

## 의존성 주입을 통한 테스트 격리 (인터페이스 분리)

**어디**: `InterfaceForCodeFabTest.h`의 `IAssemblerUnit`/`IChecker`/`IExecutor`,
`CodeFabFacadeTest.cpp`의 `MockAssemblerUnit`/`MockChecker`/`MockExecutor`.

**문제**: `CodeFabFacade`가 세 유닛을 올바른 순서로 호출하는지 검증하려면, 실제
Lexer/Parser/Checker/Interpreter를 다 실행하지 않고 "호출 순서와 인자"만 확인하고
싶습니다.

**해결**: `_DEBUG` 빌드에서만 `CodeFabFacade`가 인터페이스 참조로 세 유닛을 가지도록
해, 테스트에서는 gmock으로 만든 Mock을 주입합니다. Release 빌드에서는 인터페이스
오버헤드 없이 구체 타입을 직접 멤버로 갖습니다(`#ifdef _DEBUG` 분기).

---

## 파일 구조 정리

이번 정리에서 함께 손본 구조적 일관성 항목:

- `AssemblerUnit/Parser/test/ParserTest.cpp` → `AssemblerUnit/Parser/ParserTest.cpp`로
  이동. 다른 모든 유닛(`CheckerTest.cpp`, `InterpreterTest.cpp`, `LexerTest.cpp`,
  `EnvironmentTest.cpp` 등)은 테스트 파일을 검사 대상 클래스와 같은 디렉토리에
  나란히 두는데, Parser만 별도 `test/` 하위 디렉토리를 쓰고 있어 통일했습니다.
- 각 모듈 디렉토리(`AssemblerUnit/`, `CheckerUnit/`, `ExecutorUnit/`, `FactoryShell/`)에는
  해당 모듈이 소유한 파일만 있고, 다른 모듈의 클래스가 잘못 들어가 있는 경우는
  없었습니다(모듈 경계는 이미 잘 지켜지고 있었음).
- 루트에 남아있는 `main.cpp`, `CodeFabFacade.*`, `Visitor.*`, `CodeFabException.h`,
  `InterfaceForCodeFabTest.h`는 특정 모듈이 아니라 여러 모듈을 조합/공유하는
  cross-cutting 코드라 루트에 있는 것이 맞습니다(어느 한 모듈 하위로 옮기면 오히려
  그 모듈이 다른 모듈에 속한 것처럼 보이는 오해를 유발합니다).

## 함께 제거한 중복 코드

- `AssemblerUnit/Parser/Parser.cpp`의 `finishIndexExpr`/`parseArrayExpr`가 각각
  "인덱스/크기가 리터럴이면서 숫자가 아니면 예외"를 거의 같은 코드로 중복
  구현하고 있어서, `Parser::rejectNonNumericLiteral(expr, message)` 헬퍼로
  추출했습니다.
