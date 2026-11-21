import { useEffect, useRef, useState } from 'react';
import { Check, Copy, ExternalLink } from 'lucide-react';
import type { LibraryItem } from '../types.ts';

interface CodeViewerProps {
  library: LibraryItem;
}

export function CodeViewer({ library }: CodeViewerProps) {
  const [stripComments, setStripComments] = useState(false);
  const [copyState, setCopyState] = useState<'idle' | 'copied' | 'failed'>('idle');
  const [statusMessage, setStatusMessage] = useState('');

  const timerRef = useRef<number | undefined>(undefined);
  const lastAnnouncementRef = useRef('');

  useEffect(() => {
    return () => {
      if (timerRef.current !== undefined) {
        window.clearTimeout(timerRef.current);
      }
    };
  }, []);

  const showFeedback = (ok: boolean) => {
    if (timerRef.current !== undefined) {
      window.clearTimeout(timerRef.current);
    }

    let message = ok
      ? stripComments
        ? 'コメント抜きでコピーしました'
        : 'コピーしました'
      : 'コピーに失敗しました';

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
    const text = stripComments ? library.strippedCode : library.rawCode;
    if (!text) return;
    try {
      await navigator.clipboard.writeText(text);
      showFeedback(true);
    } catch {
      showFeedback(false);
    }
  };

  return (
    <div className="code-viewer">
      <div className="code-viewer__bar">
        <div className="code-viewer__info">
          <span className="code-viewer__filename">{library.file}</span>
          <span className="code-viewer__lines">{library.lineCount} 行</span>
        </div>

        <div className="code-viewer__controls">
          <button
            type="button"
            className="code-action-btn"
            aria-pressed={stripComments}
            title="ON にするとコメントを除いてコピーします"
            onClick={() => setStripComments((prev) => !prev)}
          >
            コメント抜き
          </button>

          <span className="copy-wrap">
            <button
              type="button"
              className="code-action-btn copy-action-btn"
              data-copied={copyState === 'copied' ? '' : undefined}
              data-failed={copyState === 'failed' ? '' : undefined}
              aria-label={
                stripComments
                  ? `${library.file} をコメント抜きでコピー`
                  : `${library.file} のコードをコピー`
              }
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

      <div
        className="code-viewer__body scroller"
        dangerouslySetInnerHTML={{ __html: library.highlightedCodeHtml }}
      />
    </div>
  );
}
