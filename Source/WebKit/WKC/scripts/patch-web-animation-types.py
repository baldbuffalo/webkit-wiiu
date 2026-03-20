import sys

path = sys.argv[1]
with open(path, 'r') as f:
    lines = f.readlines()

insertion = '#include "CSSPropertyIDHashTraits.h"\n'
result = []
done = False
for line in lines:
    if not done and line.strip() == '#pragma once':
        result.append(line)
        result.append(insertion)
        done = True
        continue
    result.append(line)

with open(path, 'w') as f:
    f.writelines(result)

print("Patched" if done else "WARNING: patch target not found")
