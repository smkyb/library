import { useCallback, useEffect, useState } from 'react';

export type ResolvedTheme = 'light' | 'dark';

const STORAGE_KEY = 'color-scheme';
const DARK_QUERY = '(prefers-color-scheme: dark)';

function readStoredScheme(): ResolvedTheme | null {
  try {
    const val = localStorage.getItem(STORAGE_KEY);
    return val === 'light' || val === 'dark' ? val : null;
  } catch {
    return null;
  }
}

function writeStoredScheme(value: ResolvedTheme | null): void {
  try {
    if (value === null) {
      localStorage.removeItem(STORAGE_KEY);
    } else {
      localStorage.setItem(STORAGE_KEY, value);
    }
  } catch {
    // Storage access can be blocked in strict privacy contexts; still update <meta>
  }
}

function getSystemScheme(): ResolvedTheme {
  if (typeof window === 'undefined' || typeof window.matchMedia !== 'function') {
    return 'light';
  }
  return window.matchMedia(DARK_QUERY).matches ? 'dark' : 'light';
}

function applyMetaColorScheme(value: ResolvedTheme | null): void {
  const meta = document.querySelector<HTMLMetaElement>('meta[name="color-scheme"]');
  if (meta) {
    meta.content = value ?? 'light dark';
  }
}

export function useTheme() {
  const [override, setOverride] = useState<ResolvedTheme | null>(() => readStoredScheme());
  const [systemScheme, setSystemScheme] = useState<ResolvedTheme>(() => getSystemScheme());

  const resolvedTheme: ResolvedTheme = override ?? systemScheme;

  useEffect(() => {
    applyMetaColorScheme(override);
  }, [override]);

  useEffect(() => {
    if (typeof window.matchMedia !== 'function') {
      return;
    }
    const media = window.matchMedia(DARK_QUERY);
    const onMediaChange = (event: MediaQueryListEvent) => {
      // Do NOT clear override when system preference changes to match it
      setSystemScheme(event.matches ? 'dark' : 'light');
    };
    media.addEventListener('change', onMediaChange);
    return () => media.removeEventListener('change', onMediaChange);
  }, []);

  useEffect(() => {
    const onStorage = (event: StorageEvent) => {
      if (event.key === STORAGE_KEY) {
        setOverride(event.newValue === 'light' || event.newValue === 'dark' ? event.newValue : null);
      }
    };
    window.addEventListener('storage', onStorage);
    return () => window.removeEventListener('storage', onStorage);
  }, []);

  const toggleTheme = useCallback(() => {
    const target: ResolvedTheme = resolvedTheme === 'dark' ? 'light' : 'dark';
    const nextOverride = target === systemScheme ? null : target;
    writeStoredScheme(nextOverride);
    applyMetaColorScheme(nextOverride);
    setOverride(nextOverride);
  }, [resolvedTheme, systemScheme]);

  return {
    resolvedTheme,
    toggleTheme,
  };
}
