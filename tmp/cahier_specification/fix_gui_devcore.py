from pathlib import Path
p=Path(__file__).with_name('gui_devcore.cpp')
s=p.read_text(encoding='utf-8')
s=s.replace('setMinimumHeight(39+42*rows.size())','setMinimumHeight(55+42*rows.size())')
s=s.replace('bh->addWidget(muted("Espace de gestion  /  "+names[n]))','bh->addWidget(single("Gestion  /  "+names[n]))')
s=s.replace('filters->addWidget(txt("Dimanche 27 septembre 2026",14,"#355F49",true))','filters->addWidget(single("Dim. 27 septembre 2026",14))')
s=s.replace('10,1.6','10,1.5').replace('9,1.7','9,1.5').replace('14,1.8','14,1.75').replace('15.5,1.6','15.5,1.5')
s=s.replace('16:00 · Salle de soins 2\\nSarra B. · 45 min','16:00 · Espace éducation\\nYoussef M. · 45 min').replace('08:00 - 16:00','08:00 - 18:00')
p.write_text(s,encoding='utf-8')
