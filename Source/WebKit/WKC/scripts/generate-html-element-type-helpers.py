import sys

tags_file = sys.argv[1]
out_file = sys.argv[2]

def to_ident(name):
    return name.replace('-', '_')

tags = []
with open(tags_file) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#') or '=' in line:
            continue
        name = line.split()[0].split(',')[0].strip()
        if name:
            tags.append(name)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('namespace WebCore {\n')
    f.write('class Element;\n')
    for tag in tags:
        ident = to_ident(tag)
        cap = ident[0].upper() + ident[1:]
        f.write(f'class HTML{cap}Element;\n')
        f.write(f'inline bool isHTML{cap}Element(const Element&) {{ return false; }}\n')
    f.write('} // namespace WebCore\n')
