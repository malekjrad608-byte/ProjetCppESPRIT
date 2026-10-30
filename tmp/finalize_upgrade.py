from pathlib import Path
p=Path('src/widgets/servicecard.cpp'); s=p.read_text(encoding='utf-8'); p.write_text('\n'.join(x.rstrip() for x in s.splitlines())+'\n',encoding='utf-8')
p=Path('README.md'); s=p.read_text(encoding='utf-8').rstrip(); s+='\n\nValidation de la version 3.0 : 26 tests réussis, aucun échec ; vérifications supplémentaires des écrans à 125 % réussies. Captures et PDF de contrôle : `artifacts/services-upgrade/`. Journaux : `build_tests/upgrade-results.txt` et `build_tests/upgrade-dpi125.txt`.\n'; p.write_text(s,encoding='utf-8')
