"""Read-only checks for the architecture required by the CRUD workshop."""
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
view_files = []
for directory in ('ui', 'dialogs', 'widgets'):
    view_files += list((root / 'src' / directory).glob('*.cpp'))
    view_files += list((root / 'src' / directory).glob('*.h'))
view_files += [root / p for p in (
    'src/employees/gestionemployes.cpp', 'src/stock/stockwidget.cpp',
    'src/medical/medicalwidget.cpp', 'src/locaux/pagelocaux.cpp', 'src/mainwindow.cpp')]
errors = []
for path in view_files:
    text = path.read_text(encoding='utf-8-sig')
    if re.search(r'\bQSqlQuery\b|\bQSqlDatabase\s*::|\bSELECT\b|\bINSERT\s+INTO\b|\bUPDATE\s+\w+\s+SET\b|\bDELETE\s+FROM\b', text):
        errors.append(f'SQL/database access in view: {path.relative_to(root)}')
for path in (root / 'src').rglob('*.cpp'):
    if 'rh' in path.parts:  # Separate application, not compiled by gestion_service.pro.
        continue
    text = path.read_text(encoding='utf-8-sig')
    for name in set(re.findall(r'QSqlQuery\s+(\w+)\s*\(', text)):
        if re.search(r'\b' + name + r'\.exec\(\s*[^\s)]', text):
            errors.append(f'SQL passed to exec instead of prepare: {path.relative_to(root)} ({name})')
for entity in ('src/models/service.h', 'src/employees/employe.h', 'src/medical/medicalrecord.h'):
    text = (root / entity).read_text(encoding='utf-8-sig')
    for operation in ('ajouter', 'afficher', 'modifier', 'supprimer'):
        if operation not in text:
            errors.append(f'{entity}: missing {operation}')
print(f'Checked {len(view_files)} view files; SQL confined to model/connection sources.')
if errors:
    print('\n'.join(errors))
    raise SystemExit(1)
print('PASS: no SQL in views; prepared queries; entity CRUD methods present.')
from check_model_sql_ownership import main as check_sql_ownership
raise SystemExit(check_sql_ownership())
