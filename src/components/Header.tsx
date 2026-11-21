import { Link } from 'react-router-dom';
import { GitBranch, Menu, Moon, Search, Sun } from 'lucide-react';
import type { SiteConfig } from '../types.ts';
import type { ResolvedTheme } from '../hooks/useTheme.ts';

interface HeaderProps {
  site: SiteConfig;
  resolvedTheme: ResolvedTheme;
  onToggleTheme: () => void;
  onOpenSearch: () => void;
  mobileNavOpen: boolean;
  onOpenMobileNav: () => void;
}

export function Header({
  site,
  resolvedTheme,
  onToggleTheme,
  onOpenSearch,
  mobileNavOpen,
  onOpenMobileNav,
}: HeaderProps) {
  const nextThemeLabel =
    resolvedTheme === 'dark' ? 'ライトテーマに切り替え' : 'ダークテーマに切り替え';

  return (
    <header className="site-header">
      <div className="site-header__inner">
        <div className="site-header__left">
          <button
            type="button"
            className="icon-btn mobile-menu-btn"
            aria-label="メニューを開く"
            aria-controls="mobile-nav-drawer"
            aria-expanded={mobileNavOpen}
            onClick={onOpenMobileNav}
          >
            <Menu size={18} aria-hidden="true" />
          </button>

          <Link to="/" className="site-brand">
            <span className="site-brand__mark" aria-hidden="true">
              {'</>'}
            </span>
            <span>{site.title}</span>
          </Link>
        </div>

        <div className="site-header__actions">
          <button
            type="button"
            className="search-trigger"
            onClick={onOpenSearch}
            aria-label="ライブラリを検索 (Ctrl+K)"
          >
            <span className="search-trigger__left">
              <Search size={15} aria-hidden="true" />
              <span className="search-trigger__label">検索...</span>
            </span>
            <span className="search-trigger__kbd" aria-hidden="true">
              <kbd>Ctrl K</kbd>
            </span>
          </button>

          <a
            href={site.githubRepo}
            target="_blank"
            rel="noopener noreferrer"
            className="icon-btn"
            aria-label="GitHub リポジトリを開く"
            title="GitHub リポジトリ"
          >
            <GitBranch size={17} aria-hidden="true" />
          </a>

          <button
            type="button"
            className="icon-btn"
            onClick={onToggleTheme}
            aria-label={nextThemeLabel}
            title={nextThemeLabel}
          >
            {resolvedTheme === 'dark' ? (
              <Sun size={17} aria-hidden="true" />
            ) : (
              <Moon size={17} aria-hidden="true" />
            )}
          </button>
        </div>
      </div>
    </header>
  );
}
