import os, sys

tags_file = sys.argv[1]
attrs_file = sys.argv[2]
out_file = sys.argv[3]

def to_ident(name):
    return name.replace('-', '_')

def read_names(path):
    names = []
    try:
        with open(path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#') or '=' in line:
                    continue
                name = line.split()[0].split(',')[0].strip()
                if name:
                    names.append(name)
    except FileNotFoundError:
        pass
    return names

tags = read_names(tags_file)
attrs = read_names(attrs_file)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('#include "QualifiedName.h"\n')
    f.write('#include <wtf/text/AtomString.h>\n')
    f.write('namespace WebCore {\n')
    f.write('namespace HTMLNames {\n')
    for tag in tags:
        f.write(f'extern const QualifiedName {to_ident(tag)}Tag;\n')
    for attr in attrs:
        f.write(f'extern const QualifiedName {to_ident(attr)}Attr;\n')
    f.write('void init();\n')
    f.write('} // namespace HTMLNames\n')
    f.write('} // namespace WebCore\n')
