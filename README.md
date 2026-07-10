# CodeFabIntepreter

CodeFab이라는 팀 전용 스크립트 언어를 파싱하고 실행하는 C++ 인터프리터 프로젝트입니다.
콘솔에서 한 줄씩 코드를 입력받아 즉시 실행하는 REPL(`PromptShell`), 파일 하나를 통째로
읽어 실행하는 파일 모드(`FileModeShell`), Stmt 단위로 멈춰가며 점검하는 디버그 모드
(`DebugModeShell`)를 함께 제공합니다.

## 진행 상황

기본기능 과제(Custom Language + Code Fab Interpreter + Prompt Shell)와, 추가기능 과제 중
함수(Chapter 2)·클래스(Chapter 3)·정적 배열·파일 모드·디버그 모드(Chapter 7) 요구사항이 모두
구현되어 단위 테스트로 검증되었습니다. 실행 전 최적화, import는 아직 착수 전입니다.

항목별 체크리스트는 [요구사항.md](요구사항.md)를 참고해 주세요.

## 아키텍처 및 프로젝트 구조

전체 실행 흐름(`AssemblerUnit` → `CheckerUnit` → `ExecutorUnit` 파이프라인), 각 유닛의 역할과 사용된
디자인 패턴, 프로젝트 폴더 구조는 [구조및설계.md](구조및설계.md)에 정리되어 있습니다.

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

```
CodeFab Interpreter - exit / quit로 종료
> var a = 10;
> print a + 5;
15
> exit
CodeFab Interpreter Exit
```

인자 없이 실행하면 `PromptShell`이 콘솔에서 코드를 한 줄씩 입력받아 즉시 실행합니다.

### 파일 모드

```
CodeFabIntepreter run script.txt
```

파일 하나를 통째로 실행합니다.

### 디버그 모드

```
CodeFabIntepreter debug script.txt
```

Stmt 단위로 멈춰가며 점검합니다(`step`/`next`/`break`/`continue`/`watch`/`inspect` 등).

함수/클래스/정적 배열 문법, 각 모드의 세부 동작과 오류 처리, 전체 명령어 목록, 문법 지원 범위는
[문법.md](문법.md)에서 확인하실 수 있습니다.

## 참고 문서

- [요구사항.md](요구사항.md) — 진행 상황(기본기능/추가기능) 체크리스트
- [구조및설계.md](구조및설계.md) — 아키텍처, 프로젝트 구조
- [문법.md](문법.md) — 문법 지원 범위 및 사용법 상세

## 기여 가이드

- 코드 스타일 및 컨벤션: [code_convention.md](code_convention.md)
- 적용된 디자인 패턴과 그 이유: [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md)
- PR 작성 시 [PR 템플릿](.github/pull_request_template.md)에 따라 요구사항, 주요 변경점, 테스트 결과, 체크리스트를 작성해 주세요.
