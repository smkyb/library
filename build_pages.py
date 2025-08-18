import shutil, os, html, subprocess, re

pages_path = "docs/pages"
if os.path.exists(pages_path):
    shutil.rmtree(pages_path)
os.makedirs(pages_path)

cnt_pages = 0
client = None

#タグを書く（始め）
def WriteTagU(f) -> None:
    f.write("""<!DOCTYPE html>
<html lang="ja">
<head>
<meta charset="UTF-8">
<title>smkyb's library</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/github-markdown-css/5.5.1/github-markdown.min.css">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">
<link rel="preload" href="./k-font/keifont.ttf" as="font" type="font/ttf" crossorigin>
<style>
    @font-face { font-family: "keifont"; src: url("./k-font/keifont.ttf") format("truetype"); font-display: swap; }
    html { background: repeating-linear-gradient(-45deg, rgb(245, 245, 245), 40px, rgb(255, 255, 255) 40px, rgb(255, 255, 255) 70px); }
    body { margin: 60px; margin-top: 60px; min-height: 100vh; font-family:'Noto Sans JP', Arial, sans-serif; font-size:large; }
    .title { padding-top: 20px; font-family: keifont; font-weight: lighter; }
    .markdown-body { box-sizing: border-box; max-width: 900px; margin: 0 auto; background: #ffffff56; backdrop-filter: blur(3px); color: black; padding: 40px; }
    .markdown-body pre { padding: 16px; overflow: auto; }
    .button_sq button       { display: flex; font-size: 22px; flex-direction: column; gap: 20px; margin-bottom: 10px; padding: 10px; padding-left: 30px; padding-right: 30px; background-color: rgb(89, 158, 157); color: white; border: none; cursor: pointer; text-align: center; transition: 0.5s; }
    .button_sq button:hover { display: flex; font-size: 22px; flex-direction: column; gap: 20px; margin-bottom: 10px; padding: 10px; padding-left: 90px; padding-right: 90px; background-color: rgb(0, 67, 67);    color: white; border: none; cursor: pointer; text-align: center; transition: 0.5s; }
    .links { text-decoration: none; color: rgb(10, 56, 117); font-weight: bold; }
    #button_copy { display: flex; font-size: 14px; }
    #button_copy_oneline { display: flex; font-size: 14px; }
    #status_bar { display: flex; justify-content: center; position: fixed; height: 70px; width:100%; top: 0px; left: 0px; background-color: rgba(255, 255, 255, 0.258);  backdrop-filter: blur(20px); font-family: sans-serif; font-size: 20px; font-weight: bolder; align-content: center; color: white; z-index: 998244353; }
    #status_bar a { margin: 20px; text-decoration: none; color: rgb(63, 97, 144); }
</style>
</head>
<body>
<div id="status_bar">
    <a href="/library/index.html">home</a>
    <a href="/library/docs/about.html">about</a>
    <a href="/library/docs/link.html">link</a>
</div>
""")

#タグを書く（終わり）
def WriteTagD(f) -> None:
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

#MarkDownのコードブロック内をエスケープしたものを返す
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

#cppコードをなるべく１行にまとめたものを返す
def MakeOneLine(code: str) -> str:
    result_lines = []
    for line in code.splitlines():
        stripped = line.strip()
        # コメント削除
        stripped = re.sub(r"//.*", "", stripped)
        stripped = re.sub(r"/\*.*?\*/", "", stripped)
        if not stripped:
            continue
        # プリプロセッサはそのまま
        if stripped.startswith("#"):
            result_lines.append(stripped)
        else:
            # 空白縮約
            stripped = re.sub(r"\s+", " ", stripped)
            if result_lines and not result_lines[-1].startswith("#"):
                result_lines[-1] += " " + stripped
            else:
                result_lines.append(stripped)
    return "\n".join(result_lines)

#末尾が.cpp(.test.cppは除く)であるファイルのパスを返す
def ListCppFile(path:str) -> list[str]:
    res:list[str] = []
    for item in os.listdir(path):
        now = path + "/" + item
        if os.path.isdir(now):
            res.extend(ListCppFile(now))
        elif now.endswith(".cpp") and not now.endswith(".test.cpp"):
            res.append(now)
    return res

#パスとコードに対応するREADMEをAIが作成する
def MakeREADME(README_path:str, code_text:str) -> None:
    if client == None:
        subprocess.run(["pip3", "install", "-q", "-U", "google-genai"])
        from google import genai
        client = genai.Client(api_key=os.getenv("GEMINI_KEY"))
    
    with open("cpp/hld/README.md", "r", encoding="utf-8") as EX_f:
        example_README1 = EX_f.read()
    with open("cpp/sort_segtree/README.md") as EX_f:
        example_README2 = EX_f.read()
    
    res = client.models.generate_content(
        model="gemini-2.5-flash",
        contents=[{
            "role": "user",
            "parts": [{ "text": f"以下に，競技プログラミングで用いるソースコード1つと，それとは異なるもののREADME.md2つを与えます．与えられたREADMEの記法にできる限り則って，与えられたソースコードのREADME.mdを作成してください．ただし，コードを利用するにあたって不必要な内部的な事情はなるべく書かないようにして，内容はなるべく端的に，「使い方が分かる程度」でお願いします．また，最初の```markdownとかはいりません．\n/*ソースコード*/\n{code_text}\n/*別のREADME.md，1つ目*/\n{example_README1}\n/*別のREADME.md，2つ目*/\n{example_README2}\n" }]
        }]
    )
    with open(README_path, "w", encoding="utf-8") as README_f:
        README_f.write(res.text)
    print("call gemini")
    print(f"token(MAX:250,000TPM) : {res.usage_metadata.prompt_token_count}")
    print(f"content :\nres.text")

def BuildPage(path:str) -> str:
    global cnt_pages
    global client
    
    dir_path = os.path.dirname(path)
    item_name = os.path.basename(path)
    cnt_pages += 1
    page_name = f"page{cnt_pages}.html"
    page_path = pages_path + "/" + page_name
    
    with open(path, "r", encoding="utf-8") as code_f:
        code_text = code_f.read()
    
    readme_path = dir_path + "/" + "README.md"
    if not os.path.exists(readme_path):
        MakeREADME(readme_path, code_text)
    
    with open(page_path, "w", encoding="utf-8") as write_f:
        with open(readme_path, "r", encoding="utf-8") as readme_f:
            WriteTagU(write_f)
            write_f.write(f"<article id=\"md_content\" class=\"markdown-body\">\n{EscapedMarkdown(readme_f.read())}</article>\n\n")
            write_f.write(f"<button id=\"button_copy\" data-copy=\"{html.escape(code_text, quote=True)}\">copy</button>\n")
            write_f.write(f"<button id=\"button_copy_oneline\" data-copy=\"{html.escape(MakeOneLine(code_text), quote=True)}\">copy_oneline</button>\n")
            WriteTagD(write_f)
    
    return f"<button onclick=\"location.href=\'{page_path}\'\">{item_name[:-4]}</button>\n"

#.cpp(.test.cppを除く)を探し，それぞれに対してページを作成し，それに通ずるボタンがまとめて書いてあるHTMLを返す
def FindCppFiles(path:str) -> str:
    global cnt_pages
    global client
    
    res_str: str = ""
    items = sorted(ListCppFile(path))
    
    if len(items) == 0:
        return ""
    
    res_str += "<div class=\"button_sq\">\n"
    for item_path in items:
        res_str += BuildPage(item_path)
    
    res_str += "</div>\n"
    return res_str

#index.html
with open("index.html", "w", encoding="utf-8") as f:
    with open("docs/index_content.txt", "r", encoding="utf-8") as content_f:
        WriteTagU(f)
        f.write(content_f.read())
        f.write(FindCppFiles("cpp"))
        WriteTagD(f)

#about.html
with open("docs/about.html", "w", encoding="utf-8") as f:
    with open("docs/about_content.txt", "r", encoding="utf-8") as content_f:
        WriteTagU(f)
        f.write(content_f.read())
        WriteTagD(f)

#link.html
with open("docs/link.html", "w", encoding="utf-8") as f:
    with open("docs/link_content.txt", "r", encoding="utf-8") as content_f:
        WriteTagU(f)
        f.write(content_f.read())
        WriteTagD(f)
