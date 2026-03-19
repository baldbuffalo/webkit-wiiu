import sys

path = sys.argv[1]
with open(path, 'r') as f:
    lines = f.readlines()

insertion = "#elif WTF_PLATFORM_WKC\n    FontCustomPlatformData();\n"
result = []
done = False
in_struct = False
for line in lines:
    if 'struct FontCustomPlatformData' in line:
        in_struct = True
    if not done and in_struct and line.strip() == "#elif USE(SKIA)":
        result.append(insertion)
        done = True
    result.append(line)

with open(path, 'w') as f:
    f.writelines(result)

print("Patched" if done else "WARNING: patch target not found")
