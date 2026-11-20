from pathlib import Path
p=Path('README.md');s=p.read_text(encoding='utf-8');anchor='Les sections suivantes sont historiques ;'
summary='Validation du 16/10/2026 : compilation Release et Debug réussie, 37 tests de régression réussis (10 tests optionnels ignorés), deux scénarios Oracle avec CRUD des cinq modules, contraintes, transferts, facturation et ouverture des interfaces réussis. Les données temporaires de test ont été supprimées. L’exécutable livré confirme `Connexion Qt QODBC : RAWAND / CDB$ROOT`. Rapports : `artifacts/five-final-integration-tests.txt`, `artifacts/five-regression-tests.txt`, `artifacts/five-final-database-state.txt`.\n\n'
s=s.replace(anchor,summary+anchor,1);p.write_text(s,encoding='utf-8')
