import sys

path = sys.argv[1]
with open(path, 'r') as f:
    lines = f.readlines()

insertion = "#elif WTF_PLATFORM_WKC\n    FontCustomPlatformData(FontPlatformData::CreationData&&);\n"
result = []
done = False
for line in lines:
    if not done and line.strip() == "#elif USE(SKIA)":
        result.append(insertion)
        done = True
    result.append(line)

with open(path, 'w') as f:
    f.writelines(result)

print("Patched" if done else "WARNING: patch target not found")
