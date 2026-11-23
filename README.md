# smkyb's Library

C++20 競技プログラミング用ライブラリ集および GitHub Pages 公開サイト（`https://smkyb.github.io/library/`）です。

編集の唯一のソースは `C:\vscode programs\Libraries` とし、このリポジトリへの片方向同期と、`content/catalog.json` による公開対象の明示管理を行います。

## ディレクトリ構成

- `libraries/` — `C:\vscode programs\Libraries` から片方向同期された `.cpp` / `.hpp` ファイル群（直接編集しないこと）
- `content/catalog.json` — サイト基本設定、カテゴリ一覧、公開ライブラリ設定（`published: true` のみサイトにビルド・配信される）
- `content/docs/<id>.md` — 任意の Markdown + KaTeX 解説ドキュメント
- `scripts/sync.mjs`, `scripts/sync.ps1` — `Libraries` からのミラーリング同期スクリプト
- `vite-plugins/libraryContentPlugin.ts` — ビルド時の Shiki シンタックスハイライト・Markdown/KaTeX・TOC・静的ルート HTML 生成プラグイン
- `src/styles/tokens.css` — 色・余白・タイポグラフィ・レイアウト幅のデザイン集約トークン

## 日常の同期・公開フロー

普段のコード編集は `C:\vscode programs\Libraries` 側のみで行います。

### 1. 同期のみ（差分確認）

```powershell
# 書き込みなしで差分を確認
npm run sync:dry

# C:\vscode programs\Libraries -> libraries/ へ同期
npm run sync
```

同期時、`0_*` ファイル（`0_cpptosnippet.cpp` 等）や `.exe`・`.txt` は自動的に除外されます。

### 2. 同期 ＋ ビルド検証 ＋ commit / push

```powershell
# libraries/ の差分のみを自動コミット・push（他に未コミット変更があると安全のため停止）
.\scripts\sync.ps1 -Push

# 解説や設定変更もまとめてコミット・push する場合
.\scripts\sync.ps1 -Push -IncludeAll -Message "docs: update library docs"
```

## 新しいライブラリをサイトに公開する手順

1. `npm run sync` で最新の `.cpp` が `libraries/` に取り込まれていることを確認する
2. `content/catalog.json` の `libraries` 配列にエントリを追加し、`"published": true` を設定する（`title`, `summary`, `complexity`, `tags`, `verified` は省略可能）
3. 解説を載せたい場合は `content/docs/<id>.md` を作成する（概要は `catalog.json` の `summary` に簡潔に記載し、ページ上部のソースコードリンクと下部の折りたたみ式ソースコードは自動表示される）。解説を書く場合は以下の見出し構成を標準とします（解答例は `#include <bits/stdc++.h>` と `#include <atcoder/XXX>` が利用可能な前提、`cin.tie(nullptr)` 等の高速化定型文は省略）。
   - `## 例`（問題文を直書きし、そのまま続けてライブラリ本体をコメントで省略した解答コードを記載）
   - `## 関数一覧`
   - `## 関数の説明`（ACL 準拠: `### 関数名` → C++ シグネチャ → 説明 → `**制約**` → `**計算量**`）
4. `npm run build` で型チェックと本番ビルドが通ることを確認する

## GitHub Pages 初回セットアップ

GitHub リポジトリ（`smkyb/library`）の **Settings → Pages → Build and deployment → Source** を **GitHub Actions** に設定してください。`main` ブランチへの push 時に `.github/workflows/deploy.yml` が自動でビルドとデプロイを実行します。

## デザイン変更ポイント

サイト全体の配色・タイポグラフィ・ボーダー・余白・レイアウト寸法は `src/styles/tokens.css` に集約しています。テーマ配色やフォントを変更する際は同ファイルを中心に編集してください。
