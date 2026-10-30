"""Check that SQL execution in compiled sources belongs to a declared C++ class.

This lexical guard complements compilation and runtime tests; it is not a C++
parser. Strings/comments are masked so SQL braces and lambdas do not confuse
the enclosing method check. Unqualified namespace functions are rejected.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def mask_cpp(text):
    pattern = (
        r'R"(?P<delimiter>[^\s()\\]{0,16})\(.*?\)(?P=delimiter)"'
        r'|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
        r'|//[^\n]*|/\*.*?\*/'
    )
    text = re.sub(pattern, lambda m: "".join("\n" if c == "\n" else " " for c in m[0]),
                  text, flags=re.S)
    return re.sub(r"(?m)^[ \t]*#.*$", lambda m: " " * len(m[0]), text)


def inspect(text, classes):
    masked = mask_cpp(text)
    methods = []
    pattern = (r"(?m)^[ \t]*(?:[\w:<>,*&]+[ \t\n]+)*[*&]?\s*"
               r"(?P<owner>\w+)::\w+\s*\([^;{}]*\)\s*(?:const\s*)?\{")
    for match in re.finditer(pattern, masked):
        if match["owner"] not in classes:
            continue
        depth, end = 1, match.end()
        while depth and end < len(masked):
            depth += (masked[end] == "{") - (masked[end] == "}")
            end += 1
        methods.append((match.start(), end, match["owner"]))
    failures, owners = [], set()
    for match in re.finditer(r"\bQSqlQuery\b|\.\s*prepare\s*\(|->\s*setQuery\s*\(", masked):
        enclosing = [owner for start, end, owner in methods if start <= match.start() < end]
        if not enclosing:
            failures.append(text.count("\n", 0, match.start()) + 1)
        else:
            owners.update(enclosing)
    return sorted(set(failures)), owners


def main():
    project = (ROOT / "sources.pri").read_text(encoding="utf-8-sig")
    files = {ROOT / p for p in re.findall(r"\$\$PWD/(\S+\.(?:cpp|h))", project)}
    files.add(ROOT / "src/main.cpp")
    classes = set()
    for path in files:
        classes.update(re.findall(r"\bclass\s+(\w+)\s*(?:final\s*)?(?:[:{])",
                                  mask_cpp(path.read_text(encoding="utf-8-sig"))))
    failures, owners = [], set()
    cpp_files = sorted(p for p in files if p.suffix == ".cpp")
    for path in cpp_files:
        lines, found = inspect(path.read_text(encoding="utf-8-sig"), classes)
        owners.update(found)
        failures.extend(f"{path.relative_to(ROOT)}:{line}: SQL outside a class method" for line in lines)
    print(f"SQL ownership: {len(cpp_files)} compiled C++ sources checked.")
    if failures:
        print("\n".join(failures))
        return 1
    print("PASS: SQL belongs to class methods: " + ", ".join(sorted(owners)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
