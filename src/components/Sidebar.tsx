import { useCallback, useEffect, useMemo, useRef } from 'react';
import { NavLink } from 'react-router-dom';
import { X } from 'lucide-react';
import type { Category, LibraryItem } from '../types.ts';

interface SidebarProps {
  siteTitle: string;
  categories: Category[];
  libraries: LibraryItem[];
  mobileOpen: boolean;
  onCloseMobile: () => void;
}

interface NavTreeProps {
  categories: Category[];
  libraries: LibraryItem[];
  onNavigate?: () => void;
}

function NavTree({ categories, libraries, onNavigate }: NavTreeProps) {
  const grouped = useMemo(() => {
    const byCat = new Map<string, LibraryItem[]>();
    for (const lib of libraries) {
      const list = byCat.get(lib.category);
      if (list) {
        list.push(lib);
      } else {
        byCat.set(lib.category, [lib]);
      }
    }
    return categories
      .map((cat) => ({
        category: cat,
        items: byCat.get(cat.id) ?? [],
      }))
      .filter((group) => group.items.length > 0);
  }, [categories, libraries]);

  return (
    <nav className="nav-tree" aria-label="ライブラリ一覧">
      <NavLink
        to="/"
        end
        className="nav-tree__home"
        onClick={onNavigate}
      >
        <span>すべてのライブラリ</span>
        <span className="nav-group__count">{libraries.length}</span>
      </NavLink>

      {grouped.map(({ category, items }) => (
        <section key={category.id} aria-labelledby={`nav-cat-${category.id}`}>
          <h2 id={`nav-cat-${category.id}`} className="nav-group__title">
            <span>{category.label}</span>
            <span className="nav-group__count">{items.length}</span>
          </h2>
          <ul className="nav-group__list">
            {items.map((lib) => (
              <li key={lib.id}>
                <NavLink
                  to={`/lib/${lib.id}`}
                  className="nav-link"
                  onClick={onNavigate}
                >
                  <span>{lib.title}</span>
                  <span className="nav-link__file">{lib.file}</span>
                </NavLink>
              </li>
            ))}
          </ul>
        </section>
      ))}
    </nav>
  );
}

export function Sidebar({
  siteTitle,
  categories,
  libraries,
  mobileOpen,
  onCloseMobile,
}: SidebarProps) {
  const dialogRef = useRef<HTMLDialogElement>(null);
  const scrollerRef = useRef<HTMLDivElement>(null);
  const sheetRef = useRef<HTMLDivElement>(null);
  const phaseRef = useRef<'closed' | 'opening' | 'open' | 'closing'>('closed');

  const finishClose = useCallback(() => {
    const dialog = dialogRef.current;
    phaseRef.current = 'closed';
    if (dialog?.open) {
      dialog.close();
      dialog.returnValue = '';
    }
    onCloseMobile();
  }, [onCloseMobile]);

  const requestCloseDrawer = useCallback(() => {
    const dialog = dialogRef.current;
    const scroller = scrollerRef.current;
    if (!dialog?.open || !scroller) {
      finishClose();
      return;
    }
    phaseRef.current = 'closing';
    if (scroller.offsetWidth === 0 || scroller.scrollLeft >= scroller.offsetWidth - 2) {
      finishClose();
      return;
    }
    scroller.scrollTo({ left: scroller.offsetWidth, behavior: 'auto' });
  }, [finishClose]);

  useEffect(() => {
    const dialog = dialogRef.current;
    const scroller = scrollerRef.current;
    const sheet = sheetRef.current;
    if (!dialog || !scroller || !sheet) return;

    const supportsScrollTimeline =
      typeof CSS !== 'undefined' &&
      typeof CSS.supports === 'function' &&
      CSS.supports('animation-timeline: scroll()');

    const onScroll = () => {
      const width = sheet.offsetWidth || 1;
      const ratio = Math.max(0, Math.min(1, 1 - scroller.scrollLeft / width));
      dialog.style.setProperty('--drawer-backdrop', String(ratio));
    };

    if (!supportsScrollTimeline) {
      scroller.addEventListener('scroll', onScroll, { passive: true });
    }

    const visibleThreshold = Math.max(0.001, 1 / (window.innerWidth || 1024));
    const observer = new IntersectionObserver(
      (entries) => {
        const entry = entries[entries.length - 1];
        if (!entry) return;
        if (entry.intersectionRatio >= 0.98) {
          phaseRef.current = 'open';
        } else if (
          entry.intersectionRatio < visibleThreshold &&
          (phaseRef.current === 'closing' || phaseRef.current === 'open')
        ) {
          finishClose();
        }
      },
      { root: dialog, threshold: [visibleThreshold, 0.98] },
    );
    observer.observe(sheet);

    const onCancel = (event: Event) => {
      if (event.cancelable) {
        event.preventDefault();
        requestCloseDrawer();
      }
    };

    const onNativeClose = () => {
      if (phaseRef.current !== 'closed') {
        phaseRef.current = 'closed';
        dialog.returnValue = '';
        onCloseMobile();
      }
    };

    dialog.addEventListener('cancel', onCancel);
    dialog.addEventListener('close', onNativeClose);

    return () => {
      observer.disconnect();
      if (!supportsScrollTimeline) {
        scroller.removeEventListener('scroll', onScroll);
      }
      dialog.removeEventListener('cancel', onCancel);
      dialog.removeEventListener('close', onNativeClose);
    };
  }, [finishClose, onCloseMobile, requestCloseDrawer]);

  useEffect(() => {
    const dialog = dialogRef.current;
    const scroller = scrollerRef.current;
    if (!dialog || !scroller) return;

    let cancelled = false;

    const openDrawer = async () => {
      if (dialog.open) return;
      phaseRef.current = 'opening';
      dialog.showModal();

      const supportsInitialTarget =
        typeof CSS !== 'undefined' &&
        typeof CSS.supports === 'function' &&
        CSS.supports('scroll-initial-target', 'nearest');

      if (!supportsInitialTarget) {
        scroller.scrollTo({ left: scroller.offsetWidth, behavior: 'instant' });
        await new Promise<void>((resolve) => {
          requestAnimationFrame(() => requestAnimationFrame(() => resolve()));
        });
      }

      if (!cancelled && dialog.open) {
        scroller.scrollTo({ left: 0, behavior: 'auto' });
      }
    };

    if (mobileOpen) {
      void openDrawer();
    } else if (dialog.open && phaseRef.current !== 'closing') {
      requestCloseDrawer();
    }

    return () => {
      cancelled = true;
    };
  }, [mobileOpen, requestCloseDrawer]);

  return (
    <>
      <aside className="sidebar-desktop" aria-label="サイドバーナビゲーション">
        <NavTree categories={categories} libraries={libraries} />
      </aside>

      <dialog
        ref={dialogRef}
        id="mobile-nav-drawer"
        className="nav-drawer"
        aria-label="ナビゲーションメニュー"
        onClick={(event) => {
          if (sheetRef.current && !sheetRef.current.contains(event.target as Node)) {
            requestCloseDrawer();
          }
        }}
      >
        <div ref={scrollerRef} className="nav-drawer__scroller">
          <div ref={sheetRef} className="nav-drawer__sheet">
            <div className="nav-drawer__head">
              <span className="nav-drawer__title">{siteTitle}</span>
              <button
                type="button"
                className="icon-btn"
                aria-label="メニューを閉じる"
                autoFocus
                onClick={requestCloseDrawer}
              >
                <X size={17} aria-hidden="true" />
              </button>
            </div>
            <div className="nav-drawer__body">
              <NavTree
                categories={categories}
                libraries={libraries}
                onNavigate={requestCloseDrawer}
              />
            </div>
          </div>
        </div>
      </dialog>
    </>
  );
}
