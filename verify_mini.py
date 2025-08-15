import subprocess, os, sys

changed_files = subprocess.run(
    ["git", "diff", "--name-only", "HEAD~1"],
    capture_output=True,
    text=True
)
changed_files = changed_files.stdout.splitlines()
changed_files = [f for f in changed_files if f.endswith(".cpp")]
changed_files = [f for f in changed_files if os.path.exists(f)]

verify_files: list[str] = []

def AddTestFiles(path:str):
    items: list[str] = os.listdir(path)
    for item in items:
        now = os.path.join(path, item)
        if item.endswith(".test.cpp"):
            verify_files.append(now)
        elif os.path.isdir(now):
            AddTestFiles(now)

for file in changed_files:
    if not file.endswith(".test.cpp"):
        AddTestFiles(os.path.dirname(file))
for file in changed_files:
    if file.endswith(".test.cpp"):
        verify_files.append(file)

verify_files = list(set(verify_files))
failed: int = 0

for file in verify_files:
    result = subprocess.run(["oj-verify", "run", file])
    if result.returncode != 0:
        failed = 1

#sys.exit(failed)
