declare module 'virtual:libraries' {
  export const site: import('./types.ts').SiteConfig;
  export const categories: import('./types.ts').Category[];
  export const libraries: import('./types.ts').LibraryItem[];
}
