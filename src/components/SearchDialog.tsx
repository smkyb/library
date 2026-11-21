import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { Search, X } from 'lucide-react';
import type { LibraryItem } from '../types.ts';

interface SearchDialogProps {
  libraries: LibraryItem[];
  isOpen: boolean;
  onClose: () => void;
}

function scoreLibrary(lib: LibraryItem, terms: string[]): number {
  const titleLower = lib.title.toLowerCase();
  const idLower = lib.id.toLowerCase();
  const fileLower = lib.file.toLowerCase();
  const summaryLower = lib.summary.toLowerCase();
  const catLower = lib.categoryLabel.toLowerCase();
  const tagsLower = lib.tags.map((t) => t.toLowerCase());

  let score = 0;
  for (const term of terms) {
    if (!lib.searchText.includes(term)) {
      return -1;
    }
    if (idLower === term || fileLower === term) {
      score += 100;
    } else if (titleLower.includes(term)) {
      score += 40;
    } else if (idLower.includes(term) || fileLower.includes(term)) {
      score += 30;
    } else if (tagsLower.some((tag) => tag.includes(term))) {
      score += 20;
    } else if (summaryLower.includes(term) || catLower.includes(term)) {
      score += 10;
    } else {
      score += 2;
    }
  }
  return score;
}

export function SearchDialog({ libraries, isOpen, onClose }: SearchDialogProps) {
  const dialogRef = useRef<HTMLDialogElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const navigate = useNavigate();

  const [query, setQuery] = useState('');
  const [activeIndex, setActiveIndex] = useState(0);

  const results = useMemo(() => {
    const normalized = query.trim().toLowerCase();
    if (!normalized) {
      return libraries;
    }
    const terms = normalized.split(/\s+/).filter(Boolean);
    return libraries
      .map((lib) => ({ lib, score: scoreLibrary(lib, terms) }))
      .filter((entry) => entry.score >= 0)
      .sort((a, b) => b.score - a.score)
      .map((entry) => entry.lib);
  }, [libraries, query]);

  const clampedIndex = results.length === 0 ? -1 : Math.min(activeIndex, results.length - 1);

  const selectLibrary = useCallback(
    (lib: LibraryItem) => {
      dialogRef.current?.close();
      navigate(`/lib/${lib.id}`);
    },
    [navigate],
  );

  useEffect(() => {
    const dialog = dialogRef.current;
    if (!dialog) return;

    if (isOpen) {
      if (!dialog.open) {
        dialog.showModal();
      }
      setQuery('');
      setActiveIndex(0);
      requestAnimationFrame(() => inputRef.current?.focus());
    } else if (dialog.open) {
      dialog.close();
    }
  }, [isOpen]);

  useEffect(() => {
    const dialog = dialogRef.current;
    if (!dialog) return;

    const onNativeClose = () => {
      dialog.returnValue = '';
      onClose();
    };
    dialog.addEventListener('close', onNativeClose);

    // Light-dismiss fallback for browsers lacking <dialog closedby="any">
    let removeFallback: (() => void) | undefined;
    if (!('closedBy' in HTMLDialogElement.prototype)) {
      const outside = (e: PointerEvent) => {
        const r = dialog.getBoundingClientRect();
        return (
          e.clientX < r.left ||
          e.clientX > r.right ||
          e.clientY < r.top ||
          e.clientY > r.bottom
        );
      };
      let pressedOutside = false;
      const onPointerDown = (e: PointerEvent) => {
        pressedOutside = outside(e);
      };
      const onPointerUp = (e: PointerEvent) => {
        if (pressedOutside && outside(e) && dialog.open) {
          dialog.close();
        }
        pressedOutside = false;
      };
      dialog.addEventListener('pointerdown', onPointerDown);
      dialog.addEventListener('pointerup', onPointerUp);
      removeFallback = () => {
        dialog.removeEventListener('pointerdown', onPointerDown);
        dialog.removeEventListener('pointerup', onPointerUp);
      };
    }

    return () => {
      dialog.removeEventListener('close', onNativeClose);
      removeFallback?.();
    };
  }, [onClose]);

  const handleKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (results.length === 0) return;

    if (event.key === 'ArrowDown') {
      event.preventDefault();
      setActiveIndex((prev) => (prev + 1) % results.length);
    } else if (event.key === 'ArrowUp') {
      event.preventDefault();
      setActiveIndex((prev) => (prev - 1 + results.length) % results.length);
    } else if (event.key === 'Enter' && clampedIndex >= 0) {
      event.preventDefault();
      const chosen = results[clampedIndex];
      if (chosen) {
        selectLibrary(chosen);
      }
    }
  };

  return (
    <dialog
      ref={dialogRef}
      className="search-dialog"
      closedby="any"
      aria-label="ライブラリ検索"
    >
      <div className="search-dialog__header">
        <Search size={18} className="search-dialog__icon" aria-hidden="true" />
        <input
          ref={inputRef}
          type="search"
          className="search-dialog__input"
          placeholder="タイトル・ファイル名・タグ・解説を検索..."
          aria-label="検索キーワード"
          value={query}
          autoFocus
          onChange={(e) => {
            setQuery(e.target.value);
            setActiveIndex(0);
          }}
          onKeyDown={handleKeyDown}
        />
        <button
          type="button"
          className="search-dialog__close"
          aria-label="検索を閉じる"
          onClick={() => dialogRef.current?.close()}
        >
          <X size={15} aria-hidden="true" />
        </button>
      </div>

      {results.length > 0 ? (
        <ul className="search-dialog__results" role="listbox" aria-label="検索結果">
          {results.map((lib, idx) => (
            <li key={lib.id} className="search-dialog__item">
              <button
                type="button"
                role="option"
                aria-selected={idx === clampedIndex}
                data-active={idx === clampedIndex}
                className="search-dialog__option"
                onMouseEnter={() => setActiveIndex(idx)}
                onClick={() => selectLibrary(lib)}
              >
                <div className="search-dialog__option-top">
                  <span className="search-dialog__option-title">{lib.title}</span>
                  <span className="search-dialog__option-meta">
                    {lib.categoryLabel} · {lib.file}
                  </span>
                </div>
                {lib.summary && (
                  <div className="search-dialog__option-summary">{lib.summary}</div>
                )}
              </button>
            </li>
          ))}
        </ul>
      ) : (
        <div className="search-dialog__empty">
          「{query}」に一致するライブラリが見つかりません。
        </div>
      )}

      <div className="search-dialog__footer">
        <div className="search-dialog__hints" aria-hidden="true">
          <span>
            <kbd>↑</kbd> <kbd>↓</kbd> 移動
          </span>
          <span>
            <kbd>Enter</kbd> 開く
          </span>
          <span>
            <kbd>Esc</kbd> 閉じる
          </span>
        </div>
        <span className="search-dialog__count">{results.length} 件</span>
      </div>
    </dialog>
  );
}
