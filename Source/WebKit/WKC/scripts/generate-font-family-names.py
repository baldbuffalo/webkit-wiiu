import os, sys

in_file = sys.argv[1]
out_file = sys.argv[2]

names = []
with open(in_file) as f:
    for line in f:
        line = line.strip()
        if line and not line.startswith('#'):
            name = line.split('=')[0].strip()
            if name:
                names.append(name)

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('#include <wtf/text/AtomString.h>\n')
    f.write('namespace WebCore {\n')
    f.write('namespace WebKitFontFamilyNames {\n')
    for name in names:
        f.write(f'extern const WTF::AtomString& {name};\n')
    f.write('} // namespace WebKitFontFamilyNames\n')
    f.write('} // namespace WebCore\n')
