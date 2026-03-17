import sys

yaml_file = sys.argv[1]
out_file = sys.argv[2]

with open(out_file, 'w') as f:
    f.write('#pragma once\n')
    f.write('#include <wtf/text/WTFString.h>\n')
    f.write('namespace WebCore {\n')
    f.write('struct SettingsValues { };\n')
    f.write('class Settings {\n')
    f.write('public:\n')
    f.write('    static Settings& shared();\n')
    f.write('    const SettingsValues& values() const { return m_values; }\n')
    f.write('private:\n')
    f.write('    SettingsValues m_values;\n')
    f.write('};\n')
    f.write('} // namespace WebCore\n')
