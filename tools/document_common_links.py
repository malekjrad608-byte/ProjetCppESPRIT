"""Document the actual Qt schema without exporting business data."""
from pathlib import Path
import json

root = Path(__file__).resolve().parents[1]
folder = root / 'artifacts/local-links-20261015'
metadata = json.loads((folder/'common-after/metadata.json').read_text(encoding='utf-8'))
constraints = {c['CONSTRAINT_NAME']: c for c in metadata['constraints']}
columns = metadata['constraint_columns']
lines = ['ANIWORLD_APP / XEPDB1 - Modele relationnel effectivement utilise par Qt',
         'Mise a jour : 15 octobre 2026 - migration 003_common_links.sql',
         'PK : cle primaire ; FK : cle etrangere ; ? : valeur NULL autorisee.', '']
for table in sorted({c['TABLE_NAME'] for c in metadata['columns']}):
    lines += [table]
    for col in (c for c in metadata['columns'] if c['TABLE_NAME'] == table):
        refs = [constraints[c['CONSTRAINT_NAME']] for c in columns if c['TABLE_NAME']==table and c['COLUMN_NAME']==col['COLUMN_NAME']]
        flags = ' '.join({'P':'PK','R':'FK','U':'UNIQUE'}[c['CONSTRAINT_TYPE']] for c in refs if c['CONSTRAINT_TYPE'] in 'PRU')
        lines += [f"  {col['COLUMN_NAME']} : {col['DATA_TYPE']} {flags}{' ?' if col['NULLABLE']=='Y' else ''}"]
    lines += ['']
lines += ['RELATIONS (toutes actives et validees lors du controle)']
for c in metadata['constraints']:
    if c['CONSTRAINT_TYPE'] != 'R': continue
    parent = constraints[c['R_CONSTRAINT_NAME']]
    childcols = ','.join(x['COLUMN_NAME'] for x in columns if x['CONSTRAINT_NAME']==c['CONSTRAINT_NAME'])
    parentcols = ','.join(x['COLUMN_NAME'] for x in columns if x['CONSTRAINT_NAME']==c['R_CONSTRAINT_NAME'])
    lines += [f"  {c['TABLE_NAME']}({childcols}) -> {parent['TABLE_NAME']}({parentcols}) ; suppression parent : {c['DELETE_RULE']}"]
lines += ['', 'REGLES',
 'Un service peut avoir un employe, un client et un local. Les liens historiques inconnus restent NULL.',
 'Une consultation medicale peut etre liee a un service. Un service peut avoir plusieurs consultations.',
 'Chaque service genere automatiquement une facture brouillon et une ligne de prestation.',
 'Exporter la facture PDF la marque EMISE. Les informations facturees sont alors figees.',
 'La facture garde ses instantanes apres suppression du service ; ID_SERVICE devient NULL.',
 'Supprimer un employe reference par un planning/service est refuse. Le rendre indisponible conserve son historique.',
 'Choisir un client existant reutilise son ID. Nouveau client cree une identite distincte, meme pour un nom identique.',
 'MEDICAL_RECORDS contient encore les informations animales par consultation ; le dossier normalise du schema cible reste distinct.',
 'Une affectation de local ne garantit pas une reservation horaire ni une mise a jour automatique de son occupation.',
 'Les anciennes pages Lucidchart de la migration 002 ne contiennent pas encore ces nouvelles relations.']
(root/'database/conception/Modele_relationnel_Qt_003.txt').write_text('\n'.join(lines)+'\n',encoding='utf-8')
report = {}
for table in ['LOCAUX','SERVICES','MEDICAL_RECORDS','STOCK_LOTS']:
    before=json.loads((folder/'common-before'/f'{table}.json').read_text(encoding='utf-8'))
    after=json.loads((folder/'common-after'/f'{table}.json').read_text(encoding='utf-8'))
    key='LOT' if table=='STOCK_LOTS' else 'ID'
    byid={r[key]:r for r in after}
    differences=[r[key] for r in before if r[key] not in byid or any(byid[r[key]].get(k)!=v for k,v in r.items())]
    report[table]={'before':len(before),'after':len(after),'original_rows_changed_or_missing':differences}
(root/'artifacts/common-links-20261015/data-preservation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report,indent=2))
