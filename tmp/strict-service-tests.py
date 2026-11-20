from pathlib import Path
import re
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('findChild<QTableWidget *>("serviceTable")','findChild<QTableView *>("serviceTable")').replace('#include <QTableWidget>','#include <QTableWidget>\n#include <QTableView>')
s=s.replace('findChild<QTableView *>("serviceTable")->rowCount()', 'findChild<QTableView *>("serviceTable")->model()->rowCount()')
# Restrict changes to test methods that exercise the service list.
parts=re.split(r'(?=    void \w+\(\) \{)',s)
for i,part in enumerate(parts):
    if 'auto *table=' not in part or 'findChild<QTableView *>("serviceTable")' not in part:continue
    part=part.replace('table->rowCount()', 'table->model()->rowCount()')
    part=re.sub(r'table->item\(([^,]+),0\)->data\(Qt::UserRole\)',r'table->model()->index(\1,0).data()',part)
    part=re.sub(r'table->item\(([^,]+),0\)->text\(\)',r'table->model()->index(\1,1).data().toString()',part)
    parts[i]=part
s=''.join(parts);p.write_text(s,encoding='utf-8')
