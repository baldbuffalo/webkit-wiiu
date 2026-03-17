import sys, re, json, os

in_file = sys.argv[1]
out_file = sys.argv[2]

properties = []

ext = os.path.splitext(in_file)[1].lower()

if ext == '.json':
    with open(in_file) as f:
        data = json.load(f)
    # handle both array and dict formats
    if isinstance(data, list):
        for entry in data:
            name = entry.get('name') or entry.get('property-name') or entry.get('id', '')
            if name and not name.startswith('-'):
                properties.append(name)
    elif isinstance(data, dict):
        for key in data.keys():
            if not key.startswith('-') and not key.startswith('//'):
                properties.append(key)
else:
    # .in format
    with open(in_file) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('//'):
                continue
            name = line.split()[0].strip()
            if name and re.match(r'^[a-zA-Z]', name):
                properties.append(name)

def to_camel(name):
    parts = re.sub(r'[-_](.)', lambda m: m.group(1).upper(), name)
    return 'CSSProperty' + parts[0].upper() + parts[1:]

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
