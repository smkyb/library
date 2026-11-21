import { Link, useParams } from 'react-router-dom';
import { ChevronLeft, ChevronRight, ExternalLink, Info } from 'lucide-react';
import { CodeViewer } from '../components/CodeViewer.tsx';
import type { LibraryItem } from '../types.ts';

interface LibraryPageProps {
  libraries: LibraryItem[];
}

export function LibraryPage({ libraries }: LibraryPageProps) {
  const { id } = useParams<{ id: string }>();
  const currentIndex = libraries.findIndex((item) => item.id === id);
  const library = currentIndex >= 0 ? libraries[currentIndex] : undefined;

  if (!library) {
    return (
      <div className="not-found">
        <h1>ライブラリが見つかりません</h1>
        <p>指定されたライブラリ「{id}」は未公開か存在しません。</p>
        <Link to="/">トップページへ戻る</Link>
      </div>
    );
  }

  const prevLib = currentIndex > 0 ? libraries[currentIndex - 1] : undefined;
  const nextLib =
    currentIndex >= 0 && currentIndex < libraries.length - 1
      ? libraries[currentIndex + 1]
      : undefined;

  return (
    <article>
      <nav className="breadcrumb" aria-label="パンくずリスト">
        <Link to="/">ホーム</Link>
        <span aria-hidden="true">/</span>
        <span>{library.categoryLabel}</span>
        <span aria-hidden="true">/</span>
        <span className="breadcrumb__file">{library.file}</span>
      </nav>

      <header className="lib-header">
        <h1 className="lib-title">{library.title}</h1>
        {!library.hasDoc && library.summary && (
          <p className="lib-summary">{library.summary}</p>
        )}

        <div className="lib-meta">
          {library.complexity && (
            <span className="meta-pill meta-pill--mono">{library.complexity}</span>
          )}
          <span className="meta-pill meta-pill--mono">{library.file}</span>
          <span className="meta-pill">{library.lineCount} 行</span>
          {library.tags.map((tag) => (
            <span key={tag} className="tag-badge">
              {tag}
            </span>
          ))}
        </div>

        {library.verified.length > 0 && (
          <div className="verified-links">
            <span className="verified-links__label">Verified:</span>
            {library.verified.map((link) => (
              <a
                key={link.url}
                href={link.url}
                target="_blank"
                rel="noopener noreferrer"
                className="verified-link"
              >
                <span>{link.label}</span>
                <ExternalLink size={12} aria-hidden="true" />
              </a>
            ))}
          </div>
        )}
      </header>

      {!library.hasDoc && library.leadingNotes.length > 0 && (
        <section className="notes-callout" aria-label="コード先頭のメモ">
          <h2 className="notes-callout__title">
            <Info size={15} aria-hidden="true" />
            <span>ソース先頭の注意書き</span>
          </h2>
          <ul className="notes-callout__list">
            {library.leadingNotes.map((note, idx) => (
              <li key={idx}>{note}</li>
            ))}
          </ul>
        </section>
      )}

      {library.hasDoc && library.docHtml && (
        <div
          className="markdown-body"
          dangerouslySetInnerHTML={{ __html: library.docHtml }}
        />
      )}

      <section id="source-code" className="source-section">
        <h2 className="source-section__heading">ソースコード</h2>
        <CodeViewer library={library} />
      </section>

      {(prevLib || nextLib) && (
        <nav className="lib-pager" aria-label="前後のライブラリ">
          {prevLib ? (
            <Link to={`/lib/${prevLib.id}`} className="pager-link">
              <span className="pager-link__dir">
                <ChevronLeft size={13} aria-hidden="true" style={{ verticalAlign: '-2px' }} /> 前のライブラリ
              </span>
              <span className="pager-link__title">{prevLib.title}</span>
            </Link>
          ) : (
            <div />
          )}

          {nextLib && (
            <Link to={`/lib/${nextLib.id}`} className="pager-link pager-link--next">
              <span className="pager-link__dir">
                次のライブラリ <ChevronRight size={13} aria-hidden="true" style={{ verticalAlign: '-2px' }} />
              </span>
              <span className="pager-link__title">{nextLib.title}</span>
            </Link>
          )}
        </nav>
      )}
    </article>
  );
}
