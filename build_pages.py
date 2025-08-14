import shutil, os, html
from markdown_it import MarkdownIt

md = MarkdownIt()

if os.path.exists("gh_pages"):
    shutil.rmtree("gh_pages")
os.makedirs("gh_pages")
cnt_pages = 0

def WriteTagU(f):
    f.write("""<!DOCTYPE html>
<html lang="ja">
<head>
<meta charset="UTF-8">
<title>smkyb's library</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/github-markdown-css/5.5.1/github-markdown.min.css"></script>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">
<style>
    body { margin: 60px; font-family:'Noto Sans JP', Arial, sans-serif; font-size:large; }
    .markdown-body { box-sizing: border-box; max-width: 900px; margin: 0 auto; }
    .markdown-body pre { padding: 16px; overflow: auto; }
    .button_sq button       { display: flex; font-size: 22px; flex-direction: column; gap: 20px; margin-bottom: 10px; padding: 10px; padding-left: 30px; padding-right: 30px; background-color: rgb(89, 158, 157); color: white; border: none; cursor: pointer; text-align: center; transition: 0.5s; }
    .button_sq button:hover { display: flex; font-size: 22px; flex-direction: column; gap: 20px; margin-bottom: 10px; padding: 10px; padding-left: 90px; padding-right: 90px; background-color: rgb(0, 67, 67);    color: white; border: none; cursor: pointer; text-align: center; transition: 0.5s; }
    #button_copy { display: flex; font-size: 14px; }
    #button_copy_oneline { display: flex; font-size: 14px; }
</style>
</head>
<body>
""")

def WriteTagD(f):
    f.write("""<script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
<script>
    marked.setOptions({ gfm: true, breaks: true, headerIds: true, mangle: false, highlight: (code, lang) => { if (lang && hljs.getLanguage(lang)) { return hljs.highlight(code, { language: lang }).value; } return hljs.highlightAuto(code).value; } });
    const mdContent = document.getElementById("md_content");
    if(mdContent) mdContent.innerHTML = marked.parse(mdContent.textContent);
    
    const copyButton = document.getElementById("button_copy")
    if(copyButton){
        copyButton.addEventListener("click", () => {
            const text = copyButton.dataset.copy;
            navigator.clipboard.writeText(text);
        })
    }
    const copyButtonOneline = document.getElementById("button_copy_oneline")
    if(copyButtonOneline){
        copyButtonOneline.addEventListener("click", () => {
            const text = copyButtonOneline.dataset.copy;
            navigator.clipboard.writeText(text);
        })
    }
</script>
</body>
</html>""")

def EscapedMarkdown(s:str) -> str:
    if len(s) == 0:
        return ""
    
    pos = s.find("`")
    if pos == -1:
        return s
    res = s[:pos]
    s = s[pos:]
    if len(s) >= 3 and s[0] == s[1] == s[2]:
        res += "```"
        s = s[3:]
        pos = s.find("```")
        assert pos != -1
        pos += 3
        res += html.escape(s[:pos], quote=True)
        s = s[pos:]
    else:
        res += "`"
        s = s[1:]
        pos = s.find("`")
        assert pos != -1
        pos += 1
        res += html.escape(s[:pos], quote=True)
        s = s[pos:]
    return res + EscapedMarkdown(s)

#by chatGPT
def MakeOneLine(src: str) -> str:
    s = src
    n = len(s)
    i = 0
    out = []
    need_space = False
    col_start = 0

    def append_space_if_needed():
        nonlocal need_space
        if need_space:
            if out and out[-1] not in ("\n", " "):
                out.append(" ")
        need_space = False

    def is_line_preprocessor_at(pos_newline_index: int) -> bool:
        line = s[col_start:pos_newline_index]
        return line.lstrip().startswith("#")

    def count_backslashes_before(idx: int) -> int:
        cnt = 0
        j = idx - 1
        while j >= 0 and s[j] == "\\":
            cnt += 1
            j -= 1
        return cnt

    while i < n:
        ch = s[i]

        if ch == '"':
            append_space_if_needed()
            out.append('"')
            i += 1
            while i < n:
                out.append(s[i])
                if s[i] == '"' and (count_backslashes_before(i) % 2 == 0):
                    i += 1
                    break
                i += 1
            need_space = False
            continue

        if ch == "'":
            append_space_if_needed()
            out.append("'")
            i += 1
            while i < n:
                out.append(s[i])
                if s[i] == "'" and (count_backslashes_before(i) % 2 == 0):
                    i += 1
                    break
                i += 1
            need_space = False
            continue

        if ch == 'R' and i + 1 < n and s[i+1] == '"':
            j = i + 2
            while j < n and s[j] != '(':
                j += 1
            if j < n and s[j] == '(':
                delim = s[i+2:j]
                end_pat = ')' + delim + '"'
                append_space_if_needed()
                out.append(s[i:j+1])
                i = j + 1
                while i < n:
                    if s.startswith(end_pat, i):
                        out.append(end_pat)
                        i += len(end_pat)
                        break
                    else:
                        out.append(s[i])
                        i += 1
                need_space = False
                continue

        # コメント削除
        if ch == '/' and i + 1 < n and s[i+1] == '/':
            i += 2
            while i < n and s[i] != '\n':
                i += 1
            continue

        if ch == '/' and i + 1 < n and s[i+1] == '*':
            i += 2
            while i + 1 < n:
                if s[i] == '*' and s[i+1] == '/':
                    i += 2
                    break
                i += 1
            continue

        if ch in (" ", "\t", "\r", "\n", "\v", "\f"):
            if ch == '\n':
                if is_line_preprocessor_at(i):
                    if out and out[-1] == " ":
                        out.pop()
                    out.append("\n")
                    need_space = False
                    i += 1
                    col_start = i
                    continue
                else:
                    need_space = True
                    i += 1
                    col_start = i
                    continue
            else:
                need_space = True
                i += 1
                continue

        append_space_if_needed()
        out.append(ch)
        i += 1

    if out and out[-1] == " ":
        out.pop()
    return "".join(out)

def FindCppFiles(path:str) -> str:
    global cnt_pages
    
    res_str: str = ""
    items = sorted(os.listdir(path))
    
    for item in items:
        now = os.path.join(path, item)
        if os.path.isdir(now):
            res = FindCppFiles(now)
            if len(res) != 0:
                cnt_pages += 1
                page_name = f"page{cnt_pages}.html"
                page_path = os.path.join("gh_pages", page_name)
                with open(page_path, "w", encoding="utf-8") as f:
                    WriteTagU(f)
                    f.write(res)
                    WriteTagD(f)
                
                if len(res_str) == 0:
                    res_str += "<div class=\"button_sq\">\n"
                res_str += f"<button onclick=\"location.href=\'/library/{page_path}\'\">{item}</button>\n"
        elif item.endswith(".cpp") and not item.endswith(".test.cpp"):
            cnt_pages += 1
            page_name = f"page{cnt_pages}.html"
            page_path = os.path.join("gh_pages", page_name)
            with open(page_path, "w", encoding="utf-8") as f:
                with open(now, "r", encoding="utf-8") as code_f:
                    code_text = code_f.read()
                    with open(os.path.join(path, "README.md"), "r") as readme_f:
                        WriteTagU(f)
                        f.write(f"<article id=\"md_content\" class=\"markdown-body\">\n{EscapedMarkdown(readme_f.read())}</article>\n")
                        f.write(f"<button id=\"button_copy\" data-copy=\"{html.escape(code_text, quote=True)}\">copy</button>\n")
                        f.write(f"<button id=\"button_copy_oneline\" data-copy=\"{html.escape(MakeOneLine(code_text), quote=True)}\">copy_oneline</button>\n")
                        WriteTagD(f)
            if len(res_str) == 0:
                res_str += "<div class=\"button_sq\">\n"
            res_str += f"<button onclick=\"location.href=\'/library/{page_path}\'\">{item}</button>\n"
    
    if len(res_str) != 0:
        res_str += "</div>\n"
    return res_str

with open("index.html", "w", encoding="utf-8") as f:
    res_str = FindCppFiles("cpp")
    if len(res_str) != 0:
        WriteTagU(f)
        f.write("""<h1>smkyb's library</h1>
""")
        f.write(res_str)
        WriteTagD(f)
