import { useMemo, useState } from 'react';
import { Link } from 'react-router-dom';
import type { Category, LibraryItem, SiteConfig } from '../types.ts';

interface HomePageProps {
  site: SiteConfig;
  categories: Category[];
  libraries: LibraryItem[];
}

export function HomePage({ site, categories, libraries }: HomePageProps) {
  const [selectedCategory, setSelectedCategory] = useState<string>('all');

  const categoryCounts = useMemo(() => {
    const counts = new Map<string, number>();
    for (const lib of libraries) {
      counts.set(lib.category, (counts.get(lib.category) ?? 0) + 1);
    }
    return categories
      .map((cat) => ({
        category: cat,
        count: counts.get(cat.id) ?? 0,
      }))
      .filter((item) => item.count > 0);
  }, [categories, libraries]);

  const filtered = useMemo(() => {
    if (selectedCategory === 'all') {
      return libraries;
    }
    return libraries.filter((lib) => lib.category === selectedCategory);
  }, [libraries, selectedCategory]);

  return (
    <div>
      <header className="home-header">
        <h1 className="home-title">{site.title}</h1>
        <p className="home-description">{site.description}</p>
      </header>

      <div className="filter-bar" role="group" aria-label="カテゴリ絞り込み">
        <button
          type="button"
          className="filter-chip"
          aria-pressed={selectedCategory === 'all'}
          onClick={() => setSelectedCategory('all')}
        >
          <span>すべて</span>
          <span className="filter-chip__count">{libraries.length}</span>
        </button>

        {categoryCounts.map(({ category, count }) => (
          <button
            key={category.id}
            type="button"
            className="filter-chip"
            aria-pressed={selectedCategory === category.id}
            onClick={() => setSelectedCategory(category.id)}
          >
            <span>{category.label}</span>
            <span className="filter-chip__count">{count}</span>
          </button>
        ))}
      </div>

      <section aria-label="公開ライブラリ一覧">
        <ul className="library-index">
          {filtered.map((lib) => (
            <li key={lib.id}>
              <Link to={`/lib/${lib.id}`} className="library-row">
                <span className="library-row__title">{lib.title}</span>
                <span className="library-row__category">{lib.categoryLabel}</span>
              </Link>
            </li>
          ))}
        </ul>
      </section>
    </div>
  );
}
