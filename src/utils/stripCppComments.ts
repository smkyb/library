/**
 * Extracts contiguous leading single-line (`// ...`) comments from the top of a C++ file.
 * Useful as automatic fallback notes when a dedicated Markdown doc does not exist yet.
 */
export function extractLeadingComments(code: string): string[] {
  const normalized = code.replace(/\r\n/g, '\n');
  const lines = normalized.split('\n');
  const notes: string[] = [];
  let inLeadingBlock = false;

  for (const line of lines) {
    const trimmed = line.trim();
    if (!inLeadingBlock && trimmed === '') {
      continue;
    }
    if (trimmed.startsWith('//')) {
      inLeadingBlock = true;
      const body = trimmed.slice(2).trim();
      if (body.length > 0) {
        notes.push(body);
      }
      continue;
    }
    break;
  }

  return notes;
}

function isIdentChar(ch: string | undefined): boolean {
  if (!ch) return false;
  return /[A-Za-z0-9_]/.test(ch);
}

/**
 * Strips C++ `//` line comments and block comments while preserving string literals,
 * character literals, and C++11 raw string literals (`R"delim(...)delim"`).
 * Lines that consisted solely of comments and whitespace are removed rather than leaving
 * empty lines behind.
 */
export function stripCppComments(code: string): string {
  const normalized = code.replace(/\r\n/g, '\n');
  const len = normalized.length;

  let out = '';
  let lineStartInOut = 0;
  let lineHadNonWhitespaceCode = false;
  let lineHadComment = false;
  let i = 0;

  const commitLineBreak = () => {
    const lineContent = out.slice(lineStartInOut);
    if (lineHadComment && !lineHadNonWhitespaceCode) {
      out = out.slice(0, lineStartInOut);
      return;
    }
    if (lineHadComment) {
      const trimmedEnd = lineContent.replace(/[ \t]+$/, '');
      out = out.slice(0, lineStartInOut) + trimmedEnd;
    }
    out += '\n';
    lineStartInOut = out.length;
    lineHadNonWhitespaceCode = false;
    lineHadComment = false;
  };

  while (i < len) {
    const ch = normalized[i];
    const next = i + 1 < len ? normalized[i + 1] : '';

    // C++11 raw string literal: optional encoding prefix (u8, u, U, L) + R"delim(...)delim"
    if (ch === 'R' && next === '"') {
      const prev = i > 0 ? normalized[i - 1] : undefined;
      const prefixAllowed =
        !isIdentChar(prev) ||
        (i >= 1 && (prev === 'L' || prev === 'u' || prev === 'U') && !isIdentChar(normalized[i - 2])) ||
        (i >= 2 && normalized.slice(i - 2, i) === 'u8' && !isIdentChar(normalized[i - 3]));

      if (prefixAllowed) {
        const parenIdx = normalized.indexOf('(', i + 2);
        if (parenIdx !== -1) {
          const delim = normalized.slice(i + 2, parenIdx);
          if (!/[\s()\\]/.test(delim)) {
            const closeMarker = `)${delim}"`;
            const closeIdx = normalized.indexOf(closeMarker, parenIdx + 1);
            if (closeIdx !== -1) {
              const rawEnd = closeIdx + closeMarker.length;
              const rawToken = normalized.slice(i, rawEnd);
              for (let k = 0; k < rawToken.length; k += 1) {
                const rc = rawToken[k];
                if (rc === '\n') {
                  out += '\n';
                  lineStartInOut = out.length;
                  lineHadNonWhitespaceCode = true;
                  lineHadComment = false;
                } else {
                  out += rc;
                }
              }
              lineHadNonWhitespaceCode = true;
              i = rawEnd;
              continue;
            }
          }
        }
      }
    }

    // Standard string or character literal
    if (ch === '"' || ch === "'") {
      const quote = ch;
      out += ch;
      i += 1;
      while (i < len) {
        const sc = normalized[i];
        if (sc === '\\' && i + 1 < len) {
          out += sc;
          const escaped = normalized[i + 1];
          out += escaped;
          if (escaped === '\n') {
            lineStartInOut = out.length;
            lineHadNonWhitespaceCode = true;
            lineHadComment = false;
          }
          i += 2;
          continue;
        }
        out += sc;
        i += 1;
        if (sc === quote) {
          break;
        }
        if (sc === '\n') {
          lineStartInOut = out.length;
          lineHadNonWhitespaceCode = true;
          lineHadComment = false;
          break;
        }
      }
      lineHadNonWhitespaceCode = true;
      continue;
    }

    // Line comment
    if (ch === '/' && next === '/') {
      lineHadComment = true;
      i += 2;
      while (i < len && normalized[i] !== '\n') {
        i += 1;
      }
      continue;
    }

    // Block comment
    if (ch === '/' && next === '*') {
      lineHadComment = true;
      const beforeChar = out.length > lineStartInOut ? out[out.length - 1] : undefined;
      i += 2;
      let crossedNewline = false;
      while (i < len && !(normalized[i] === '*' && normalized[i + 1] === '/')) {
        if (normalized[i] === '\n') {
          crossedNewline = true;
        }
        i += 1;
      }
      if (i < len) {
        i += 2;
      }
      const afterChar = i < len ? normalized[i] : undefined;
      if (crossedNewline) {
        if (!lineHadNonWhitespaceCode) {
          out = out.slice(0, lineStartInOut);
        } else {
          out = out.slice(0, lineStartInOut) + out.slice(lineStartInOut).replace(/[ \t]+$/, '');
          out += '\n';
          lineStartInOut = out.length;
          lineHadNonWhitespaceCode = false;
        }
        lineHadComment = true;
      } else if (isIdentChar(beforeChar) && isIdentChar(afterChar)) {
        out += ' ';
      }
      continue;
    }

    if (ch === '\n') {
      commitLineBreak();
      i += 1;
      continue;
    }

    if (ch !== ' ' && ch !== '\t' && ch !== '\r') {
      lineHadNonWhitespaceCode = true;
    }
    out += ch;
    i += 1;
  }

  // Handle final line if there was no trailing newline
  if (out.length > lineStartInOut) {
    const tail = out.slice(lineStartInOut);
    if (lineHadComment && !lineHadNonWhitespaceCode) {
      out = out.slice(0, lineStartInOut);
    } else if (lineHadComment) {
      out = out.slice(0, lineStartInOut) + tail.replace(/[ \t]+$/, '');
    }
  }

  return out.replace(/^\n+/, '').replace(/\n+$/, '');
}

type CppTokenKind = 'preproc' | 'word' | 'string' | 'char' | 'raw_string' | 'punct';

interface CppToken {
  kind: CppTokenKind;
  text: string;
  hadSpaceBefore: boolean;
}

const COMPACT_SOFT_LINE_LIMIT = 180;
const COMPACT_HARD_LINE_LIMIT = 240;

const MULTI_CHAR_PUNCTS = [
  '<<=',
  '>>=',
  '<=>',
  '->*',
  '...',
  '::',
  '.*',
  '->',
  '++',
  '--',
  '<<',
  '>>',
  '<=',
  '>=',
  '==',
  '!=',
  '&&',
  '||',
  '+=',
  '-=',
  '*=',
  '/=',
  '%=',
  '^=',
  '&=',
  '|=',
  '<:',
  ':>',
  '<%',
  '%>',
  '%:',
  '##',
];

const DANGEROUS_PUNCT_PAIRS = new Set([
  '/*',
  '//',
  '++',
  '--',
  '<<',
  '>>',
  '<=',
  '>=',
  '==',
  '!=',
  '&&',
  '||',
  '+=',
  '-=',
  '*=',
  '/=',
  '%=',
  '^=',
  '&=',
  '|=',
  '::',
  '->',
  '.*',
  '..',
  '<:',
  ':>',
  '<%',
  '%>',
  '%:',
  '[[',
  '##',
]);

function isWordStart(ch: string | undefined): boolean {
  if (!ch) return false;
  return /[A-Za-z_]/.test(ch);
}

function isDigit(ch: string | undefined): boolean {
  if (!ch) return false;
  return ch >= '0' && ch <= '9';
}

function matchStringPrefix(src: string, index: number): number {
  if (src.startsWith('u8', index)) return 2;
  const ch = src[index];
  if (ch === 'u' || ch === 'U' || ch === 'L') return 1;
  return 0;
}

function tokenizeCppForCompact(stripped: string): CppToken[] {
  const tokens: CppToken[] = [];
  const len = stripped.length;
  let i = 0;
  let atLineStart = true;
  let pendingSpace = false;

  while (i < len) {
    const ch = stripped[i];

    // Whitespace
    if (ch === ' ' || ch === '\t' || ch === '\r' || ch === '\n') {
      if (ch === '\n') {
        atLineStart = true;
      }
      pendingSpace = true;
      i += 1;
      continue;
    }

    // Preprocessor directive (must remain on its own logical line, preserving `\` continuations)
    if (atLineStart && ch === '#') {
      const start = i;
      while (i < len) {
        if (stripped[i] === '\\' && stripped[i + 1] === '\n') {
          i += 2;
          continue;
        }
        if (stripped[i] === '\n') {
          break;
        }
        i += 1;
      }
      const directive = stripped.slice(start, i).replace(/[ \t]+$/, '');
      tokens.push({ kind: 'preproc', text: directive, hadSpaceBefore: pendingSpace });
      if (i < len && stripped[i] === '\n') {
        i += 1;
      }
      atLineStart = true;
      pendingSpace = true;
      continue;
    }

    atLineStart = false;

    // C++11 raw string literal with optional encoding prefix: (u8|u|U|L)?R"delim(...)delim"
    const prefixLen = matchStringPrefix(stripped, i);
    const rawStart = i + prefixLen;
    if (stripped[rawStart] === 'R' && stripped[rawStart + 1] === '"') {
      const parenIdx = stripped.indexOf('(', rawStart + 2);
      if (parenIdx !== -1) {
        const delim = stripped.slice(rawStart + 2, parenIdx);
        if (!/[\s()\\]/.test(delim)) {
          const closeMarker = `)${delim}"`;
          const closeIdx = stripped.indexOf(closeMarker, parenIdx + 1);
          if (closeIdx !== -1) {
            const rawEnd = closeIdx + closeMarker.length;
            tokens.push({
              kind: 'raw_string',
              text: stripped.slice(i, rawEnd),
              hadSpaceBefore: pendingSpace,
            });
            i = rawEnd;
            pendingSpace = false;
            continue;
          }
        }
      }
    }

    // String or character literal with optional encoding prefix
    const quotePos = i + prefixLen;
    if (stripped[quotePos] === '"' || stripped[quotePos] === "'") {
      const quote = stripped[quotePos];
      let j = quotePos + 1;
      while (j < len) {
        if (stripped[j] === '\\' && j + 1 < len) {
          j += 2;
          continue;
        }
        const cur = stripped[j];
        j += 1;
        if (cur === quote || cur === '\n') {
          break;
        }
      }
      tokens.push({
        kind: quote === '"' ? 'string' : 'char',
        text: stripped.slice(i, j),
        hadSpaceBefore: pendingSpace,
      });
      i = j;
      pendingSpace = false;
      continue;
    }

    // Preprocessing number (handles 1'000'000, 1e-9, 0x1.fp+3, .5, 123ULL)
    if (isDigit(ch) || (ch === '.' && isDigit(stripped[i + 1]))) {
      let j = i + 1;
      while (j < len) {
        const c = stripped[j];
        const prev = stripped[j - 1];
        if ((c === '+' || c === '-') && (prev === 'e' || prev === 'E' || prev === 'p' || prev === 'P')) {
          j += 1;
          continue;
        }
        if (c === "'" && isIdentChar(stripped[j + 1])) {
          j += 2;
          continue;
        }
        if (isIdentChar(c) || c === '.') {
          j += 1;
          continue;
        }
        break;
      }
      tokens.push({ kind: 'word', text: stripped.slice(i, j), hadSpaceBefore: pendingSpace });
      i = j;
      pendingSpace = false;
      continue;
    }

    // Identifier / keyword
    if (isWordStart(ch)) {
      let j = i + 1;
      while (j < len && isIdentChar(stripped[j])) {
        j += 1;
      }
      tokens.push({ kind: 'word', text: stripped.slice(i, j), hadSpaceBefore: pendingSpace });
      i = j;
      pendingSpace = false;
      continue;
    }

    // Multi-character or single-character punctuator
    let matchedPunct = ch;
    for (const p of MULTI_CHAR_PUNCTS) {
      if (stripped.startsWith(p, i)) {
        matchedPunct = p;
        break;
      }
    }
    tokens.push({ kind: 'punct', text: matchedPunct, hadSpaceBefore: pendingSpace });
    i += matchedPunct.length;
    pendingSpace = false;
  }

  return tokens;
}

function needsSpaceBetween(prev: CppToken, next: CppToken): boolean {
  if (!next.hadSpaceBefore) {
    return false;
  }

  const prevIsLiteral = prev.kind === 'string' || prev.kind === 'char' || prev.kind === 'raw_string';
  const nextIsLiteral = next.kind === 'string' || next.kind === 'char' || next.kind === 'raw_string';

  if (prev.kind === 'word' && next.kind === 'word') return true;
  if (prev.kind === 'word' && nextIsLiteral) return true;
  if (prevIsLiteral && next.kind === 'word') return true;

  if (prev.kind === 'punct' && next.kind === 'punct') {
    const pair = prev.text[prev.text.length - 1] + next.text[0];
    if (DANGEROUS_PUNCT_PAIRS.has(pair)) return true;
  }

  // Prevent merging a number with a leading dot punctuator (rare, e.g. `1 .`)
  if (prev.kind === 'word' && next.text.startsWith('.')) return true;
  if (prev.text.endsWith('.') && next.kind === 'word') return true;

  return false;
}

function isClosingOrSeparator(tok: CppToken): boolean {
  return tok.kind === 'punct' && (tok.text === ';' || tok.text === ',' || tok.text === ')' || tok.text === ']');
}

/**
 * Compacts C++ code for pasting into editors: removes comments and excess whitespace
 * while preserving preprocessor directives, string/raw-string literals, and token boundaries,
 * and inserts moderate line breaks at statement/block boundaries so editors stay responsive.
 */
export function compactCppCode(
  code: string,
  softLineLimit = COMPACT_SOFT_LINE_LIMIT,
  hardLineLimit = COMPACT_HARD_LINE_LIMIT,
): string {
  const stripped = stripCppComments(code);
  if (!stripped) return '';

  const tokens = tokenizeCppForCompact(stripped);
  const lines: string[] = [];
  let currentLine = '';
  let prevTok: CppToken | null = null;

  const flushLine = () => {
    if (currentLine.length > 0) {
      lines.push(currentLine);
      currentLine = '';
      prevTok = null;
    }
  };

  for (const tok of tokens) {
    if (tok.kind === 'preproc') {
      flushLine();
      lines.push(tok.text);
      continue;
    }

    if (prevTok !== null && currentLine.length > 0) {
      const space = needsSpaceBetween(prevTok, tok);
      const addedLen = (space ? 1 : 0) + tok.text.length;
      const cleanBoundary =
        prevTok.kind === 'punct' &&
        (prevTok.text === ';' || (prevTok.text === '}' && !isClosingOrSeparator(tok)));

      if (cleanBoundary && currentLine.length >= softLineLimit) {
        flushLine();
      } else if (
        currentLine.length + addedLen > hardLineLimit &&
        (!isClosingOrSeparator(tok) || currentLine.length > hardLineLimit + 40)
      ) {
        flushLine();
      }
    }

    const space = prevTok !== null && currentLine.length > 0 && needsSpaceBetween(prevTok, tok);
    const piece = (space ? ' ' : '') + tok.text;

    if (tok.kind === 'raw_string' && piece.includes('\n')) {
      const combined = currentLine + piece;
      const parts = combined.split('\n');
      for (let k = 0; k < parts.length - 1; k += 1) {
        lines.push(parts[k]!);
      }
      currentLine = parts[parts.length - 1]!;
    } else {
      currentLine += piece;
    }
    prevTok = tok;
  }

  flushLine();
  return lines.join('\n');
}
