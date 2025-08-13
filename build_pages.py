import shutil, os

if os.path.exists("gh_pages"):
    shutil.rmtree("gh_pages")
os.makedirs("gh_pages")
cnt_pages = 0

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
                    f.write(res)
                
                if len(res_str) == 0:
                    res_str += "<ul>\n"
                res_str += f"<li><a href=/library/{page_path}>{item}</a></li>\n"
        elif item.endswith(".cpp"):
            cnt_pages += 1
            page_name = f"page{cnt_pages}.html"
            page_path = os.path.join("gh_pages", page_name)
            with open(page_path, "w", encoding="utf-8") as f:
                with open(now, "r", encoding="utf-8") as code_f:
                    f.write(f"<pre>{code_f.read()}</pre>")
            
            if len(res_str) == 0:
                res_str += "<ul>\n"
            res_str += f"<li><a href=/library/{page_path}>{item}</a></li>\n"
    
    if len(res_str) != 0:
        res_str += "</ul>\n"
    
    return res_str

with open("index.html", "w", encoding="utf-8") as f:
    res_str = FindCppFiles("cpp")
    if len(res_str) != 0:
        f.write(res_str)
