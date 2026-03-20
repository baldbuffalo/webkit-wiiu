import sys, re

def patch_file(path):
    with open(path, encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Remove lines referencing test-only enumeration/raw types
    # Use word boundaries to avoid false matches
    content = re.sub(r'^[^\n]*\bTestEnumeration\b[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestRaw\b[^\n]*\n', '', content, flags=re.MULTILINE)

    # Remove only TestRenderStyleStorage lines, not TestRenderStyleProperties
    content = re.sub(r'^[^\n]*\bTestRenderStyleStorage[^\n]*\n', '', content, flags=re.MULTILINE)
    content = re.sub(r'^[^\n]*\bTestRenderStyleHasExplicitlySet[^\n]*\n', '', content, flags=re.MULTILINE)

    # Remove entire ColorPropertyTraits specialization blocks for test properties
    content = re.sub(
        r'template<>\s*struct\s+ColorPropertyTraits<PropertyNameConstant<CSSPropertyTest[^>]*>>[^}]*\};\n',
        '',
        content,
        flags=re.DOTALL
    )

    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

for path in sys.argv[1:]:
    patch_file(path)
    print(f'Patched: {path}')
