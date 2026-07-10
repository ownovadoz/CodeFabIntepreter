# CodeFabIntepreter 설계 문서

이 문서는 현재 구현된 코드를 기준으로 역으로 정리한 설계 문서입니다. "어떻게 만들 것인가"를
미리 정한 스펙이 아니라, "실제로 어떻게 만들어져 있는가"를 사후에 구조화한 자료이므로, 코드와
문서가 어긋나면 코드가 우선합니다. 사용법/문법 예시는 [README.md](README.md), 적용된 디자인
패턴과 그 근거는 [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md)에 별도로 정리되어 있으며, 이 문서는
그 둘을 아우르는 전체 아키텍처와 각 모듈의 내부 설계에 집중합니다.

## 1. 목적과 범위

CodeFab이라는 자체 스크립트 언어를 위한 Tree-walking 인터프리터를 C++20으로 구현한 프로젝트입니다.
아래 세 가지 실행 방식을 하나의 코어(어셈블 → 검사 → 실행 파이프라인) 위에서 제공합니다.

| 실행 방식 | 진입 클래스 | 용도 |
|---|---|---|
| REPL | `PromptShell` | 표준 입력에서 한 줄씩 읽어 즉시 실행 |
| 파일 실행 | `FileModeShell` | 파일 하나를 통째로 읽어 실행 |
| 디버그 | `DebugModeShell` | 문장(Statement) 단위로 멈춰가며 점검 |

지원하는 언어 기능은 변수/제어문/함수/재귀/클래스/상속/정적 배열/import이며, 실행 전 최적화로
정적 바인딩과 상수 폴딩을 수행합니다. 기능별 완료 현황은 README.md의 표를 참고하십시오.

## 2. 전체 아키텍처

```
소스 문자열
   │
   ▼
┌───────────────┐  Lexer: 문자열 → Token 시퀀스
│ AssemblerUnit │  Parser: Token → AST(Statement/Expression), 재귀 하향 파싱
└───────────────┘
   │  vector<unique_ptr<Statement>>
   ▼
┌───────────────┐  Checker: 스코프/선언 규칙 정적 검사 (Visitor)
│  CheckerUnit  │  (여기서 예외가 나면 아래 실행 단계로 넘어가지 않음)
└───────────────┘
   │
   ▼
┌───────────────┐  Interpreter::interpret() 내부에서 순서대로:
│ ExecutorUnit  │    1) Resolver.resolve()       - 정적 바인딩(거리 계산)
│               │    2) ConstantFolder.fold()    - 리터럴 연산 미리 계산
│               │    3) 문장 순차 실행(Visitor)   - Environment에 값을 읽고 씀
└───────────────┘
```

이 세 유닛을 조합해 "코드 한 뭉치(한 줄 또는 파일 전체)를 어셈블→검사→실행까지 한 번에
처리"하는 진입점이 `CodeFabFacade`이며, `FactoryShell` 하위의 세 Shell 클래스가 각자의 방식으로
`CodeFabFacade::execute()`를 호출합니다.

핵심 설계 원칙 두 가지가 파이프라인 순서를 지배합니다.

- **검사 후 실행 분리**: `CodeFabFacade::execute()`는 조립된 문장 전체를 먼저 전부 `Checker`로
  검사한 뒤에야 `Interpreter::interpret()`을 호출합니다. 뒤쪽 문장의 정적 오류 때문에 앞쪽 문장이
  이미 실행되어 버리는 일이 없도록 하기 위함입니다.
- **REPL 한 줄 = 독립된 통계 단위, 그러나 스코프는 영속**: `PromptShell`은 매 줄마다 새로운
  `assemble()` 호출로 disjoint한 AST를 만들지만, `Checker`/`Interpreter`(및 `Resolver`)는 매 줄마다
  새로 만들지 않고 세션 동안 하나의 인스턴스를 계속 재사용합니다. 그래서 한 줄에서 선언한 전역
  변수/함수/클래스를 다음 줄에서 참조할 수 있고, 전역 스코프의 중복 선언도 줄이 나뉘어도 검출됩니다.

## 3. AssemblerUnit — 문자열을 AST로

### 3.1 Tokenizer

- `Lexer`(`Tokenizer/Lexer.h/.cpp`): 소스 문자열을 한 글자씩 훑어 `Token` 시퀀스로 변환합니다.
  숫자/문자열/식별자/키워드/연산자/한 줄 주석(`//`)을 처리하며, 키워드는
  `unordered_map<string_view, TokenType>` 테이블(`scanIdentifier` 내부)로 식별합니다.
- `Token`(`Tokenizer/Token.h`): `TokenType type`, `string lexeme`, `Value literal`, `int line`을
  갖는 값 타입입니다. `TokenType`은 구분자/연산자/리터럴/키워드/`END_OF_FILE`로 나뉘며, 새 키워드를
  추가할 때는 (1) enum에 추가, (2) `tokenTypeToString`에 case 추가, (3) `Lexer`의 키워드 테이블에
  등록, (4) `Parser::parseStatement()`의 dispatch switch(아래 3.2)에 반영이 필요합니다.

### 3.2 Parser

재귀 하향 파서(`Parser.h/.cpp`)이며, `parse(tokens)`가 `parseStatement()`를 반복 호출해
최상위 `vector<unique_ptr<Statement>>`를 만듭니다. `parseStatement()`는 현재 토큰 타입으로
분기하는 **완전 열거(exhaustive) switch**입니다 — `default:`가 없으므로 `TokenType`에 새 값을
추가하면 이 switch를 반드시 갱신해야 하며, 문장을 시작할 수 없는 토큰(예: `ALIAS`, `INSTANCEOF`,
연산자류)은 `throw CodeFabException(token, "Unexpected token.")` 목록에 명시적으로 나열됩니다.

식(Expression) 파싱은 우선순위가 낮은 것부터 높은 것 순으로 아래 체인을 통해 이뤄집니다
(각 단계는 자기보다 우선순위 높은 단계를 피연산자로 받는 좌결합 파서, `parseLeftAssocExpr`
헬퍼로 통일):

```
expression
  → assignment            (=, 우결합, 좌변이 Variable/Get/Index여야 함)
    → logic_or             (or)
      → logic_and           (and)
        → equality           (==, !=)
          → instanceof         (instanceof, 좌결합 반복)
            → comparison         (>, >=, <, <=)
              → term               (+, -)
                → factor             (*, /)
                  → unary              (단항 -, ! / 우결합 재귀)
                    → call                (함수 호출 (), 인덱싱 [], 필드 접근 .의 임의 연쇄)
                      → primary             (리터럴, 식별자, 그룹핑, this, Super, Array(n))
```

대입식은 좌변을 먼저 일반식으로 파싱한 뒤 `=` 뒤를 재귀 파싱하고, 좌변의 실제 타입에 따라
`VariableExpr`→`AssignExpr`, `GetExpr`→`SetExpr`(이미 만든 `GetExpr`에서 object 소유권을 꺼내 재사용),
`IndexExpr`→`IndexSetExpr`로 변환합니다. 이 변환 때문에 `a.b`, `arr[i]`는 먼저 "읽기" 형태로
파싱됐다가 `=`을 만나는 순간 "쓰기" 노드로 바뀝니다.

`Func` 선언과 클래스 메서드 선언은 `finishFunctionDecl(name)`(파라미터 목록 + `{ 본문 }` 파싱)을
공유합니다. 클래스 상속은 `:` 뒤에 **단일 식별자**만 허용합니다(`Class B : A.X { ... }`처럼 점(.)이
낀 표현식은 허용하지 않음 — 상속 대상이 다른 이름(예: import 네임스페이스의 멤버)이라면 먼저
`var Base = ns.X;`로 지역 변수에 담은 뒤 그 이름으로 상속해야 합니다).

### 3.3 AST와 Visitor

`Statement`/`Expression`(둘 다 공통 베이스 `StatementOrExpression`, `Node.h`)은 각각
`accept(StmtVisitor&)`/`accept(ExprVisitor&)`만 갖고 있고, "무엇을 할지"는 `Visitor.h`의
`ExprVisitor`/`StmtVisitor` 순수 가상 인터페이스를 구현하는 별도 클래스에 둡니다(Visitor 패턴,
자세한 근거는 DESIGN_PATTERNS.md 참고). 현재 노드 타입과 그 필드:

| Expression | 필드 |
|---|---|
| `LiteralExpr` | `Token` |
| `VariableExpr` | `Token`(식별자) |
| `AssignExpr` | `Token`(식별자), `Expression`(값) |
| `BinaryExpr` / `LogicalExpr` | 좌항, `Token`(연산자), 우항 |
| `UnaryExpr` | `Token`(연산자), 피연산자 |
| `GroupingExpr` | 내부 식 |
| `CallExpr` | callee, `Token`(닫는 괄호), 인자 목록 |
| `GetExpr` / `SetExpr` | object, `Token`(이름) [, value] |
| `ThisExpr` / `SuperExpr` | `Token`(키워드) [, `Token`(메서드명)] |
| `InstanceOfExpr` | object, `Token`(키워드), `Token`(클래스명) |
| `ArrayExpr` | size 식 |
| `IndexExpr` / `IndexSetExpr` | array 식, index 식 [, value 식] |

| Statement | 필드 |
|---|---|
| `ExpressionStmt` / `PrintStmt` / `ReturnStmt` | 식 [, `Token`(키워드)] |
| `IfStmt` | condition, then/else 분기(Statement) |
| `BlockStmt` | `vector<unique_ptr<Statement>>` |
| `VarDeclareStmt` | `Token`(이름), initializer 식 |
| `ForStmt` | init(`VarDeclareStmt`), condition, increment, body |
| `FunctionStmt` | `Token`(이름), `vector<Token>`(파라미터), body(`BlockStmt`) |
| `ClassStmt` | `Token`(이름), superclass(`VariableExpr`, nullable), methods(`vector<FunctionStmt>`) |
| `ImportStmt` | `Token`(경로 문자열), `Token`(별칭) |

`Visitor.h`/`Visitor.cpp`의 `ExprVisitor`/`StmtVisitor`를 구현하는 클래스는 현재 5개이며, 새 노드
타입을 추가하면 다섯 곳 전부가 컴파일 에러를 내 누락을 막습니다.

| 구현체 | 위치 | 역할 |
|---|---|---|
| `Checker` | CheckerUnit | 정적 검사 |
| `Interpreter` | ExecutorUnit | 실제 실행 |
| `Resolver` | ExecutorUnit | 정적 바인딩(거리 계산) |
| `ConstantFolder` | ExecutorUnit | 상수 폴딩 |
| `LineResolver` | ExecutorUnit | AST 노드의 소스 줄 번호 조회(예외 메시지/디버그 정지 위치용) |

## 4. CheckerUnit — 정적 검사

`Checker`(`Checker.h/.cpp`)는 위 다섯 Visitor 중 하나이며, `vector<unordered_map<string, bool>>
scope_stack`으로 렉시컬 스코프를 추적합니다. 각 스코프의 이름은 `declare`(선언됨, `false`) →
`define`(정의 완료, `true`) 2단계 상태를 가지며, 이 2단계 덕분에 "초기화식이 자기 자신을 읽는"
자기참조(`var a = a + 1;`)를 `visitVariableExpr`에서 즉시 감지합니다.

- **생성자에서 `beginScope()`를 한 번 호출하고 절대 `endScope()`하지 않습니다** — 이 스코프가
  전역 스코프 역할을 하며, `PromptShell`처럼 여러 번 나뉘어 들어오는 `check()` 호출에서도 전역
  변수/함수/클래스/import 별칭의 중복 선언을 검출할 수 있게 합니다(`Interpreter`의
  `global_environment`가 세션 내내 유지되는 것과 대응).
- RAII 가드 세 가지로 문맥을 추적합니다: `ScopeGuard`(스코프 진입/이탈), `FunctionGuard`(중첩 함수
  깊이 `function_depth`, 생성자 여부 `in_initializer`, 그리고 반복문 문맥 `loop_depth`를 함수
  경계에서 0으로 리셋 — 함수는 반복문 안에서 선언돼도 호출될 때만 실행되므로), `ClassGuard`
  (`this`/`Super` 사용 가능 여부와 상위 클래스 존재 여부를 스택으로 추적).
- `visitForStmt`는 `ScopeGuard` + `LoopGuard`(반복문 몸통 직계에서 `import` 사용을 금지하기 위한
  `loop_depth` 카운터)를 함께 엽니다.
- `visitImportStmt`는 반복문 문맥(`loop_depth > 0`)이면 즉시 예외를 던지고, 아니면 `alias`를
  일반 변수/함수/클래스와 동일한 `declare`/`define`으로 스코프에 등록합니다 — 그래서 같은 스코프
  중복 alias는 기존 중복 선언 오류 메시지를 그대로 재사용해서 얻습니다.
- 그 외 검사: 함수 외부 `return`, `init` 안의 `return`, 클래스 외부 `this`/`Super`, 부모 없는
  클래스의 `Super`, 자기 자신을 상속하는 클래스, 파라미터 이름 중복 등.
- **하지 않는 것**: 상속 대상이 실제로 클래스인지, `instanceof`의 대상이 클래스인지, import할
  파일이 실제로 존재하는지 같은 "값의 타입을 알아야 하는" 검사는 여기서 하지 않고 런타임(실행
  시점)에 `Interpreter`가 판단합니다. Checker는 순수하게 스코프/선언 규칙만 봅니다.

## 5. ExecutorUnit — 실행

### 5.1 Environment — 변수 스코프 체인

```cpp
class Environment : public enable_shared_from_this<Environment> {
    void define(const string& name, const Value& value);
    Value get(const Token& name) const;            // 이름으로 enclosing 체인을 훑어 탐색
    void assign(const Token& name, const Value& value);
    Value getAt(int distance, const string& name);        // 정적 바인딩: 거리만큼 곧장 이동
    void assignAt(int distance, const string& name, const Value& value);
    bool isGlobal() const;                                 // enclosing == nullptr
    const unordered_map<string, Value>& getOwnVariables() const;
    shared_ptr<Environment> enclosing;
    unordered_map<string, Value> values;
};
```

새 블록/함수 호출/`for`/클래스 메서드의 `this`·`super` 바인딩마다 새 `Environment`가
`make_shared<Environment>(현재 environment)`로 만들어지고, 그 블록/호출이 끝나면 이전
`environment`로 복원됩니다(try/catch로 예외가 나도 복원 보장). `get`/`assign`은 이름을 들고
`enclosing`을 거슬러 올라가는 선형 탐색이고, `getAt`/`assignAt`은 Resolver가 미리 계산해둔
`distance`만큼만 `enclosing`을 따라간 뒤 그 스코프의 해시맵에 바로 접근합니다.

### 5.2 Interpreter — 실행기

`Interpreter`는 `globals`(항상 최상위, `enclosing == nullptr`)와 현재 실행 중인
`environment`(둘 다 `shared_ptr<Environment>`) 두 포인터를 들고 있습니다. `interpret(statements)`가
호출될 때마다:

```cpp
resolver.resolve(statements);       // 1) 정적 바인딩 - 거리 미리 계산
constant_folder.fold(statements);   // 2) 상수 폴딩 - 리터럴 연산 미리 계산
for (stmt : statements) execute(stmt.get());   // 3) 순차 실행
```

`evaluate(expr)`는 먼저 `constant_folder`에 이미 계산된 값이 있는지 찾고, 있으면 그 값을 즉시
반환합니다. 변수 읽기(`evaluateVariableExpr`)/쓰기(`evaluateAssignExpr`)/`this`/`Super`/
`instanceof`의 클래스명 조회는 모두 `lookUpVariable(name, expr)`을 거칩니다:

```cpp
Value Interpreter::lookUpVariable(const Token& name, const Expression* expr) {
    if (const int* distance = resolver.getDistance(expr))
        return environment->getAt(*distance, name.getLexeme());
    try { return globals->get(name); }              // 미해결 = 전역이라고 간주하고 곧장 접근
    catch (const CodeFabException&) { return environment->get(name); }  // 그래도 없으면 체인 탐색으로 폴백
}
```

두 번째 갈래(`globals`로 곧장 접근)가 핵심 설계 결정입니다 — 상세 근거는 6장 참고.

블록/함수 호출/`for`/클래스 선언이 만드는 `Environment` 중첩 구조:

- 블록(`{ }`) 진입: `executeBlockStmt` → `make_shared<Environment>(현재 environment)` 하나.
- 함수 호출: `CodeFabFunction::call`이 파라미터를 담을 `Environment` **하나만** 만들고, 그 안에서
  본문 문장을 곧장 실행합니다(본문을 위한 별도 블록 스코프를 더 만들지 않음).
- `for`문: 반복 변수를 담을 전용 `Environment`를 하나 만들고, 그 안에서 조건/증감/몸통을
  실행합니다. 몸통이 `BlockStmt`라면 `executeBlockStmt`가 그 안에 또 하나의 스코프를 만듭니다.
- 클래스 메서드: 상속이 있으면 `"super"`를 바인딩한 `Environment`를 클래스의 모든 메서드가
  공유하는 closure로 사용하고, 인스턴스에 바인딩될 때(`CodeFabFunction::bind`)마다 `"this"`를
  바인딩한 `Environment`를 그 위에 하나 더 씌운 뒤, 호출 시 파라미터 `Environment`를 그 위에
  다시 씌웁니다. 즉 실제 체인은 (안→밖) `파라미터 → this → (있다면) super → 클래스 선언 시점의
  environment`입니다. Resolver는 이 정확한 중첩 순서를 그대로 스코프 스택으로 재현합니다.

`return`은 예외 기반 제어 흐름입니다: `ReturnSignal(Value)`을 던지고 `CodeFabFunction::call`이
잡아서 반환값으로 씁니다(에러가 아니라 정상 제어 흐름이므로 `CodeFabException`을 상속하지 않는
별도 타입). 생성자(`init`)는 `return`에 값을 실었더라도(Checker가 애초에 금지하지만) 항상
`this`(closure에서 꺼낸 인스턴스)를 반환하도록 강제됩니다.

### 5.3 Resolver — 정적 바인딩(Chapter 5)

`interpret()` 직전에 한 번 AST를 훑어, 변수/`this`/`Super`/`instanceof`의 클래스명 참조마다
"몇 단계 바깥 스코프에서 선언됐는지"(거리, `int`)를 `unordered_map<const Expression*, int> locals`에
미리 계산해둡니다. Checker와 **동일한 스코프 스택 추적 방식**을 쓰지만 두 가지가 다릅니다.

1. Checker는 전역 스코프를 인스턴스 생존 기간 내내 열어두지만(중복 선언 검출용), Resolver는
   `resolve()` 호출(=REPL 한 줄)마다 지역 스코프 스택을 빈 채로 시작합니다. 그 호출의 지역 스코프
   안에서 찾지 못한 참조는 전역 변수로 간주해 거리를 남기지 않습니다.
2. `if`/`for`의 분기·몸통 자체는 새 스코프를 열지 않습니다 — Interpreter도 그 문장이 `BlockStmt`일
   때만 새 `Environment`를 만들기 때문에, Resolver도 실제 실행 시점의 중첩 구조를 그대로 따라갑니다
   (Checker가 then/else에 각각 별도 스코프를 주는 것과는 다른 목적이므로 여기선 따르지 않음).

거리 계산 방식은 고전적인 Lox Resolver와 동일합니다: 지역 스코프 스택을 안쪽부터 훑어 이름을
찾은 스코프의 인덱스를 `i`라 하면 `거리 = (스택 크기 - 1) - i`.

**왜 미해결 참조가 `environment` 체인이 아니라 `globals`로 직행해야 하는가**: 만약 미해결
참조를 기존 `environment` 체인으로 그냥 훑게 두면, 아래처럼 클로저 선언 이후 같은 블록에 같은
이름이 다시 선언되는 경우 정적 스코프가 아니라 동적 스코프처럼 동작하는 버그가 생깁니다.

```
var a = "global";
{
  Func showA() { print a; }
  showA();          // "global"
  var a = "block";
  showA();          // 정적 바인딩이 없다면 "block"이 잘못 출력됨
}
```

`showA` 선언 시점에는 블록에 아직 `a`가 없으므로 Resolver는 이 참조를 "전역"으로 남겨두는데,
그 후 `var a = "block";`이 **같은 Environment 객체**(블록 하나에 Environment 하나)에 `a`를 정의해
버리면, 이름 기반 체인 탐색은 이 늦게 생긴 지역 변수를 잘못 붙잡습니다. `globals`로 곧장 접근하면
이 문제가 사라집니다. 다만 Resolver를 거치지 않고 `CodeFabFunction`을 직접 호출하는 화이트박스
테스트(예: `CodeFabFunctionTest`)를 위해, `globals`에도 없으면 기존 `environment` 체인 탐색으로
한 번 더 대체합니다(정상 파이프라인에서는 이 마지막 폴백에 도달할 일이 없습니다).

### 5.4 ConstantFolder — 상수 폴딩(Chapter 5)

`interpret()` 직전에(Resolver 다음) AST를 훑어, 리터럴로만 이뤄진 연산식(`+`/`-`/`*`/`/`, 비교,
`and`/`or`, 단항, 그룹)의 값을 `unordered_map<const Expression*, Value> folded`에 미리 계산해둡니다.
연산 로직을 새로 베끼지 않고 `Interpreter::evaluate`를 그대로 재사용해서 계산하므로(폴더가
`Interpreter&` 참조를 들고 있음) 실제 실행 결과와 항상 같습니다. 0으로 나누기처럼 실행 중 오류가
나는 연산은 `try/catch`로 삼키고 접지 않은 채 그대로 두어, 실행 시점에 원래와 동일한 예외와 줄
번호로 보고되도록 합니다. `Interpreter::evaluate`는 이 맵에서 값을 찾으면 재계산 없이 즉시
반환합니다.

### 5.5 Callable 계층 — Strategy 패턴

`Value`(`Tokenizer/Value.h`)는 `variant<monostate, bool, double, string, shared_ptr<Callable>,
shared_ptr<CodeFabArray>>`입니다. `Callable`은 `arity()`/`call(Interpreter&, args)`/`toString()`
세 메서드만 요구하는 인터페이스이며, `Interpreter::evaluateCallExpr`는 이 세 메서드만으로 실제
무엇이 호출되는지 몰라도 호출을 위임합니다(Strategy 패턴).

| 구현체 | 호출 가능? | 요약 |
|---|---|---|
| `CodeFabFunction` | 예 | `Func`/메서드 공용. `declaration`(`FunctionStmt*`, raw pointer)과 `closure`(`Environment`)를 들고 있다가 호출 시 파라미터 `Environment`를 새로 만들어 본문을 실행. `bind(instance)`는 `"this"`를 바인딩한 새 `Environment`를 closure로 갖는 **새 `CodeFabFunction` 복사본**을 인스턴스별로 만들어 반환(클래스 정의 시점에 미리 바인딩해두지 않음). |
| `CodeFabClass` | 예 | 자신을 호출하면(`Robot()`) 인스턴스를 만드는 `Callable`. `findMethod`가 상속 체인을 거슬러 올라가며 메서드를 찾아 오버라이딩을 지원하고, `isSubclassOf`가 `instanceof`를 판별. |
| `CodeFabInstance` | 아니오 | 필드를 `unordered_map<string, Value>`에 동적으로 저장. `get()`은 필드 우선, 없으면 메서드를 찾아 `bind()`한 값을 반환. `Callable`을 구현하지만(값이 객체 슬롯 하나만 가지므로) `evaluateCallExpr`가 호출 직전에 걸러냄. |
| `CodeFabNamespace` | 아니오 | `import "a.txt" alias a;`의 `a`가 평가되는 값. `CodeFabInstance`와 같은 모양의 "이름→값 가방"이며, import된 파일의 최상위 바인딩이 여기 채워짐. 마찬가지로 호출 직전에 걸러냄. |

`CodeFabArray`는 `Callable`이 아니라 `Value`가 별도 슬롯으로 갖는 값이며, 항상 `shared_ptr`로
담겨 참조로 공유됩니다(배열을 변수/인자에 대입해도 같은 배열을 가리킴). 크기는 생성 시 고정되고
원소는 `monostate`(nil)로 초기화됩니다.

### 5.6 import — 서브 파이프라인 재사용

`Interpreter::executeImportStmt`가 실제 동작을 수행합니다.

1. `file_exists(path)`로 존재를 확인(없으면 예외), `import_stack`에 이미 있는 경로면 순환
   import로 판단해 예외.
2. `read_source(path)`로 파일 전체를 읽고, **자신만의** `AssemblerUnit`으로 assemble, **새로
   만든** `Checker` 인스턴스로 그 파일만 검사(Checker는 상태를 밖으로 들고 다니지 않으므로 이
   Interpreter의 검사 상태와 섞이지 않음).
3. `globals`/`environment`를 임시로 새 `Environment`(비어 있음)로 바꿔치기하고, **같은
   Interpreter 인스턴스로 `interpret()`을 재귀 호출**합니다 — Resolver/ConstantFolder를 포함한
   전체 실행 파이프라인이 그대로 재사용되고, import된 파일은 완전히 독립된 전역 스코프에서
   실행됩니다.
4. 실행이 끝나면 그 임시 `globals`의 `getOwnVariables()`를 `CodeFabNamespace`에 옮겨 담고,
   원래의 `globals`/`environment`로 복원한 뒤, 그 네임스페이스를 alias 이름으로 현재
   `environment`에 정의합니다.
5. import된 파일의 AST(`vector<unique_ptr<Statement>>`)는 그 안에서 선언된 `Func`/`Class`의
   클로저가 raw pointer로 계속 가리키므로, `imported_statements`에 담아 이 Interpreter가 살아있는
   동안 함께 보관합니다(`CodeFabFacade::retained_statements`와 같은 이유).

파일 읽기(`file_exists`/`read_source`)는 `function<...>`으로 주입 가능하며(`_DEBUG`에서만 생성자
인자로 노출, release는 항상 `std::filesystem::exists`/`ifstream` 슬러프 사용), 테스트는 가짜
파일 시스템을 주입해 실제 디스크 없이 import 전체 파이프라인을 검증합니다
(`ExecutorUnit/ImportTest.cpp`).

## 6. CodeFabFacade — 파이프라인 조립

```cpp
class CodeFabFacade {
public:
    void execute(const string& code_line);            // assemble → check → interpret
    void setBeforeStatementHook(function<void(int)>);  // 디버그 모드용 훅 전달
    vector<VariableSnapshot> inspectVariables() const; // watch/inspect용 변수 열거
private:
    AssemblerUnit assembler_unit; Checker checker; Interpreter executor; // (release)
    vector<vector<unique_ptr<Statement>>> retained_statements;
};
```

`_DEBUG` 빌드에서는 `IAssemblerUnit`/`IChecker`/`IExecutor`(`InterfaceForCodeFabTest.h`) 포인터로
바꿔치기해 gmock 목으로 주입할 수 있는 두 번째 생성자를 추가로 제공합니다(`CodeFabFacadeTest.cpp`가
이 경로로 파이프라인 호출 순서를 검증). `execute()`가 호출될 때마다 조립된 문장을
`retained_statements`에 보관해 Facade가 살아있는 동안(REPL 세션 내내) `Func`/`Class` 클로저가
가리키는 원본 AST가 계속 유효하도록 합니다. `CodeFabFacade`는 **파일 시스템을 주입할 방법이
없으므로**, import를 실제로 exercise하는 테스트는 Facade 대신 `Interpreter`를 직접 다룹니다
(8장의 통합 테스트 참고).

## 7. FactoryShell — 세 가지 실행 방식

```
IShellMode(인터페이스: enter())
  ├─ PromptShell               표준 입력을 한 줄씩 읽어 CodeFabFacade::execute() 호출, exit/quit로 종료
  └─ FileBackedShell(Template Method, CodeFabFacade 보유)
       enter(): 파일 존재 확인 → 줄 단위로 읽기 → afterLoad() 훅 → beforeExecute() 훅
                → reportShellExceptions([]{ facade.execute(전체 소스) })
       ├─ FileModeShell   훅을 오버라이드하지 않는 순수 실행
       └─ DebugModeShell  beforeExecute()에서 setBeforeStatementHook 등록,
                          afterLoad()에서 로딩 로그 출력
```

- `ArgumentParser::parse(argv)`가 `{run, path}` → File, `{debug, path}` → Debug, 인자 없음 → Prompt,
  그 외 → Invalid로 판정하며, `main.cpp`가 그 결과에 맞는 Shell을 하나 생성해 `enter()`를 호출합니다.
  `_DEBUG` 빌드의 `main`은 이 CLI 분기를 전혀 타지 않고 `RUN_ALL_TESTS()`만 실행합니다(디버그
  빌드 = 테스트 바이너리).
- `DebugModeShell`은 `Interpreter::setBeforeStatementHook`으로 등록한 콜백(`onBeforeStatement`)이
  매 문장 실행 직전 호출되는 것을 이용해 정지 지점을 만듭니다. `mode`(`Step`/`Next`/`Continue`)와
  `breakpoints`(줄 번호 집합), `watched_variables`(이름 집합)를 들고 있고, 정지할 때마다 표준
  입력에서 명령어를 한 줄 읽어 `commands` 테이블(문자열 → `DebugCommand` 구현체)에서 찾아
  위임합니다. 현재 구현에서 `step`과 `next`는 동일하게 동작합니다(둘 다 `mode != Continue`를
  참으로 만들 뿐, "호출 안으로 들어가지 않고 건너뛰는" 별도 로직은 없습니다).
- `DebugCommand`는 Command 패턴입니다: `step`/`next`/`continue`는 모드를 바꾸고 실행을 재개
  (`true` 반환)하며, `break <줄>`/`remove <줄>`/`breakpoints`/`watch <이름>`/`unwatch <이름>`/
  `watches`/`inspect`는 상태만 바꾸거나 출력하고 같은 정지 지점에서 다음 명령을 계속 받습니다
  (`false` 반환).
- 파일 읽기(`defaultFileExists`/`defaultReadLines`, `std::filesystem`/`ifstream` 기반)는
  `FileBackedShell`에, 오류 보고(`reportShellExceptions` — `CodeFabException`/`std::exception`/
  그 외 예외를 순서대로 잡아 표준 에러로 출력)는 `shell_exception_reporter.h`에 분리되어
  `FileModeShell`/`DebugModeShell`이 공유합니다.

## 8. 값/오류 모델

- **`Value`**: `variant<monostate, bool, double, string, shared_ptr<Callable>,
  shared_ptr<CodeFabArray>>`. `isNumber`/`isString`/`isCallable`/`isArray`/`isTruthy`/`stringify`가
  타입 판별·출력을 담당하는 자유 함수로 `Value.h`에 있습니다. `isTruthy`는 `monostate`만 거짓,
  `bool`은 자기 값, 그 외는 전부 참입니다(0.0/빈 문자열도 참 — JavaScript식이 아니라 Ruby/Lox식
  진리값 규칙).
- **`CodeFabException`**(`runtime_error` 상속): `(int line, message)` 또는 `(Token, message)`
  두 생성자가 있으며, 후자는 `[line N] Type : ..., Lexeme : ..., literal : ..., message : ...`
  형태로 토큰 정보를 함께 남깁니다. 파싱 오류, Checker의 정적 검사 오류, 런타임 오류(0으로
  나누기, 정의되지 않은 변수, 타입 불일치, 배열 범위 초과 등)가 전부 이 한 타입으로 표현됩니다.
- **`ReturnSignal`**은 오류가 아닌 정상 제어 흐름이라 `CodeFabException`을 상속하지 않는 별도
  타입입니다 — "예외 계층 = 오류"라는 불변식을 지키기 위한 의도적인 구분입니다.
- 오류가 보고되는 장소는 계층마다 다릅니다: `FileBackedShell`은 `reportShellExceptions`로 감싸서
  표준 에러로 출력하고 셸을 계속 살려두며, `PromptShell`은 한 줄 실행 중 예외를 잡아 다음 줄
  입력을 계속 받고, `main.cpp`의 최상위 `try/catch`는 `PromptShell`처럼 자체적으로 예외를 삼키지
  않는 셸을 위한 마지막 안전망입니다.

## 9. 테스트 전략

- **유닛 테스트**: 각 클래스마다 같은 디렉터리에 `*Test.cpp`가 있으며(gtest/gmock), Fixture
  패턴(`TEST_F`)으로 반복되는 픽스처 생성을 한곳에 모읍니다. 화이트박스 테스트(예:
  `CodeFabFunctionTest`, `CodeFabClassTest`)는 Resolver 등 상위 파이프라인을 거치지 않고 AST를
  손으로 만들어 대상 클래스만 직접 호출합니다.
- **Facade 수준 테스트**(`CodeFabFacadeTest.cpp`): 실제 `AssemblerUnit`/`Checker`/`Interpreter`를
  그대로 쓰는(목이 아닌) end-to-end 테스트와, gmock으로 세 컴포넌트를 목 처리해 파이프라인 호출
  순서만 검증하는 테스트가 공존합니다.
- **통합 테스트**(`IntegrationTest.cpp`): 클래스 상속, 배열 필드, 재귀, 클로저(정적 바인딩),
  상수 폴딩, import를 한 프로그램 안에 함께 조합해 실제 파이프라인으로 실행해봅니다. `CodeFabFacade`가
  파일 시스템을 주입할 수 없다는 제약 때문에, import가 섞인 시나리오는 Facade가 내부적으로 하는
  일(assemble → check → interpret)을 가짜 파일 시스템을 주입한 `Interpreter`로 직접 재현하는
  `TestPipeline` 헬퍼로 검증합니다.
- **수동 회귀 스크립트**(`TestCase/*.txt`): gtest와 별개로, 실제로 빌드된 실행 파일에 `run_all.bat`으로
  돌려보는 CodeFab 스크립트 모음입니다. 각 파일은 `// expect:` 주석으로 기대 출력을 명시하고,
  `generate_report.js`가 실제 출력과 diff해 HTML 리포트를 만듭니다. 기능 하나당 파일 하나로,
  gtest 스위트에는 포함되지 않는 별도의 수동 확인 절차입니다.

## 10. 확장할 때 지켜야 할 것

- **새 AST 노드 타입을 추가할 때**: `Statement.h`/`Expression.h`에 클래스 추가 → `Visitor.h`의
  `ExprVisitor`/`StmtVisitor`에 순수 가상 메서드 추가 → `Visitor.cpp`에 `accept()` 구현 추가 →
  다섯 Visitor 구현체(Checker/Interpreter/Resolver/ConstantFolder/LineResolver) 전부에
  `visit*`를 추가. 컴파일러가 누락을 강제로 잡아주므로 순서를 지키지 않아도 마지막에 반드시
  드러납니다.
- **새 키워드를 추가할 때**: `TokenType` enum → `tokenTypeToString` → `Lexer`의 키워드 테이블 →
  `Parser::parseStatement()`(또는 해당 식 파싱 체인)의 완전 열거 switch에 case 추가(누락 시
  "문장을 시작할 수 없는 토큰" 목록에라도 명시적으로 나열해야 함).
- **Environment 중첩을 바꾸는 실행 변경(새로운 스코프 생성 지점 추가 등)을 할 때**: `Resolver`의
  대응하는 `visit*`도 반드시 같은 지점에서 `beginScope`/`endScope`를 열고 닫도록 맞춰야 합니다 —
  그렇지 않으면 정적 바인딩이 계산한 거리와 런타임 `Environment` 체인의 실제 중첩이 어긋나
  엉뚱한 스코프의 값을 읽거나 쓰게 됩니다.
- **새로운 "이름→값 가방" 타입(예: 모듈, 네임스페이스, 레코드)을 추가할 때**: `CodeFabInstance`/
  `CodeFabNamespace`처럼 `Callable`을 구현하되 `call()`은 항상 예외를 던지게 하고,
  `Interpreter::evaluateCallExpr`의 "호출 직전 걸러내기" 목록에 새 타입을 추가해야 합니다 —
  그렇지 않으면 `x()`처럼 잘못 호출했을 때 `CodeFabException` 대신 처리되지 않은
  `std::logic_error`로 죽습니다.
