import sys, re, os, glob

out_file = sys.argv[1]
source_dir = sys.argv[2]

EXCLUDED = {'CSSPropertyInvalid', 'cssPropertyIDEnumValueCount'}

identifiers = set()
pattern = re.compile(r'\bCSSProperty([A-Z][a-zA-Z0-9]*)\b')

for ext in ('*.h', '*.cpp'):
    for path in glob.glob(os.path.join(source_dir, '**', ext), recursive=True):
        try:
            with open(path, encoding='utf-8', errors='ignore') as f:
                for m in pattern.finditer(f.read()):
                    name = m.group(0)
                    if name not in EXCLUDED:
                        identifiers.add(name)
        except Exception:
            pass

props = sorted(identifiers)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('#include <cstdint>\n')
    f.write('namespace WebCore {\n')
    f.write('enum CSSPropertyID : uint16_t {\n')
    f.write('    CSSPropertyInvalid = 0,\n')
    for i, name in enumerate(props, 1):
        f.write(f'    {name} = {i},\n')
    f.write(f'    cssPropertyIDEnumValueCount = {len(props) + 1},\n')
    f.write('};\n')
    f.write('} // namespace WebCore\n')
    f.write('using WebCore::CSSPropertyID;\n')
    f.write('using WebCore::cssPropertyIDEnumValueCount;\n')
