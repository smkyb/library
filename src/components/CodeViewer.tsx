import { useEffect, useId, useMemo, useRef, useState } from 'react';
import { Check, ChevronDown, ChevronRight, Copy, ExternalLink } from 'lucide-react';
import type { LibraryItem } from '../types.ts';
import { compactCppCode } from '../utils/stripCppComments.ts';

type CopyMode = 'raw' | 'stripped' | 'compact';

interface CodeViewerProps {
  library: LibraryItem;
}

export function CodeViewer({ library }: CodeViewerProps) {
  const [isExpanded, setIsExpanded] = useState(false);
  const [copyMode, setCopyMode] = useState<CopyMode>('raw');
  const [copyState, setCopyState] = useState<'idle' | 'copied' | 'failed'>('idle');
  const [statusMessage, setStatusMessage] = useState('');
  const bodyId = useId();

  const compactCode = useMemo(() => compactCppCode(library.rawCode), [library.rawCode]);
  const compactLineCount = useMemo(
    () => (compactCode === '' ? 0 : compactCode.split('\n').length),
    [compactCode],
  );

  const timerRef = useRef<number | undefined>(undefined);
  const lastAnnouncementRef = useRef('');

  useEffect(() => {
    return () => {
      if (timerRef.current !== undefined) {
        window.clearTimeout(timerRef.current);
      }
    };
  }, []);

  const showFeedback = (ok: boolean, mode: CopyMode) => {
    if (timerRef.current !== undefined) {
      window.clearTimeout(timerRef.current);
    }

    let message = 'コピーに失敗しました';
    if (ok) {
      if (mode === 'compact') {
        message = `圧縮してコピーしました（${compactLineCount} 行）`;
      } else if (mode === 'stripped') {
        message = 'コメント抜きでコピーしました';
      } else {
        message = 'コピーしました';
      }
    }

    // Append NBSP when repeating the exact same message so VoiceOver announces it again
    if (message === lastAnnouncementRef.current) {
      message += '\u00A0';
    }
    lastAnnouncementRef.current = message;

    setCopyState(ok ? 'copied' : 'failed');
    setStatusMessage('');
    requestAnimationFrame(() => {
      setStatusMessage(message);
    });

    timerRef.current = window.setTimeout(() => {
      setCopyState('idle');
      setStatusMessage('');
    }, 2000);
  };

  const handleCopy = async () => {
    const text =
      copyMode === 'compact'
        ? compactCode
        : copyMode === 'stripped'
          ? library.strippedCode
          : library.rawCode;
    if (!text) return;
    try {
      await navigator.clipboard.writeText(text);
      showFeedback(true, copyMode);
    } catch {
      showFeedback(false, copyMode);
    }
  };

  const copyAriaLabel =
    copyMode === 'compact'
      ? `${library.file} を圧縮してコピー（${compactLineCount} 行）`
      : copyMode === 'stripped'
        ? `${library.file} をコメント抜きでコピー`
        : `${library.file} のコードをコピー`;

  return (
    <div className="code-viewer" data-expanded={isExpanded ? '' : undefined}>
      <div className="code-viewer__bar">
        <button
          type="button"
          className="code-viewer__toggle"
          aria-expanded={isExpanded}
          aria-controls={bodyId}
          onClick={() => setIsExpanded((prev) => !prev)}
        >
          {isExpanded ? (
            <ChevronDown size={15} className="code-viewer__chevron" aria-hidden="true" />
          ) : (
            <ChevronRight size={15} className="code-viewer__chevron" aria-hidden="true" />
          )}
          <span className="code-viewer__filename">{library.file}</span>
          <span className="code-viewer__lines">
            {copyMode === 'compact'
              ? `${library.lineCount} 行（圧縮時 ${compactLineCount} 行）`
              : `${library.lineCount} 行`}
          </span>
          <span className="code-viewer__toggle-hint">
            {isExpanded ? '折りたたむ' : '展開'}
          </span>
        </button>

        <div className="code-viewer__controls">
          <button
            type="button"
            className="code-action-btn"
            aria-pressed={copyMode === 'stripped'}
            title="ON にするとコメントを除いてコピーします"
            onClick={() => setCopyMode((prev) => (prev === 'stripped' ? 'raw' : 'stripped'))}
          >
            コメント抜き
          </button>

          <button
            type="button"
            className="code-action-btn"
            aria-pressed={copyMode === 'compact'}
            title={`ON にするとコメントを除き、適度に改行して圧縮コピーします（${compactLineCount} 行）`}
            onClick={() => setCopyMode((prev) => (prev === 'compact' ? 'raw' : 'compact'))}
          >
            圧縮
          </button>

          <span className="copy-wrap">
            <button
              type="button"
              className="code-action-btn copy-action-btn"
              data-copied={copyState === 'copied' ? '' : undefined}
              data-failed={copyState === 'failed' ? '' : undefined}
              aria-label={copyAriaLabel}
              onClick={() => {
                void handleCopy();
              }}
            >
              {copyState === 'copied' ? (
                <Check size={14} aria-hidden="true" />
              ) : (
                <Copy size={14} aria-hidden="true" />
              )}
              <span>{copyState === 'copied' ? 'コピー済' : 'コピー'}</span>
            </button>
            <span className="copy-status" role="status">
              {statusMessage}
            </span>
          </span>

          <a
            href={library.githubUrl}
            target="_blank"
            rel="noopener noreferrer"
            className="code-action-btn"
            aria-label={`${library.file} を GitHub で開く`}
          >
            <ExternalLink size={14} aria-hidden="true" />
            <span>GitHub</span>
          </a>
        </div>
      </div>

      {isExpanded && (
        <div
          id={bodyId}
          className="code-viewer__body scroller"
          dangerouslySetInnerHTML={{ __html: library.highlightedCodeHtml }}
        />
      )}
    </div>
  );
}
