import sys

tags_file = sys.argv[1]
out_file = sys.argv[2]

tags = []
with open(tags_file) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        name = line.split()[0].split(',')[0].strip()
        if name:
            tags.append(name)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('namespace WebCore {\n')
    for tag in tags:
        cap = tag[0].upper() + tag[1:]
        f.write(f'class HTML{cap}Element;\n')
        f.write(f'inline bool isHTML{cap}Element(const Element&) {{ return false; }}\n')
    f.write('} // namespace WebCore\n')
