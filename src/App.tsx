import { useEffect, useState } from 'react';
import { Link, Route, Routes, useLocation, useMatch } from 'react-router-dom';
import { categories, libraries, site } from 'virtual:libraries';
import { Header } from './components/Header.tsx';
import { Sidebar } from './components/Sidebar.tsx';
import { SearchDialog } from './components/SearchDialog.tsx';
import { TableOfContents } from './components/TableOfContents.tsx';
import { HomePage } from './pages/HomePage.tsx';
import { LibraryPage } from './pages/LibraryPage.tsx';
import { useTheme } from './hooks/useTheme.ts';

function isEditableElement(target: EventTarget | null): boolean {
  if (!(target instanceof HTMLElement)) return false;
  const tag = target.tagName;
  return (
    tag === 'INPUT' ||
    tag === 'TEXTAREA' ||
    tag === 'SELECT' ||
    target.isContentEditable
  );
}

export function App() {
  const { resolvedTheme, toggleTheme } = useTheme();
  const [searchOpen, setSearchOpen] = useState(false);
  const [mobileNavOpen, setMobileNavOpen] = useState(false);

  const location = useLocation();
  const libMatch = useMatch('/lib/:id');
  const activeLib = libMatch?.params.id
    ? libraries.find((item) => item.id === libMatch.params.id)
    : undefined;

  // Update document title on route change
  useEffect(() => {
    if (activeLib) {
      document.title = `${activeLib.title} | ${site.title}`;
    } else {
      document.title = site.title;
    }
  }, [activeLib]);

  // Scroll to top or hash anchor on navigation
  useEffect(() => {
    if (location.hash) {
      const targetId = decodeURIComponent(location.hash.slice(1));
      const el = document.getElementById(targetId);
      if (el) {
        el.scrollIntoView({ behavior: 'auto' });
        return;
      }
    }
    window.scrollTo({ top: 0, behavior: 'auto' });
  }, [location.pathname, location.hash]);

  // Global keyboard shortcuts (Ctrl+K / Cmd+K / '/')
  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'k') {
        event.preventDefault();
        setSearchOpen((prev) => !prev);
        return;
      }
      if (
        event.key === '/' &&
        !event.ctrlKey &&
        !event.metaKey &&
        !event.altKey &&
        !searchOpen &&
        !isEditableElement(event.target)
      ) {
        event.preventDefault();
        setSearchOpen(true);
      }
    };
    window.addEventListener('keydown', onKeyDown);
    return () => window.removeEventListener('keydown', onKeyDown);
  }, [searchOpen]);

  const showToc = Boolean(activeLib && activeLib.toc.length > 0);

  return (
    <div className="site-shell">
      <Header
        site={site}
        resolvedTheme={resolvedTheme}
        onToggleTheme={toggleTheme}
        onOpenSearch={() => setSearchOpen(true)}
        mobileNavOpen={mobileNavOpen}
        onOpenMobileNav={() => setMobileNavOpen(true)}
      />

      <div className={showToc ? 'workspace workspace--with-toc' : 'workspace'}>
        <Sidebar
          siteTitle={site.title}
          categories={categories}
          libraries={libraries}
          mobileOpen={mobileNavOpen}
          onCloseMobile={() => setMobileNavOpen(false)}
        />

        <main className="main-content">
          <Routes>
            <Route
              path="/"
              element={
                <HomePage
                  site={site}
                  categories={categories}
                  libraries={libraries}
                />
              }
            />
            <Route
              path="/lib/:id"
              element={<LibraryPage libraries={libraries} />}
            />
            <Route
              path="*"
              element={
                <div className="not-found">
                  <h1>ページが見つかりません</h1>
                  <p>お探しのページは移動または削除された可能性があります。</p>
                  <Link to="/">トップページへ戻る</Link>
                </div>
              }
            />
          </Routes>
        </main>

        {showToc && activeLib && <TableOfContents items={activeLib.toc} />}
      </div>

      <SearchDialog
        libraries={libraries}
        isOpen={searchOpen}
        onClose={() => setSearchOpen(false)}
      />
    </div>
  );
}
