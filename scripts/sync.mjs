import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const DEFAULT_SRC_DIR = 'C:\\vscode programs\\Libraries';
const ALLOWED_EXTENSIONS = new Set(['.cpp', '.hpp']);

function parseArgs(argv) {
  let srcDir = process.env.CP_LIBRARIES_SRC || DEFAULT_SRC_DIR;
  let dryRun = false;

  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === '--dry-run') {
      dryRun = true;
    } else if (arg === '--src') {
      const next = argv[i + 1];
      if (!next) {
        throw new Error('--src にはディレクトリパスを指定してください。');
      }
      srcDir = next;
      i += 1;
    } else if (arg === '--help' || arg === '-h') {
      console.log('Usage: node scripts/sync.mjs [--src <dir>] [--dry-run]');
      process.exit(0);
    } else {
      throw new Error(`不明な引数です: ${arg}`);
    }
  }

  return { srcDir: path.resolve(srcDir), dryRun };
}

function shouldSkipEntry(name) {
  return name.startsWith('.') || name.startsWith('0_');
}

function collectSourceFiles(rootDir, currentDir = rootDir, result = new Map()) {
  const entries = fs.readdirSync(currentDir, { withFileTypes: true });
  entries.sort((a, b) => a.name.localeCompare(b.name));

  for (const entry of entries) {
    if (shouldSkipEntry(entry.name)) {
      continue;
    }
    const fullPath = path.join(currentDir, entry.name);
    if (entry.isDirectory()) {
      collectSourceFiles(rootDir, fullPath, result);
      continue;
    }
    if (!entry.isFile()) {
      continue;
    }
    const ext = path.extname(entry.name).toLowerCase();
    if (!ALLOWED_EXTENSIONS.has(ext)) {
      continue;
    }
    const relPath = path.relative(rootDir, fullPath).split(path.sep).join('/');
    result.set(relPath, fullPath);
  }
  return result;
}

function collectExistingFiles(rootDir, currentDir = rootDir, result = new Map()) {
  if (!fs.existsSync(currentDir)) {
    return result;
  }
  const entries = fs.readdirSync(currentDir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(currentDir, entry.name);
    if (entry.isDirectory()) {
      collectExistingFiles(rootDir, fullPath, result);
    } else if (entry.isFile()) {
      const relPath = path.relative(rootDir, fullPath).split(path.sep).join('/');
      result.set(relPath, fullPath);
    }
  }
  return result;
}

function pruneEmptyDirs(rootDir, currentDir = rootDir) {
  if (!fs.existsSync(currentDir)) {
    return;
  }
  const entries = fs.readdirSync(currentDir, { withFileTypes: true });
  for (const entry of entries) {
    if (entry.isDirectory()) {
      pruneEmptyDirs(rootDir, path.join(currentDir, entry.name));
    }
  }
  if (currentDir !== rootDir) {
    const remaining = fs.readdirSync(currentDir);
    if (remaining.length === 0) {
      fs.rmdirSync(currentDir);
    }
  }
}

function validateCatalog(repoRoot, sourceFiles) {
  const catalogPath = path.join(repoRoot, 'content', 'catalog.json');
  if (!fs.existsSync(catalogPath)) {
    return null;
  }
  const catalog = JSON.parse(fs.readFileSync(catalogPath, 'utf8'));
  const entries = Array.isArray(catalog.libraries) ? catalog.libraries : [];
  const missingPublished = [];
  const registeredFiles = new Set();

  for (const entry of entries) {
    const normalizedFile = String(entry.file ?? '').replace(/\\/g, '/');
    registeredFiles.add(normalizedFile);
    if (entry.published === true && !sourceFiles.has(normalizedFile)) {
      missingPublished.push(`${entry.id} (${normalizedFile})`);
    }
  }

  if (missingPublished.length > 0) {
    throw new Error(
      `catalog.json で公開指定されているファイルがコピー元に存在しません: ${missingPublished.join(', ')}`,
    );
  }

  let publishedCount = 0;
  for (const entry of entries) {
    if (entry.published === true) {
      publishedCount += 1;
    }
  }

  let unregisteredCount = 0;
  for (const relPath of sourceFiles.keys()) {
    if (!registeredFiles.has(relPath)) {
      unregisteredCount += 1;
    }
  }

  return {
    publishedCount,
    totalCatalogEntries: entries.length,
    unregisteredCount,
  };
}

function main() {
  const { srcDir, dryRun } = parseArgs(process.argv.slice(2));
  const scriptDir = path.dirname(fileURLToPath(import.meta.url));
  const repoRoot = path.resolve(scriptDir, '..');
  const destDir = path.join(repoRoot, 'libraries');

  if (!fs.existsSync(srcDir) || !fs.statSync(srcDir).isDirectory()) {
    throw new Error(`コピー元ディレクトリが見つかりません: ${srcDir}`);
  }

  const sourceFiles = collectSourceFiles(srcDir);
  if (sourceFiles.size === 0) {
    throw new Error(`コピー元ディレクトリに同期対象の .cpp/.hpp がありません: ${srcDir}`);
  }

  const existingFiles = collectExistingFiles(destDir);
  const added = [];
  const updated = [];
  const deleted = [];
  let unchanged = 0;

  for (const [relPath, srcPath] of sourceFiles) {
    const dstPath = path.join(destDir, ...relPath.split('/'));
    const srcBuf = fs.readFileSync(srcPath);

    if (!existingFiles.has(relPath)) {
      added.push(relPath);
      if (!dryRun) {
        fs.mkdirSync(path.dirname(dstPath), { recursive: true });
        fs.writeFileSync(dstPath, srcBuf);
      }
      continue;
    }

    const dstBuf = fs.readFileSync(dstPath);
    if (!srcBuf.equals(dstBuf)) {
      updated.push(relPath);
      if (!dryRun) {
        fs.writeFileSync(dstPath, srcBuf);
      }
    } else {
      unchanged += 1;
    }
  }

  for (const [relPath, dstPath] of existingFiles) {
    if (!sourceFiles.has(relPath)) {
      deleted.push(relPath);
      if (!dryRun) {
        fs.unlinkSync(dstPath);
      }
    }
  }

  if (!dryRun) {
    fs.mkdirSync(destDir, { recursive: true });
    pruneEmptyDirs(destDir);
  }

  const catalogStats = validateCatalog(repoRoot, sourceFiles);

  console.log(`${dryRun ? '[DRY-RUN] ' : ''}Sync: ${srcDir} -> ${destDir}`);
  console.log(
    `Result: +${added.length} added, ~${updated.length} updated, -${deleted.length} deleted, ${unchanged} unchanged (total ${sourceFiles.size} files)`,
  );

  for (const f of added) console.log(`  + ${f}`);
  for (const f of updated) console.log(`  ~ ${f}`);
  for (const f of deleted) console.log(`  - ${f}`);

  if (catalogStats) {
    console.log(
      `Catalog: ${catalogStats.publishedCount} published / ${catalogStats.totalCatalogEntries} registered (${catalogStats.unregisteredCount} synced files unpublished)`,
    );
  }
}

try {
  main();
} catch (error) {
  console.error(`Error: ${error instanceof Error ? error.message : String(error)}`);
  process.exit(1);
}
