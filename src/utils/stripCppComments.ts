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
