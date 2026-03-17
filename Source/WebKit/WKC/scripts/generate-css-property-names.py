import sys, re

in_file = sys.argv[1]
out_file = sys.argv[2]

properties = []
with open(in_file) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('//'):
            continue
        # lines like: background-color or background-color [...]
        name = line.split()[0].strip()
        if name and not name.startswith('-'):
            properties.append(name)

def to_camel(name):
    parts = name.replace('-', ' ').title().replace(' ', '')
    return 'CSSProperty' + parts

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('namespace WebCore {\n')
    f.write('enum CSSPropertyID {\n')
    f.write('    CSSPropertyInvalid = 0,\n')
    for i, prop in enumerate(properties, 1):
        f.write(f'    {to_camel(prop)} = {i},\n')
    f.write('};\n')
    f.write('} // namespace WebCore\n')
    f.write('using WebCore::CSSPropertyID;\n')
