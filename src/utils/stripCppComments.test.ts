import { describe, it } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { compactCppCode, extractLeadingComments, stripCppComments } from './stripCppComments.ts';

const testDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(testDir, '..', '..');

describe('stripCppComments', () => {
  it('removes leading comment lines without leaving leading blank lines', () => {
    const input = [
      '// 注意: line 1',
      '//       line 2',
      'int add(int a, int b) {',
      '    return a + b;',
      '}',
    ].join('\n');

    const expected = [
      'int add(int a, int b) {',
      '    return a + b;',
      '}',
    ].join('\n');

    assert.equal(stripCppComments(input), expected);
  });

  it('strips trailing inline line and block comments and trims trailing spaces', () => {
    const input = [
      'int x = 1; // inline comment',
      'int y = 2; /* trailing block */  ',
      'int z = 3; /* mid */ + 4;',
    ].join('\n');

    const expected = [
      'int x = 1;',
      'int y = 2;',
      'int z = 3;  + 4;',
    ].join('\n');

    assert.equal(stripCppComments(input), expected);
  });

  it('inserts a separating space when a block comment splits two identifiers', () => {
    assert.equal(stripCppComments('int/*c*/x = 0;'), 'int x = 0;');
  });

  it('preserves comment-like sequences inside strings, chars, and raw strings', () => {
    const input = [
      'string url = "https://example.com/a/*b*/"; // real comment',
      'string esc = "quote \\"//still string\\"";',
      "char slash = '/';",
      'const char* raw = R"delim(// keep this\n/* and this */)delim";',
    ].join('\n');

    const expected = [
      'string url = "https://example.com/a/*b*/";',
      'string esc = "quote \\"//still string\\"";',
      "char slash = '/';",
      'const char* raw = R"delim(// keep this\n/* and this */)delim";',
    ].join('\n');

    assert.equal(stripCppComments(input), expected);
  });

  it('preserves intentional blank lines while deleting comment-only lines', () => {
    const input = [
      'int a = 1;',
      '',
      '// only comment',
      'int b = 2;',
    ].join('\n');

    const expected = [
      'int a = 1;',
      '',
      'int b = 2;',
    ].join('\n');

    assert.equal(stripCppComments(input), expected);
  });

  it('handles real library files (dijkstra.cpp and hld.cpp)', () => {
    const dijkstraRaw = fs.readFileSync(path.join(repoRoot, 'libraries', 'dijkstra.cpp'), 'utf8');
    const dijkstraStripped = stripCppComments(dijkstraRaw);
    assert.ok(!dijkstraStripped.includes('// 注意:'));
    assert.ok(dijkstraStripped.startsWith('struct _Dijkstra {'));

    const hldRaw = fs.readFileSync(path.join(repoRoot, 'libraries', 'hld.cpp'), 'utf8');
    const hldStripped = stripCppComments(hldRaw);
    assert.ok(!hldStripped.includes('// 注意:'));
    assert.ok(hldStripped.startsWith('template<typename node_type'));
  });
});

describe('extractLeadingComments', () => {
  it('extracts leading // lines and ignores comments later in the file', () => {
    const input = [
      '// 注意: line 1',
      '//       line 2',
      'struct Foo {',
      '    // inside comment',
      '};',
    ].join('\n');

    assert.deepEqual(extractLeadingComments(input), ['注意: line 1', 'line 2']);
  });
});

describe('compactCppCode', () => {
  it('removes comments and excess whitespace while preserving token boundaries', () => {
    const input = [
      '// leading comment',
      'template<typename T>',
      'T add(const T& a, const T& b) {',
      '    return a + b; // trailing',
      '}',
    ].join('\n');

    assert.equal(
      compactCppCode(input),
      'template<typename T>T add(const T&a,const T&b){return a+b;}',
    );
  });

  it('keeps preprocessor directives on their own lines including continuations', () => {
    const input = [
      '#include <bits/stdc++.h>',
      '#define FOO(x) \\',
      '    ((x) + 1)',
      'int f(int a) {',
      '    #ifdef DEBUG',
      '    return FOO(a);',
      '    #else',
      '    return a;',
      '    #endif',
      '}',
    ].join('\n');

    const expected = [
      '#include <bits/stdc++.h>',
      '#define FOO(x) \\',
      '    ((x) + 1)',
      'int f(int a){',
      '#ifdef DEBUG',
      'return FOO(a);',
      '#else',
      'return a;',
      '#endif',
      '}',
    ].join('\n');

    assert.equal(compactCppCode(input), expected);
  });

  it('prevents dangerous token pasting and preserves attributes, numbers, and literals', () => {
    const input = [
      'int x = a + +b - -c;',
      'int y = d / *p;',
      'if (x < ::g) [[likely]] {',
      '    double z = 1e-9 + 1\'000\'000 + .5;',
      '    const char* s = "a + b // keep";',
      '    const char* r = R"d(line1\n// line2)d";',
      '    int v = arr[ [&]{ return 0; }() ];',
      '}',
    ].join('\n');

    const compacted = compactCppCode(input, 500, 600);
    assert.ok(compacted.includes('a+ +b- -c'));
    assert.ok(compacted.includes('d/ *p'));
    assert.ok(compacted.includes('x< ::g'));
    assert.ok(compacted.includes('[[likely]]'));
    assert.ok(compacted.includes("1e-9+1'000'000+.5"));
    assert.ok(compacted.includes('"a + b // keep"'));
    assert.ok(compacted.includes('R"d(line1\n// line2)d"'));
    assert.ok(compacted.includes('arr[ [&]{return 0;}()]'));
  });

  it('inserts moderate line breaks on large files like sortable_segtree.cpp', () => {
    const raw = fs.readFileSync(path.join(repoRoot, 'libraries', 'sortable_segtree.cpp'), 'utf8');
    const rawLines = raw.replace(/\r\n/g, '\n').split('\n').length;
    const compacted = compactCppCode(raw);
    const compactLines = compacted.split('\n');

    assert.ok(compactLines.length > 20, 'should keep multiple moderate lines');
    assert.ok(compactLines.length < rawLines / 4, 'should significantly reduce line count');
    for (const line of compactLines) {
      assert.ok(line.length <= 320, `line too long: ${line.length}`);
    }
  });
});
