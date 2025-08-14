import shutil, os, html

if os.path.exists("gh_pages"):
    shutil.rmtree("gh_pages")
os.makedirs("gh_pages")
cnt_pages = 0

def WriteTagU(f):
    f.write("<!DOCTYPE html>\n" \
    "<html lang=\"ja\">\n" \
    "<head>\n" \
    "<meta charset=\"UTF-8\">\n" \
    "<tytle>smkyb's library</title>\n" \
    "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?family=Noto+Sans+JP&display=swap\">\n" \
    "<script src=\"https://cdn.jsdelivr.net/npm/marked/marked.min.js\"></script>\n" \
    "<style>\n" \
    "body {\n" \
    "    margin: 60px;\n" \
    "    font-family:'Noto Sans JP', Arial, sans-serif;\n" \
    "    font-size:large;\n" \
    "}\n" \
    "</style>\n" \
    "</head>\n" \
    "<body>\n")

def WriteTagD(f):
    f.write("<script>\n" \
    "    const content = document.getElementById(\"md_content\")\n" \
    "    content.innerHTML = marked.parse(content.textContent)\n" \
    "</script>\n" \
    "</body>\n" \
    "</html>\n")

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
                    res_str += "<ul>\n"
                res_str += f"<li><a href=/library/{page_path}>{item}</a></li>\n"
        elif item.endswith(".cpp") and not item.endswith(".test.cpp"):
            cnt_pages += 1
            page_name = f"page{cnt_pages}.html"
            page_path = os.path.join("gh_pages", page_name)
            with open(page_path, "w", encoding="utf-8") as f:
                with open(now, "r", encoding="utf-8") as code_f:
                    with open(os.path.join(path, "README.md"), "r") as readme_f:
                        WriteTagU(f)
                        f.write(f"<div id=\"md_content\">\n{readme_f.read()}</div>\n")
                        f.write(f"<pre>\n{html.escape(code_f.read(), quote=True)}</pre>\n")
                        WriteTagD(f)
            
            if len(res_str) == 0:
                res_str += "<ul>\n"
            res_str += f"<li><a href=/library/{page_path}>{item}</a></li>\n"
    
    if len(res_str) != 0:
        res_str += "</ul>\n"
    
    return res_str

with open("index.html", "w", encoding="utf-8") as f:
    res_str = FindCppFiles("cpp")
    if len(res_str) != 0:
        WriteTagU(f)
        f.write(res_str)
        WriteTagD(f)
