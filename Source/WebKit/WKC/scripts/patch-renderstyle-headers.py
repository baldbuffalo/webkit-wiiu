import sys, re

def patch_file(path):
    with open(path, encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # Remove lines referencing test-only enumeration/raw types
    content = re.sub(r'.*TestEnumeration.*\n', '', content)
    content = re.sub(r'.*TestRaw.*\n', '', content)
    content = re.sub(r'.*TestRenderStyle(?!Properties).*\n', '', content)

    # Remove entire ColorPropertyTraits specialization blocks for test properties
    # These are multi-line blocks like:
    # template<> struct ColorPropertyTraits<PropertyNameConstant<CSSPropertyTestColor>> {
    #     ...
    # };
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
