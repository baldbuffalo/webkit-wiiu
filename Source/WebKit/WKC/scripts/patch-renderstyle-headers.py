import sys, re

def patch_file(path):
    with open(path, encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Remove lines referencing test-only enumeration/raw types
    content = re.sub(r'^[^\n]*\bTestEnumeration\b[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestRaw\b[^\n]*\n', '', content, flags=re.MULTILINE)

    # Remove only TestRenderStyleStorage and TestRenderStyleHasExplicitlySet lines
    content = re.sub(r'^[^\n]*\bTestRenderStyleStorage[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestRenderStyleHasExplicitlySet[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestLogicalPropertyGroup[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestColor\b[^\n]*\n', '', content, flags=re.MULTILINE)

    # Remove lines referencing level1 (test-only storage struct member)
    content = re.sub(r'^[^\n]*\blevel1\b[^\n]*\n', '', content, flags=re.MULTILINE)

    # Remove entire ColorPropertyTraits specialization blocks for test properties
    content = re.sub(
        r'template<>\s*struct\s+ColorPropertyTraits<PropertyNameConstant<CSSPropertyTest[^>]*>>[^}]*\};\n',
        '',
        content,
        flags=re.DOTALL
    )

    # Remove entire inline function bodies for test color specializations
    # e.g. inline const Color& ColorPropertyTraits<PropertyNameConstant<CSSPropertyTest...>>::...
    content = re.sub(
        r'inline[^\n]*ColorPropertyTraits<PropertyNameConstant<CSSPropertyTest[^>]*>>[^\n]*\n[^}]*\}\n',
        '',
        content,
        flags=re.DOTALL
    )

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

for path in sys.argv[1:]:
    patch_file(path)
    print(f'Patched: {path}')
