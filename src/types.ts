export interface SiteConfig {
  title: string;
  description: string;
  githubRepo: string;
  defaultBranch: string;
}

export interface Category {
  id: string;
  label: string;
}

export interface TocItem {
  id: string;
  text: string;
  level: 2 | 3;
}

export interface VerifiedLink {
  label: string;
  url: string;
}

export interface LibraryItem {
  id: string;
  file: string;
  category: string;
  categoryLabel: string;
  title: string;
  summary: string;
  complexity?: string;
  tags: string[];
  verified: VerifiedLink[];
  lineCount: number;
  rawCode: string;
  strippedCode: string;
  highlightedCodeHtml: string;
  leadingNotes: string[];
  hasDoc: boolean;
  docHtml: string;
  toc: TocItem[];
  githubUrl: string;
  searchText: string;
}

export interface LibraryCatalogData {
  site: SiteConfig;
  categories: Category[];
  libraries: LibraryItem[];
}
