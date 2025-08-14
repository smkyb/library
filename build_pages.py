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
    .button_sq { display: flex; font-size: 22px; flex-direction: column; gap: 20px; margin-bottom: 10px; padding: 10px; background-color: rgb(89, 158, 157); color: white; border: none; padding-left: 20px; padding-right: 20px; cursor: pointer; text-align: center; }
</style>
</head>
<body>
""")

def WriteTagD(f):
    f.write("""<script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
<script>
    marked.setOptions({ gfm: true, breaks: true, headerIds: true, mangle: false, highlight: (code, lang) => { if (lang && hljs.getLanguage(lang)) { return hljs.highlight(code, { language: lang }).value; } return hljs.highlightAuto(code).value; } });
    const el = document.getElementById("md_content");
    el.innerHTML = marked.parse(el.textContent);
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
                
                res_str += f"<button class=\"button_sq\" onclick=\"location.href=\'/library/{page_path}\'\">{item}</button>\n"
        elif item.endswith(".cpp") and not item.endswith(".test.cpp"):
            cnt_pages += 1
            page_name = f"page{cnt_pages}.html"
            page_path = os.path.join("gh_pages", page_name)
            with open(page_path, "w", encoding="utf-8") as f:
                with open(now, "r", encoding="utf-8") as code_f:
                    with open(os.path.join(path, "README.md"), "r") as readme_f:
                        WriteTagU(f)
                        f.write(f"<article id=\"md_content\" class=\"markdown-body\">\n{EscapedMarkdown(readme_f.read())}</article>\n")
                        f.write(f"<pre>\n{html.escape(code_f.read(), quote=True)}</pre>\n")
                        WriteTagD(f)
            res_str += f"<button class=\"button_sq\" onclick=\"location.href=\'/library/{page_path}\'\">{item}</button>\n"
    
    return res_str

with open("index.html", "w", encoding="utf-8") as f:
    res_str = FindCppFiles("cpp")
    if len(res_str) != 0:
        WriteTagU(f)
        f.write(res_str)
        WriteTagD(f)
