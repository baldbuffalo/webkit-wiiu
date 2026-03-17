import sys, re, os, glob

out_file = sys.argv[1]
source_dir = sys.argv[2]

# Scan all .h and .cpp files for CSSProperty identifiers
identifiers = set()
pattern = re.compile(r'\bCSSProperty([A-Z][a-zA-Z0-9]*)\b')

for ext in ('*.h', '*.cpp'):
    for path in glob.glob(os.path.join(source_dir, '**', ext), recursive=True):
        try:
            with open(path, encoding='utf-8', errors='ignore') as f:
                for m in pattern.finditer(f.read()):
                    identifiers.add(m.group(0))
        except Exception:
            pass

# Sort for stability
props = sorted(identifiers)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('namespace WebCore {\n')
    f.write('enum CSSPropertyID {\n')
    f.write('    CSSPropertyInvalid = 0,\n')
    for i, name in enumerate(props, 1):
        f.write(f'    {name} = {i},\n')
    f.write('};\n')
    f.write('} // namespace WebCore\n')
    f.write('using WebCore::CSSPropertyID;\n')
