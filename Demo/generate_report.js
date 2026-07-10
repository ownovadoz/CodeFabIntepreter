// run_all.bat이 모든 테스트케이스를 실행한 뒤 호출하는 리포트 생성기.
// 각 소스 파일의 "// expect" 주석을 다시 읽고, 실제로 exe를 실행해 캡처한 출력과
// 순서대로 대조해서, 같은 폴더에 날짜가 들어간 HTML 리포트를 새로 만든다.

const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const SCRIPT_DIR = __dirname;
const TESTCASE_DIR = path.join(SCRIPT_DIR, '..', 'TestCase');
const EXE = path.join(SCRIPT_DIR, 'CodeFabIntepreter.exe');

const CASES = [
	{ file: '01_variables_and_print.txt', category: '언어 기능', desc: '변수 선언, 숫자/문자열/불리언 print' },
	{ file: '02_arithmetic_operators.txt', category: '언어 기능', desc: '+ - * /, 문자열 덧셈, 단항 -' },
	{ file: '03_comparison_and_logical.txt', category: '언어 기능', desc: '비교 연산자, and/or, 단항 !' },
	{ file: '04_if_else.txt', category: '언어 기능', desc: 'if / else 분기' },
	{ file: '05_for_loop.txt', category: '언어 기능', desc: 'for 반복, 누적합, 반복문 안 if' },
	{ file: '06_blocks_and_scope.txt', category: '언어 기능', desc: '블록 스코프, 변수 가리기(shadowing)' },
	{ file: '07_error_duplicate_declaration.txt', category: 'Checker 오류', desc: '같은 스코프에서 변수 중복 선언' },
	{ file: '08_error_self_reference.txt', category: 'Checker 오류', desc: '선언 시 자기 자신을 초기화식에서 참조' },
	{ file: '09_error_type_mismatch.txt', category: '런타임 오류', desc: '숫자 - 문자열 잘못된 연산' },
	{ file: '10_error_undefined_variable.txt', category: '런타임 오류', desc: '선언되지 않은 변수 참조' },
	{ file: '11_error_divide_by_zero.txt', category: '런타임 오류', desc: '0으로 나누기' },
	{ file: '13_function_basic.txt', category: '함수', desc: '함수 선언·호출·return, 반환값 없는 함수' },
	{ file: '14_function_recursion.txt', category: '함수', desc: '재귀 호출 — 팩토리얼, 피보나치' },
	{ file: '15_error_function_return_outside.txt', category: 'Checker 오류', desc: '함수 외부에서 return 사용' },
	{ file: '16_error_function_duplicate_param.txt', category: 'Checker 오류', desc: '함수 파라미터 이름 중복' },
	{ file: '17_error_function_arg_count_mismatch.txt', category: '런타임 오류', desc: '함수 호출 인자 개수 불일치' },
	{ file: '18_error_function_call_non_callable.txt', category: '런타임 오류', desc: '함수가 아닌 값을 호출' },
	{ file: '19_class_basic.txt', category: '클래스', desc: '클래스 선언·인스턴스·필드·메서드·this·생성자(init)' },
	{ file: '20_class_inheritance.txt', category: '클래스', desc: '상속·메서드 오버라이딩·Super 호출·instanceof' },
	{ file: '21_error_class_this_outside.txt', category: 'Checker 오류', desc: '클래스 외부에서 this 사용' },
	{ file: '22_error_class_super_outside.txt', category: 'Checker 오류', desc: '클래스 외부에서 Super 사용' },
	{ file: '23_error_class_self_inherit.txt', category: 'Checker 오류', desc: '클래스가 자기 자신을 상속' },
	{ file: '24_error_class_init_return.txt', category: 'Checker 오류', desc: '생성자(init) 안에서 return 사용' },
	{ file: '25_error_class_undefined_field.txt', category: '런타임 오류', desc: '존재하지 않는 필드 접근' },
	{ file: '26_array_basic.txt', category: '배열', desc: '배열 생성·nil 초기화·인덱스 읽기/쓰기(리터럴/계산된 인덱스)' },
	{ file: '27_array_reference_sharing.txt', category: '배열', desc: '배열은 참조로 공유되는 값' },
	{ file: '28_error_array_out_of_range.txt', category: '런타임 오류', desc: '배열 인덱스 범위 초과' },
	{ file: '29_error_array_index_type.txt', category: '런타임 오류', desc: '배열 인덱스가 숫자가 아님' },
	{ file: '30_error_array_non_array_index.txt', category: '런타임 오류', desc: '배열이 아닌 값에 인덱스로 접근' },
	{ file: '31_error_array_size_type.txt', category: '런타임 오류', desc: '배열 크기가 숫자가 아님' },
];

const DEBUG_SOURCE = '12_debug_mode_demo.txt';
const DEBUG_COMMANDS = '12_debug_mode_demo.commands.txt';

function escapeHtml(text) {
	return String(text)
		.replace(/&/g, '&amp;')
		.replace(/</g, '&lt;')
		.replace(/>/g, '&gt;');
}

// "expect" 주석 한 줄을 {kind, fragments} 로 변환한다.
// kind: 'error' -> 통째로 한 조각. 'normal' -> 괄호 설명을 떼고 콤마로 나눈 여러 조각.
// core가 비어 있으면(예: "(실행되지 않음...)") 검증할 조각이 없는 항목으로 취급한다.
function parseExpect(kind, raw) {
	if (kind === 'error') {
		return [raw.trim()];
	}
	const withoutParen = raw.replace(/\s*\([^)]*\)\s*$/, '').trim();
	if (!withoutParen) return [];
	return withoutParen.split(',').map((s) => s.trim()).filter(Boolean);
}

// annotation 목록을 실제 출력(actualText)과 순서대로 대조한다.
function matchAgainstOutput(annotations, actualText) {
	let cursor = 0;
	let matchedCount = 0;
	let totalCount = 0;
	const results = annotations.map((ann) => {
		if (ann.fragments.length === 0) {
			return { ...ann, fragmentResults: [], allMatched: true, info: true };
		}
		const fragmentResults = ann.fragments.map((frag) => {
			totalCount++;
			const idx = actualText.indexOf(frag, cursor);
			if (idx === -1) return { text: frag, matched: false };
			cursor = idx + frag.length;
			matchedCount++;
			return { text: frag, matched: true };
		});
		return { ...ann, fragmentResults, allMatched: fragmentResults.every((f) => f.matched), info: false };
	});
	return { results, matchedCount, totalCount };
}

function extractAnnotations(sourceLines) {
	const annotations = [];
	sourceLines.forEach((line, idx) => {
		const m = line.match(/\/\/\s*expect(\s+error)?:\s*(.*)$/);
		if (!m) return;
		const kind = m[1] ? 'error' : 'normal';
		const raw = m[2];
		annotations.push({
			lineNumber: idx + 1,
			kind,
			raw,
			context: line.split('//')[0].trim(),
			fragments: parseExpect(kind, raw),
		});
	});
	return annotations;
}

function runFile(file) {
	const result = spawnSync(EXE, ['run', path.join(TESTCASE_DIR, file)], { encoding: 'utf8' });
	return (result.stdout || '') + (result.stderr || '');
}

function buildRegularCase(caseDef) {
	console.log(`===== ${caseDef.file} =====`);
	const sourcePath = path.join(TESTCASE_DIR, caseDef.file);
	const sourceLines = fs.readFileSync(sourcePath, 'utf8').split(/\r?\n/);
	const annotations = extractAnnotations(sourceLines);
	const actual = runFile(caseDef.file);
	console.log(actual);
	const { results, matchedCount, totalCount } = matchAgainstOutput(annotations, actual);
	return { ...caseDef, results, matchedCount, totalCount, rawActual: actual };
}

function buildDebugCase() {
	console.log(`===== ${DEBUG_SOURCE} (debug mode) =====`);
	const commandLines = fs.readFileSync(path.join(TESTCASE_DIR, DEBUG_COMMANDS), 'utf8').split(/\r?\n/).filter(Boolean);
	const commands = [];
	const annotations = [];
	commandLines.forEach((line) => {
		const m = line.match(/^(\S+(?:\s+\S+)?)\s*\/\/\s*expect:\s*(.*)$/);
		const cmd = m ? m[1].trim() : line.split('//')[0].trim();
		commands.push(cmd);
		if (m) {
			const fragments = m[2].split('/').map((s) => s.replace(/\s*\([^)]*\)\s*$/, '').trim()).filter(Boolean);
			annotations.push({ lineNumber: null, kind: 'normal', raw: m[2], context: cmd, fragments });
		} else {
			annotations.push({ lineNumber: null, kind: 'normal', raw: '', context: cmd, fragments: [] });
		}
	});

	const stdinData = commands.join('\n') + '\n';
	const result = spawnSync(EXE, ['debug', path.join(TESTCASE_DIR, DEBUG_SOURCE)], { input: stdinData, encoding: 'utf8' });
	const actual = (result.stdout || '') + (result.stderr || '');
	console.log(actual);
	const { results, matchedCount, totalCount } = matchAgainstOutput(annotations, actual);

	// 첫 명령어를 입력하기 전, 파일 로딩과 함께 자동으로 멈춘 초기 상태.
	// "몇 번째 줄에서 시작했는지"를 시간순 타임라인의 맨 앞에 보여주기 위해 따로 떼어둔다.
	const firstPromptIndex = actual.indexOf('> ');
	const initialSegment = (firstPromptIndex === -1 ? actual : actual.slice(0, firstPromptIndex)).trim();

	return {
		file: DEBUG_SOURCE,
		category: '디버그 모드',
		desc: `${DEBUG_COMMANDS}를 표준 입력으로 흘려보낸 세션 기록 (파일 로딩 → 명령어 입력을 시간순으로 표시)`,
		commands,
		initialSegment,
		results,
		matchedCount,
		totalCount,
		rawActual: actual,
	};
}

function renderRegularCaseHtml(c) {
	const featureCategories = ['언어 기능', '함수', '클래스', '배열'];
	const chipClass = featureCategories.includes(c.category) ? 'cat' : 'cat errcase-tag';
	const rows = c.results.map((r) => {
		const contextLine = `<div class="context">${r.context ? escapeHtml(r.context) : '<span class="mono">(source line)</span>'}<span class="line-tag">line ${r.lineNumber}</span></div>`;
		if (r.info) {
			return `<div class="assertion">${contextLine}<div class="compare"><span class="tick info">–</span><div class="note">검증 대상 출력 없음 (${escapeHtml(r.raw)})</div></div></div>`;
		}
		const tick = r.allMatched ? '<span class="tick">✓</span>' : '<span class="tick fail">✗</span>';
		const expectedHtml = r.fragmentResults.map((f) => escapeHtml(f.text)).join('<br>');
		const actualHtml = r.fragmentResults.map((f) => `${f.matched ? '' : '<span class="fail-mark">[미일치] </span>'}${escapeHtml(f.text)}`).join('<br>');
		return `<div class="assertion">${contextLine}<div class="compare">${tick}<div><span class="field-label">actual</span><div class="val">${actualHtml}</div></div><span class="arrow">=</span><div><span class="field-label">expect</span><div class="val">${expectedHtml}</div></div></div></div>`;
	}).join('\n');

	return `
    <div class="case">
      <div class="case-head">
        <div class="case-title">
          <span class="case-file">${escapeHtml(c.file)}</span>
          <span class="case-desc">${escapeHtml(c.desc)}</span>
        </div>
        <div class="chips"><span class="chip ${chipClass}">${escapeHtml(c.category)}</span><span class="chip ${c.matchedCount === c.totalCount ? 'pass' : 'fail'}">${c.matchedCount}/${c.totalCount} 일치</span></div>
      </div>
      <div class="rows">
${rows}
      </div>
    </div>`;
}

// "[DEBUG] N번째 줄에서 정지 (breakpoint)? → <소스코드>" 형태의 줄에서
// 실제 소스코드 부분만 따로 뽑아낸다. 그 소스코드는 12_debug_mode_demo.txt를
// 통해 인터프리터에 "입력"되는 줄이므로, command 입력과 같은 방식으로 보여준다.
function extractSourceLine(text) {
	const m = text.match(/^\[DEBUG\]\s*(\d+)번째\s*줄에서\s*정지(\s*\(breakpoint\))?\s*→\s*(.*)$/);
	if (!m) return null;
	const lineNumber = m[1];
	const status = m[2] ? '정지 (breakpoint)' : '정지';
	return { prefix: `[DEBUG] ${status}`, lineNumber, code: m[3] };
}

function stateRowHtml(tick, out) {
	return `<div class="turn turn-state">${tick}<div><span class="turn-label">결과</span><div class="out">${out}</div></div></div>`;
}

// 파일의 몇 번째 줄에서 이 소스코드가 입력됐는지 line-tag로 함께 보여줘서,
// command가 파일 내 어느 위치에서 실행됐는지 한눈에 알 수 있게 한다.
function sourceInputRowHtml(code, lineNumber) {
	const tag = lineNumber ? `<span class="line-tag">line ${lineNumber}</span>` : '';
	return `<div class="turn turn-input"><span class="turn-arrow">›</span><div><span class="turn-label">소스코드 입력</span><div class="cmd-input">${escapeHtml(code)}${tag}</div></div></div>`;
}

function commandInputRowHtml(cmd) {
	return `<div class="turn turn-input"><span class="turn-arrow">›</span><div><span class="turn-label">command 입력</span><div class="cmd-input">${escapeHtml(cmd)}</div></div></div>`;
}

// tick과 여러 줄(fragment)로 이뤄진 하나의 "결과" 묶음을, 필요하면
// [결과 행] + [그 안에 있던 소스코드를 뽑아낸 입력 행]으로 나눠서 rows에 쌓는다.
function pushResultRows(rows, tick, lines) {
	const restLines = [];
	let extracted = null;
	lines.forEach((line) => {
		const found = extractSourceLine(line.plain);
		if (found && extracted === null) {
			extracted = found;
			restLines.push({ plain: found.prefix, html: escapeHtml(found.prefix) });
		} else {
			restLines.push(line);
		}
	});
	const out = restLines.map((l) => l.html).join('<br>');
	rows.push(stateRowHtml(tick, out));
	if (extracted !== null) rows.push(sourceInputRowHtml(extracted.code, extracted.lineNumber));
}

function renderDebugCaseHtml(c) {
	// 결과 -> 소스코드 입력 -> command 입력 -> 결과 -> 소스코드 입력 -> ... 순서가
	// 그대로 보이도록, "입력" 행과 "결과" 행을 번갈아 하나씩 쌓는다.
	const rows = [];
	pushResultRows(rows, '<span class="tick info">–</span>', [{ plain: c.initialSegment, html: escapeHtml(c.initialSegment) }]);

	c.results.forEach((r, i) => {
		rows.push(commandInputRowHtml(c.commands[i] || ''));

		if (r.info) {
			rows.push(stateRowHtml('<span class="tick info">–</span>', '<span style="opacity:.6">(검증 대상 출력 없음)</span>'));
			return;
		}
		const tick = r.allMatched ? '<span class="tick">✓</span>' : '<span class="tick fail">✗</span>';
		const lines = r.fragmentResults.map((f) => ({
			plain: f.text,
			html: `${f.matched ? '' : '<span class="fail-mark">[미일치] </span>'}${escapeHtml(f.text)}`,
		}));
		pushResultRows(rows, tick, lines);
	});
	const commandTurns = rows.join('\n');

	return `
    <div class="case">
      <div class="case-head">
        <div class="case-title">
          <span class="case-file">${escapeHtml(c.file)}</span>
          <span class="case-desc">${escapeHtml(c.desc)}</span>
        </div>
        <div class="chips"><span class="chip cat">${escapeHtml(c.category)}</span><span class="chip ${c.matchedCount === c.totalCount ? 'pass' : 'fail'}">${c.matchedCount}/${c.totalCount} 일치</span></div>
      </div>
      <div class="transcript">
${commandTurns}
      </div>
    </div>`;
}

function groupByCategory(cases) {
	const order = ['언어 기능', '함수', '클래스', '배열', 'Checker 오류', '런타임 오류', '디버그 모드'];
	const groups = new Map();
	order.forEach((k) => groups.set(k, []));
	cases.forEach((c) => {
		if (!groups.has(c.category)) groups.set(c.category, []);
		groups.get(c.category).push(c);
	});
	return order.filter((k) => groups.get(k).length > 0).map((k) => [k, groups.get(k)]);
}

function pad(n) { return String(n).padStart(2, '0'); }

function main() {
	if (!fs.existsSync(EXE)) {
		console.error('[ERROR] exe not found: ' + EXE);
		process.exit(1);
	}

	const regularResults = CASES.map(buildRegularCase);
	const debugResult = buildDebugCase();
	const allCases = [...regularResults, debugResult];

	const totalMatched = allCases.reduce((sum, c) => sum + c.matchedCount, 0);
	const totalCount = allCases.reduce((sum, c) => sum + c.totalCount, 0);
	const filesPassed = allCases.filter((c) => c.matchedCount === c.totalCount).length;

	const grouped = groupByCategory(allCases);
	const sectionsHtml = grouped.map(([category, cases]) => {
		const sectionTitle = {
			'언어 기능': '언어 기본 기능',
			'함수': '함수 (Func) 기능',
			'클래스': '클래스 (Class) 기능',
			'배열': '정적 배열 (Array) 기능',
			'Checker 오류': 'Checker 오류 (실행 전 정적 검사)',
			'런타임 오류': 'Executor 런타임 오류 (실행 중 발생)',
			'디버그 모드': '디버그 모드 — Stmt 단위 stepping / breakpoint / watch / inspect',
		}[category] || category;
		const casesHtml = cases.map((c) => c.commands ? renderDebugCaseHtml(c) : renderRegularCaseHtml(c)).join('\n');
		return `  <div class="section-label">${escapeHtml(sectionTitle)}</div>\n  <div class="cases">\n${casesHtml}\n  </div>`;
	}).join('\n\n');

	const now = new Date();
	const stamp = `${now.getFullYear()}${pad(now.getMonth() + 1)}${pad(now.getDate())}_${pad(now.getHours())}${pad(now.getMinutes())}${pad(now.getSeconds())}`;
	const displayStamp = `${now.getFullYear()}-${pad(now.getMonth() + 1)}-${pad(now.getDate())} ${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;

	const html = `<title>CodeFab Interpreter — TestCase 검증 리포트</title>
<style>
  :root {
    --bg: #f6f5f1; --surface: #ffffff; --border: #dcd8cf; --text: #1c1b19; --text-muted: #6b675e;
    --accent: #2f6f5e; --accent-soft: #e4ece9; --pass: #2f7d4f; --pass-bg: #e3f0e6;
    --errorcase: #a9720c; --errorcase-bg: #f7ecd6; --fail: #b3261e; --fail-bg: #fbe4e2;
    --code-bg: #f1efe8; --shadow: 0 1px 2px rgba(28,27,25,.04), 0 4px 12px rgba(28,27,25,.03);
  }
  :root[data-theme="dark"] {
    --bg: #14181a; --surface: #1b2023; --border: #2b3235; --text: #e7e6e1; --text-muted: #9aa39f;
    --accent: #6fcf9e; --accent-soft: rgba(111,207,158,.1); --pass: #6fcf9e; --pass-bg: rgba(111,207,158,.12);
    --errorcase: #e0b15c; --errorcase-bg: rgba(224,177,92,.12); --fail: #e57373; --fail-bg: rgba(229,115,115,.14);
    --code-bg: rgba(255,255,255,.04); --shadow: 0 1px 2px rgba(0,0,0,.3), 0 4px 16px rgba(0,0,0,.25);
  }
  @media (prefers-color-scheme: dark) {
    :root:not([data-theme="light"]) {
      --bg: #14181a; --surface: #1b2023; --border: #2b3235; --text: #e7e6e1; --text-muted: #9aa39f;
      --accent: #6fcf9e; --accent-soft: rgba(111,207,158,.1); --pass: #6fcf9e; --pass-bg: rgba(111,207,158,.12);
      --errorcase: #e0b15c; --errorcase-bg: rgba(224,177,92,.12); --fail: #e57373; --fail-bg: rgba(229,115,115,.14);
      --code-bg: rgba(255,255,255,.04); --shadow: 0 1px 2px rgba(0,0,0,.3), 0 4px 16px rgba(0,0,0,.25);
    }
  }
  * { box-sizing: border-box; }
  html, body { margin: 0; padding: 0; background: var(--bg); color: var(--text); font-family: -apple-system,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif; -webkit-font-smoothing: antialiased; }
  .mono { font-family: ui-monospace,"SFMono-Regular","Cascadia Mono",Consolas,"Liberation Mono",Menlo,monospace; }
  .wrap { max-width: 920px; margin: 0 auto; padding: 56px 24px 96px; }
  header.top { display: flex; flex-direction: column; gap: 6px; margin-bottom: 8px; }
  .eyebrow { font-size: 12.5px; letter-spacing: .08em; text-transform: uppercase; color: var(--accent); font-weight: 600; }
  h1 { font-size: clamp(26px,4vw,34px); line-height: 1.15; margin: 0; text-wrap: balance; letter-spacing: -.01em; }
  h1 .prompt { color: var(--accent); font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; font-weight: 500; }
  .lede { color: var(--text-muted); font-size: 15px; line-height: 1.6; max-width: 62ch; margin: 4px 0 0; }
  .summary { display: grid; grid-template-columns: repeat(4,1fr); gap: 1px; background: var(--border); border: 1px solid var(--border); border-radius: 10px; overflow: hidden; margin: 32px 0 40px; box-shadow: var(--shadow); }
  .stat { background: var(--surface); padding: 18px 20px; display: flex; flex-direction: column; gap: 4px; }
  .stat .num { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; font-variant-numeric: tabular-nums; font-size: 26px; font-weight: 600; color: var(--text); }
  .stat .num.pass { color: var(--pass); }
  .stat .label { font-size: 12px; color: var(--text-muted); letter-spacing: .02em; }
  .section-label { font-size: 24px; font-weight: 600; letter-spacing: .08em; text-transform: uppercase; color: var(--text-muted); margin: 40px 0 14px; padding-bottom: 8px; border-bottom: 1px solid var(--border); }
  .cases { display: flex; flex-direction: column; gap: 14px; }
  .case { background: var(--surface); border: 1px solid var(--border); border-radius: 10px; box-shadow: var(--shadow); overflow: hidden; }
  .case-head { display: flex; align-items: baseline; justify-content: space-between; gap: 12px; padding: 14px 18px; border-bottom: 1px solid var(--border); flex-wrap: wrap; }
  .case-title { display: flex; align-items: baseline; gap: 10px; min-width: 0; }
  .case-file { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; font-size: 13.5px; font-weight: 600; color: var(--text); white-space: nowrap; }
  .case-desc { font-size: 13px; color: var(--text-muted); overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
  .chips { display: flex; gap: 6px; flex-shrink: 0; }
  .chip { font-size: 11px; font-weight: 600; padding: 3px 9px; border-radius: 100px; letter-spacing: .01em; white-space: nowrap; }
  .chip.cat { background: var(--accent-soft); color: var(--accent); }
  .chip.pass { background: var(--pass-bg); color: var(--pass); }
  .chip.fail { background: var(--fail-bg); color: var(--fail); }
  .rows { display: flex; flex-direction: column; }
  .assertion { padding: 12px 18px; border-top: 1px solid var(--border); font-size: 13px; }
  .assertion:first-child { border-top: none; }
  .assertion .context { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; font-size: 14px; font-weight: 600; color: var(--text); margin-bottom: 10px; text-align: left; }
  .assertion .context .line-tag { font-family: -apple-system,"Segoe UI",Roboto,sans-serif; font-size: 10.5px; font-weight: 600; letter-spacing: .05em; text-transform: uppercase; color: var(--text-muted); margin-left: 10px; }
  .assertion .compare { display: grid; grid-template-columns: 22px minmax(0,1fr) 22px minmax(0,1fr); align-items: start; gap: 10px; }
  .assertion .note { color: var(--text-muted); grid-column: 2 / -1; }
  .assertion .tick { color: var(--pass); font-weight: 700; text-align: center; line-height: 1.6; }
  .assertion .tick.fail { color: var(--fail); }
  .assertion .tick.info { color: var(--text-muted); }
  .assertion .field-label { font-size: 12px; letter-spacing: .06em; text-transform: uppercase; color: var(--text-muted); display: block; margin-bottom: 2px; }
  .assertion .val { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; background: var(--code-bg); border-radius: 6px; padding: 5px 8px; line-height: 1.55; overflow-x: auto; word-break: break-word; text-align: left; }
  .assertion .arrow { color: var(--text-muted); text-align: center; line-height: 1.6; }
  .errcase-tag { color: var(--errorcase); background: var(--errorcase-bg); }
  .fail-mark { color: var(--fail); font-weight: 700; }
  .turn-label { font-size: 10.5px; letter-spacing: .06em; text-transform: uppercase; font-weight: 600; color: var(--text-muted); display: block; margin-bottom: 2px; }
  .transcript { display: flex; flex-direction: column; }
  .turn { display: grid; grid-template-columns: 22px minmax(0,1fr); gap: 10px; padding: 10px 18px; border-top: 1px solid var(--border); font-size: 13px; align-items: start; }
  .turn:first-child { border-top: none; }
  .turn .tick { color: var(--pass); font-weight: 700; text-align: center; line-height: 1.6; }
  .turn .tick.fail { color: var(--fail); }
  .turn .tick.info { color: var(--text-muted); }
  .turn .out { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; background: var(--code-bg); border-radius: 6px; padding: 5px 8px; line-height: 1.55; overflow-x: auto; word-break: break-word; text-align: left; }
  .turn-input { background: var(--accent-soft); }
  .turn-input .turn-arrow { color: var(--accent); font-weight: 700; text-align: center; line-height: 1.6; }
  .turn-input .cmd-input { font-family: ui-monospace,"SFMono-Regular",Consolas,Menlo,monospace; font-weight: 600; color: var(--accent); text-align: left; background: var(--code-bg); border-radius: 6px; padding: 5px 8px; line-height: 1.55; }
  .turn-input .cmd-input::before { content: "> "; color: var(--text-muted); }
  .turn-input .cmd-input .line-tag { font-family: -apple-system,"Segoe UI",Roboto,sans-serif; font-size: 10.5px; font-weight: 600; letter-spacing: .05em; text-transform: uppercase; color: var(--text-muted); margin-left: 10px; }
  footer { margin-top: 56px; padding-top: 20px; border-top: 1px solid var(--border); color: var(--text-muted); font-size: 12.5px; line-height: 1.7; }
  footer .mono { color: var(--text); }
  @media (max-width: 640px) {
    .summary { grid-template-columns: repeat(2,1fr); }
    .assertion .compare { grid-template-columns: 18px 1fr; } .assertion .arrow { display: none; }
    .turn { grid-template-columns: 18px minmax(0,1fr); }
  }
</style>
<div class="wrap">
  <header class="top">
    <div class="eyebrow">CodeFab Interpreter · 평가 시연 검증</div>
    <h1><span class="prompt">&gt;</span> TestCase 실행 결과 리포트</h1>
    <p class="lede">
      <span class="mono">run_all.bat</span> 실행 시점(${escapeHtml(displayStamp)})에 ${allCases.length}개 스크립트를
      Release 실행 파일로 직접 돌려 캡처한 실제 출력과, 각 소스 파일의 <span class="mono">// expect</span> 주석을
      한 줄씩 새로 대조해서 생성한 리포트입니다.
    </p>
  </header>
  <div class="summary">
    <div class="stat"><span class="num ${filesPassed === allCases.length ? 'pass' : ''}">${filesPassed} / ${allCases.length}</span><span class="label">테스트 파일 통과</span></div>
    <div class="stat"><span class="num ${totalMatched === totalCount ? 'pass' : ''}">${totalMatched} / ${totalCount}</span><span class="label">expect 항목 일치</span></div>
    <div class="stat"><span class="num">${allCases.length}</span><span class="label">전체 테스트케이스 수</span></div>
    <div class="stat"><span class="num">${escapeHtml(stamp)}</span><span class="label">생성 시각(파일명 태그)</span></div>
  </div>
${sectionsHtml}
  <footer>
    실행 명령: <span class="mono">CodeFabIntepreter\\x64\\Release\\CodeFabIntepreter.exe run|debug &lt;파일&gt;</span> ·
    이 리포트는 <span class="mono">run_all.bat</span> 실행마다 <span class="mono">generate_report.js</span>가 새로 생성합니다.
  </footer>
</div>`;

	console.log('All test cases finished.');
	console.log('');

	const outName = `run_all_${stamp}.html`;
	const outPath = path.join(SCRIPT_DIR, outName);
	fs.writeFileSync(outPath, html, 'utf8');
	console.log(`Report written: ${outName}`);
}

main();
