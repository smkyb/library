import fs from 'node:fs';
import path from 'node:path';
import type { Plugin, ResolvedConfig } from 'vite';
import { createHighlighter, type Highlighter } from 'shiki';
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import remarkGfm from 'remark-gfm';
import remarkMath from 'remark-math';
import remarkRehype from 'remark-rehype';
import rehypeSlug from 'rehype-slug';
import rehypeKatex from 'rehype-katex';
import rehypeStringify from 'rehype-stringify';
import { toString as mdastToString } from 'mdast-util-to-string';
import { toString as hastToString } from 'hast-util-to-string';
import { visit } from 'unist-util-visit';
import { extractLeadingComments, stripCppComments } from '../src/utils/stripCppComments.ts';
import type { Category, LibraryItem, SiteConfig, TocItem, VerifiedLink } from '../src/types.ts';

const VIRTUAL_MODULE_ID = 'virtual:libraries';
const RESOLVED_VIRTUAL_ID = `\0${VIRTUAL_MODULE_ID}`;

const SHIKI_THEMES = {
  light: 'github-light',
  dark: 'github-dark-default',
} as const;

const SHIKI_LANGS = ['cpp', 'c', 'bash', 'shellscript', 'powershell', 'json', 'yaml', 'markdown', 'text'] as const;

interface RawCatalogEntry {
  id?: unknown;
  file?: unknown;
  category?: unknown;
  title?: unknown;
  summary?: unknown;
  complexity?: unknown;
  tags?: unknown;
  published?: unknown;
  verified?: unknown;
}

interface RawCatalog {
  site?: Partial<SiteConfig>;
  categories?: unknown;
  libraries?: unknown;
}

function assertNonEmptyString(val: unknown, field: string): string {
  if (typeof val !== 'string' || val.trim() === '') {
    throw new Error(`[catalog.json] "${field}" は空でない文字列である必要があります。`);
  }
  return val.trim();
}

function parseVerifiedLinks(val: unknown, entryId: string): VerifiedLink[] {
  if (val === undefined) return [];
  if (!Array.isArray(val)) {
    throw new Error(`[catalog.json] "${entryId}.verified" は配列である必要があります。`);
  }
  return val.map((item, idx) => {
    if (!item || typeof item !== 'object') {
      throw new Error(`[catalog.json] "${entryId}.verified[${idx}]" が不正です。`);
    }
    const obj = item as Record<string, unknown>;
    return {
      label: assertNonEmptyString(obj.label, `${entryId}.verified[${idx}].label`),
      url: assertNonEmptyString(obj.url, `${entryId}.verified[${idx}].url`),
    };
  });
}

function parseTags(val: unknown, entryId: string): string[] {
  if (val === undefined) return [];
  if (!Array.isArray(val) || val.some((t) => typeof t !== 'string')) {
    throw new Error(`[catalog.json] "${entryId}.tags" は文字列の配列である必要があります。`);
  }
  return val.map((t) => t.trim()).filter(Boolean);
}

function escapeHtml(text: string): string {
  return text
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;')
    .replaceAll('"', '&quot;');
}

function resolveSafePath(baseDir: string, relPath: string): string {
  const normalizedRel = relPath.replace(/\\/g, '/');
  const resolvedBase = path.resolve(baseDir);
  const resolvedTarget = path.resolve(resolvedBase, ...normalizedRel.split('/'));
  if (!resolvedTarget.startsWith(resolvedBase + path.sep)) {
    throw new Error(`不正な相対パスが指定されました: ${relPath}`);
  }
  return resolvedTarget;
}

function resolveShikiLang(rawLang: string, highlighter: Highlighter): string {
  const normalized = rawLang.toLowerCase();
  const aliases: Record<string, string> = {
    'c++': 'cpp',
    cc: 'cpp',
    hpp: 'cpp',
    sh: 'bash',
    shell: 'bash',
    ps1: 'powershell',
    pwsh: 'powershell',
    yml: 'yaml',
    md: 'markdown',
    plaintext: 'text',
    txt: 'text',
  };
  const candidate = aliases[normalized] ?? normalized;
  const loaded = highlighter.getLoadedLanguages();
  return loaded.includes(candidate) ? candidate : 'text';
}

interface HastNode {
  type: string;
  tagName?: string;
  properties?: Record<string, unknown>;
  children?: HastNode[];
}

async function compileMarkdown(
  markdownSource: string,
  highlighter: Highlighter,
): Promise<{ html: string; toc: TocItem[]; plainText: string }> {
  const toc: TocItem[] = [];

  const mdProcessor = unified().use(remarkParse).use(remarkGfm).use(remarkMath);
  const mdTree = mdProcessor.parse(markdownSource);
  const plainText = mdastToString(mdTree).replace(/\s+/g, ' ').trim();

  const rehypeCollectAndHighlight = () => (tree: HastNode) => {
    visit(tree as never, 'element', (node: HastNode, index: number | undefined, parent: HastNode | undefined) => {
      if (node.tagName === 'h2' || node.tagName === 'h3') {
        const id = typeof node.properties?.id === 'string' ? node.properties.id : '';
        const text = hastToString(node as never).trim();
        if (id && text) {
          toc.push({
            id,
            text,
            level: node.tagName === 'h2' ? 2 : 3,
          });
        }
        return;
      }

      if (
        node.tagName === 'pre' &&
        parent &&
        typeof index === 'number' &&
        Array.isArray(node.children) &&
        node.children.length === 1
      ) {
        const codeChild = node.children[0];
        if (codeChild?.type === 'element' && codeChild.tagName === 'code') {
          const classNames = Array.isArray(codeChild.properties?.className)
            ? codeChild.properties.className.map(String)
            : [];
          const langClass = classNames.find((c) => c.startsWith('language-'));
          const rawLang = langClass ? langClass.slice('language-'.length) : 'text';
          const lang = resolveShikiLang(rawLang, highlighter);
          const codeText = hastToString(codeChild as never).replace(/\n$/, '');

          const highlightedRoot = highlighter.codeToHast(codeText, {
            lang,
            themes: SHIKI_THEMES,
            defaultColor: false,
          }) as unknown as HastNode;

          const preEl = highlightedRoot.children?.[0];
          if (preEl && Array.isArray(parent.children)) {
            parent.children[index] = preEl;
          }
        }
      }
    });
  };

  const file = await unified()
    .use(remarkParse)
    .use(remarkGfm)
    .use(remarkMath)
    .use(remarkRehype)
    .use(rehypeSlug)
    .use(rehypeKatex)
    .use(rehypeCollectAndHighlight)
    .use(rehypeStringify)
    .process(markdownSource);

  return {
    html: String(file),
    toc,
    plainText,
  };
}

export function libraryContentPlugin(): Plugin {
  let rootDir = process.cwd();
  let resolvedConfig: ResolvedConfig | null = null;
  let highlighterPromise: Promise<Highlighter> | null = null;
  let cachedPayload: { site: SiteConfig; categories: Category[]; libraries: LibraryItem[] } | null = null;

  const getHighlighter = () => {
    if (!highlighterPromise) {
      highlighterPromise = createHighlighter({
        themes: [SHIKI_THEMES.light, SHIKI_THEMES.dark],
        langs: [...SHIKI_LANGS],
      });
    }
    return highlighterPromise;
  };

  const buildCatalogPayload = async () => {
    const catalogPath = path.join(rootDir, 'content', 'catalog.json');
    if (!fs.existsSync(catalogPath)) {
      throw new Error(`カタログファイルが見つかりません: ${catalogPath}`);
    }

    const rawCatalog = JSON.parse(fs.readFileSync(catalogPath, 'utf8')) as RawCatalog;
    const site: SiteConfig = {
      title: assertNonEmptyString(rawCatalog.site?.title, 'site.title'),
      description: assertNonEmptyString(rawCatalog.site?.description, 'site.description'),
      githubRepo: assertNonEmptyString(rawCatalog.site?.githubRepo, 'site.githubRepo').replace(/\/+$/, ''),
      defaultBranch: assertNonEmptyString(rawCatalog.site?.defaultBranch, 'site.defaultBranch'),
    };

    if (!Array.isArray(rawCatalog.categories)) {
      throw new Error('[catalog.json] "categories" は配列である必要があります。');
    }
    const categories: Category[] = [];
    const categoryMap = new Map<string, string>();
    for (let i = 0; i < rawCatalog.categories.length; i += 1) {
      const cat = rawCatalog.categories[i] as Record<string, unknown>;
      const id = assertNonEmptyString(cat?.id, `categories[${i}].id`);
      const label = assertNonEmptyString(cat?.label, `categories[${i}].label`);
      if (categoryMap.has(id)) {
        throw new Error(`[catalog.json] カテゴリ ID が重複しています: ${id}`);
      }
      categoryMap.set(id, label);
      categories.push({ id, label });
    }

    if (!Array.isArray(rawCatalog.libraries)) {
      throw new Error('[catalog.json] "libraries" は配列である必要があります。');
    }

    const highlighter = await getHighlighter();
    const librariesDir = path.join(rootDir, 'libraries');
    const docsDir = path.join(rootDir, 'content', 'docs');
    const seenIds = new Set<string>();
    const libraries: LibraryItem[] = [];

    for (let i = 0; i < rawCatalog.libraries.length; i += 1) {
      const entry = rawCatalog.libraries[i] as RawCatalogEntry;
      const id = assertNonEmptyString(entry?.id, `libraries[${i}].id`);
      if (seenIds.has(id)) {
        throw new Error(`[catalog.json] ライブラリ ID が重複しています: ${id}`);
      }
      seenIds.add(id);

      const file = assertNonEmptyString(entry?.file, `libraries[${i}].file`).replace(/\\/g, '/');
      const category = assertNonEmptyString(entry?.category, `libraries[${i}].category`);
      const categoryLabel = categoryMap.get(category);
      if (!categoryLabel) {
        throw new Error(`[catalog.json] "${id}" のカテゴリ "${category}" は categories に未定義です。`);
      }

      if (typeof entry?.published !== 'boolean') {
        throw new Error(`[catalog.json] "${id}.published" は boolean である必要があります。`);
      }
      if (!entry.published) {
        continue;
      }

      const sourcePath = resolveSafePath(librariesDir, file);
      if (!fs.existsSync(sourcePath)) {
        throw new Error(
          `公開設定されたライブラリファイルが見つかりません: ${sourcePath} ("npm run sync" を実行してください)`,
        );
      }

      const rawFileContent = fs.readFileSync(sourcePath, 'utf8');
      const rawCode = rawFileContent.replace(/\r\n/g, '\n').replace(/\n+$/, '');
      const lineCount = rawCode === '' ? 0 : rawCode.split('\n').length;
      const strippedCode = stripCppComments(rawCode);
      const leadingNotes = extractLeadingComments(rawCode);

      const highlightedCodeHtml = highlighter.codeToHtml(rawCode, {
        lang: 'cpp',
        themes: SHIKI_THEMES,
        defaultColor: false,
      });

      const docPath = resolveSafePath(docsDir, `${id}.md`);
      const hasDoc = fs.existsSync(docPath);
      let docHtml = '';
      let docPlainText = '';
      const toc: TocItem[] = [];

      if (hasDoc) {
        const mdSource = fs.readFileSync(docPath, 'utf8').replace(/\r\n/g, '\n');
        const compiled = await compileMarkdown(mdSource, highlighter);
        docHtml = compiled.html;
        docPlainText = compiled.plainText;
        toc.push(...compiled.toc);
      }

      toc.push({ id: 'source-code', text: 'ソースコード', level: 2 });

      const title =
        typeof entry.title === 'string' && entry.title.trim() !== '' ? entry.title.trim() : id;
      const summary =
        typeof entry.summary === 'string' && entry.summary.trim() !== ''
          ? entry.summary.trim()
          : (leadingNotes[0] ?? '');
      const complexity =
        typeof entry.complexity === 'string' && entry.complexity.trim() !== ''
          ? entry.complexity.trim()
          : undefined;
      const tags = parseTags(entry.tags, id);
      const verified = parseVerifiedLinks(entry.verified, id);
      const githubUrl = `${site.githubRepo}/blob/${site.defaultBranch}/libraries/${file}`;

      const searchText = [
        id,
        file,
        title,
        categoryLabel,
        summary,
        complexity ?? '',
        ...tags,
        ...leadingNotes,
        docPlainText,
      ]
        .join(' ')
        .replace(/\s+/g, ' ')
        .trim()
        .toLowerCase();

      libraries.push({
        id,
        file,
        category,
        categoryLabel,
        title,
        summary,
        complexity,
        tags,
        verified,
        lineCount,
        rawCode,
        strippedCode,
        highlightedCodeHtml,
        leadingNotes,
        hasDoc,
        docHtml,
        toc,
        githubUrl,
        searchText,
      });
    }

    cachedPayload = { site, categories, libraries };
    return cachedPayload;
  };

  return {
    name: 'library-content-plugin',

    configResolved(config) {
      rootDir = config.root;
      resolvedConfig = config;
    },

    resolveId(id) {
      if (id === VIRTUAL_MODULE_ID) {
        return RESOLVED_VIRTUAL_ID;
      }
      return null;
    },

    async load(id) {
      if (id !== RESOLVED_VIRTUAL_ID) {
        return null;
      }
      const payload = await buildCatalogPayload();
      return [
        `export const site = ${JSON.stringify(payload.site)};`,
        `export const categories = ${JSON.stringify(payload.categories)};`,
        `export const libraries = ${JSON.stringify(payload.libraries)};`,
      ].join('\n');
    },

    configureServer(server) {
      const contentDir = path.resolve(rootDir, 'content').toLowerCase();
      const librariesDir = path.resolve(rootDir, 'libraries').toLowerCase();

      server.watcher.add([path.resolve(rootDir, 'content'), path.resolve(rootDir, 'libraries')]);

      const onFsEvent = (changedFile: string) => {
        const normalized = path.resolve(changedFile).toLowerCase();
        if (!normalized.startsWith(contentDir) && !normalized.startsWith(librariesDir)) {
          return;
        }
        const mod = server.moduleGraph.getModuleById(RESOLVED_VIRTUAL_ID);
        if (mod) {
          server.moduleGraph.invalidateModule(mod);
        }
        server.ws.send({ type: 'full-reload' });
      };

      server.watcher.on('add', onFsEvent);
      server.watcher.on('change', onFsEvent);
      server.watcher.on('unlink', onFsEvent);
    },

    async closeBundle() {
      if (!resolvedConfig || resolvedConfig.command !== 'build') {
        return;
      }
      const outDir = path.resolve(resolvedConfig.root, resolvedConfig.build.outDir);
      const indexHtmlPath = path.join(outDir, 'index.html');
      if (!fs.existsSync(indexHtmlPath)) {
        return;
      }

      const payload = cachedPayload ?? (await buildCatalogPayload());
      const baseHtml = fs.readFileSync(indexHtmlPath, 'utf8');

      // GitHub Pages SPA fallback for unknown routes
      fs.writeFileSync(path.join(outDir, '404.html'), baseHtml, 'utf8');

      // Pre-create entry HTML per published library so direct navigation returns HTTP 200
      for (const lib of payload.libraries) {
        const pageTitle = `${escapeHtml(lib.title)} | ${escapeHtml(payload.site.title)}`;
        const pageDescription = escapeHtml(lib.summary || payload.site.description);
        let pageHtml = baseHtml.replace(/<title>[\s\S]*?<\/title>/, `<title>${pageTitle}</title>`);
        if (/<meta\s+name="description"\s+content="[^"]*"\s*\/?>/i.test(pageHtml)) {
          pageHtml = pageHtml.replace(
            /<meta\s+name="description"\s+content="[^"]*"\s*\/?>/i,
            `<meta name="description" content="${pageDescription}">`,
          );
        } else {
          pageHtml = pageHtml.replace(
            '</head>',
            `  <meta name="description" content="${pageDescription}">\n  </head>`,
          );
        }

        const routeDir = path.join(outDir, 'lib', lib.id);
        fs.mkdirSync(routeDir, { recursive: true });
        fs.writeFileSync(path.join(routeDir, 'index.html'), pageHtml, 'utf8');
      }
    },
  };
}
