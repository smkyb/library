import { useEffect, useState } from 'react';
import type { TocItem } from '../types.ts';

interface TableOfContentsProps {
  items: TocItem[];
}

export function TableOfContents({ items }: TableOfContentsProps) {
  const [activeId, setActiveId] = useState<string>(() => items[0]?.id ?? '');

  useEffect(() => {
    setActiveId(items[0]?.id ?? '');
    if (items.length === 0) return;

    const elements = items
      .map((item) => document.getElementById(item.id))
      .filter((el): el is HTMLElement => el !== null);

    if (elements.length === 0) return;

    const observer = new IntersectionObserver(
      (entries) => {
        const visible = entries
          .filter((entry) => entry.isIntersecting)
          .sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);

        if (visible.length > 0 && visible[0]?.target.id) {
          setActiveId(visible[0].target.id);
        }
      },
      { rootMargin: '-72px 0px -65% 0px', threshold: 0 },
    );

    for (const el of elements) {
      observer.observe(el);
    }

    return () => observer.disconnect();
  }, [items]);

  if (items.length === 0) {
    return null;
  }

  return (
    <aside className="toc-aside" aria-label="目次">
      <nav aria-label="このページの内容">
        <h2 className="toc-nav__title">目次</h2>
        <ul className="toc-nav__list">
          {items.map((item) => (
            <li key={item.id}>
              <a
                href={`#${item.id}`}
                className={
                  item.level === 3
                    ? 'toc-nav__link toc-nav__link--sub'
                    : 'toc-nav__link'
                }
                aria-current={activeId === item.id ? 'location' : undefined}
              >
                {item.text}
              </a>
            </li>
          ))}
        </ul>
      </nav>
    </aside>
  );
}
