from pathlib import Path
import xml.etree.ElementTree as E
import json,copy,shutil

root=Path(__file__).resolve().parents[2]
capture=root/'artifacts/new-stock-integration/designer'
bindings=json.loads((capture/'bindings.json').read_text())
assets=root/'resources/designer'
assets.mkdir(parents=True,exist_ok=True)
keep={'geometry','minimumSize','maximumSize','sizePolicy','font','styleSheet','text','wordWrap','alignment','textFormat','frameShape','frameShadow','lineWidth','midLineWidth','widgetResizable','horizontalScrollBarPolicy','verticalScrollBarPolicy','placeholderText','clearButtonEnabled','maxLength','echoMode','readOnly','checkable','checked','autoDefault','default','flat','iconSize','toolTip','cursor','autoFillBackground','accessibleName','accessibleDescription','orientation','textVisible','minimum','maximum','value','format','currentIndex','editTriggers','selectionBehavior','selectionMode','showGrid','wordWrap','alternatingRowColors','sortingEnabled','focusPolicy','verticalScrollMode','horizontalScrollMode','sizeAdjustPolicy','minimumContentsLength','editable','insertPolicy','spacing','horizontalSpacing','verticalSpacing','sizeConstraint','checkState','title'}
used=set()
def prop(node,name,kind,value):
 for child in list(node.findall('property')):
  if child.get('name')==name:node.remove(child)
 child=E.SubElement(node,'property',name=name)
 E.SubElement(child,kind).text=str(value)
 return child
def clean(node,meta,page):
 cls=node.get('class','');name=node.get('name','');info=meta.get(name,{})
 # Private Qt children (viewports, combo popup windows, headers) are recreated by Qt.
 if node.tag=='widget':
  if cls=='QStackedWidget':
   layout=node.find('layout');pages=list(layout.findall('item/widget')) if layout is not None else []
   for child in list(node):
    if child.tag in ('layout','widget'):node.remove(child)
   for child in pages:node.append(child)
  elif cls=='QScrollArea':
   content=next((w for w in node.iter('widget') if w.get('name')==info.get('content')),None)
   for child in list(node):
    if child.tag in ('widget','layout'):node.remove(child)
   if content is not None:node.append(content)
  elif cls in ('QComboBox','QLineEdit','QTableView','QTableWidget','QLabel','QPushButton','QProgressBar','CardGrid','DetailPanel','ClinicMap'):
   for child in list(node):
    if child.tag in ('widget','layout','action','addaction'):node.remove(child)
  elif node.find('layout') is not None:
   for child in list(node.findall('widget')):node.remove(child)
  if cls in ('ServiceWidget','StockWidget','GestionEmployes','PageLocaux'):node.set('class','QWidget')
 if node.tag=='layout':
  if name in ('Service_m_grid','Service_m_statusLayout','Employees_m_gridCards'):
   for item in list(node.findall('item')):node.remove(item)
  for key in ('stretch','columnstretch','rowstretch'):
   values=info.get(key,[])
   if any(values):node.set(key,','.join(map(str,values)))
  for key,value in zip(('leftMargin','topMargin','rightMargin','bottomMargin'),info.get('margins',[])):prop(node,key,'number',value)
 for child in list(node):
  if child.tag=='property':
   pn=child.get('name')
   if pn not in keep and pn not in ('leftMargin','topMargin','rightMargin','bottomMargin'):node.remove(child)
   elif pn in ('styleSheet','text','toolTip','accessibleName','accessibleDescription') and not ''.join(child.itertext()).strip():node.remove(child)
  elif child.tag in ('widget','layout','item'):clean(child,meta,page)
  elif child.tag in ('action','addaction'):node.remove(child)
 if node.tag=='widget':
  if cls=='QComboBox':
   for child in list(node.findall('item')):node.remove(child)
   for item in info.get('items',[]):
    child=E.SubElement(node,'item');prop(child,'text','string',item['text'])
  for key,val in info.get('props',{}).items():
   if key.startswith('_') or not isinstance(val,(str,bool,int)):continue
   p=prop(node,key,'bool' if isinstance(val,bool) else 'number' if isinstance(val,int) else 'string',str(val).lower() if isinstance(val,bool) else val);p.set('stdset','0')
  old=bindings.get(name,{}).get('oldName','')
  if old and old!=name:prop(node,'objectName','string',old)
  if info.get('hidden') and cls not in ('QWidget','QStackedWidget','QScrollArea'):prop(node,'visible','bool','false')
  icon=capture/'icons'/f'{name}.png'
  if icon.exists() and (info.get('icon') or info.get('pixmap')):
   shutil.copy2(icon,assets/icon.name);used.add(icon.name)
   if info.get('icon'):
    p=E.SubElement(node,'property',name='icon');iset=E.SubElement(p,'iconset',resource='../resources/designer.qrc');E.SubElement(iset,'normaloff').text=':/designer/'+icon.name
   else:
    p=prop(node,'pixmap','pixmap',':/designer/'+icon.name);p.find('pixmap').set('resource','../resources/designer.qrc')
 # Restore a sensible initial selected page; data controllers set their own current page.
 if cls=='QStackedWidget':prop(node,'currentIndex','number',0)

ui=E.Element('ui',version='4.0');E.SubElement(ui,'class').text='MainWindow'
main=E.SubElement(ui,'widget',{'class':'QMainWindow','name':'MainWindow'})
prop(main,'windowTitle','string','ANIWORLD — Interface intégrée')
central=E.SubElement(main,'widget',{'class':'QWidget','name':'centralWidget'})
layout=E.SubElement(central,'layout',{'class':'QVBoxLayout','name':'integratedLayout'})
item=E.SubElement(layout,'item');stack=E.SubElement(item,'widget',{'class':'QStackedWidget','name':'pageStack'})
for name in ['Service','Stock','Employees','Locaux']:
 doc=E.parse(capture/(name+'.ui'));w=doc.getroot().find('widget');meta=json.loads((capture/(name+'.json')).read_text());clean(w,meta,name)
 w.set('name',name+'Page');prop(w,'windowTitle','string',name)
 stack.append(w)
medical=E.parse(root/'src/medical/medicalwidget.ui').getroot().find('widget')
medical.set('name','MedicalPage')
# Prefix the medical object's Designer identifiers; the generated medical binding removes the prefix.
for obj in medical.iter():
 if obj.tag in ('widget','layout','spacer','action') and obj.get('name') and obj is not medical:obj.set('name','Medical_'+obj.get('name'))
stack.append(medical)
custom=E.SubElement(ui,'customwidgets')
for cls,base,header in [('CardGrid','QWidget','src/locaux/cardgrid.h'),('DetailPanel','QFrame','src/locaux/detailpanel.h'),('ClinicMap','QWidget','src/locaux/clinicmap.h')]:
 c=E.SubElement(custom,'customwidget');E.SubElement(c,'class').text=cls;E.SubElement(c,'extends').text=base;E.SubElement(c,'header').text=header
resources=E.SubElement(ui,'resources');E.SubElement(resources,'include',location='../resources/designer.qrc')
E.SubElement(ui,'connections');E.indent(ui,space=' ')
E.ElementTree(ui).write(root/'src/mainwindow.ui',encoding='utf-8',xml_declaration=True)
qrc=E.Element('RCC');res=E.SubElement(qrc,'qresource',prefix='/designer')
for filename in sorted(used):E.SubElement(res,'file',alias=filename).text='designer/'+filename
E.indent(qrc);E.ElementTree(qrc).write(root/'resources/designer.qrc',encoding='utf-8',xml_declaration=True)
print('Created single Designer source:',(root/'src/mainwindow.ui').stat().st_size,'bytes;',len(used),'icons')
