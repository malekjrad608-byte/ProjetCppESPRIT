from pathlib import Path
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('    void oracleFiveTablesIntegration() {',Path('tmp/five-ui-test.txt').read_text(encoding='utf-8-sig')+'\n    void oracleFiveTablesIntegration() {');p.write_text(s,encoding='utf-8')
