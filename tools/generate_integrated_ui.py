"""Compile the five pages of the single Designer form with Qt's uic.

mainwindow.ui is the only editable form. Small generated Ui classes let existing
page controllers keep their signals/slots and independent CRUD tests.
"""
import argparse
import copy
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser()
parser.add_argument('--input', required=True)
parser.add_argument('--output', required=True)
parser.add_argument('--uic', required=True)
args = parser.parse_args()
document = ET.parse(args.input).getroot()
sections = [('ServicePage', 'ServicePage'), ('StockPage', 'StockPage'),
            ('EmployeesPage', 'EmployeesPage'), ('LocauxPage', 'LocauxPage'),
            ('MedicalPage', 'MedicalWidget')]
headers = ['// Generated from src/mainwindow.ui. Do not edit.\n#pragma once\n']
with tempfile.TemporaryDirectory(prefix='aniworld-uic-') as temporary:
    for name, class_name in sections:
        page = document.find(f".//widget[@name='{name}']")
        if page is None:
            raise SystemExit(f'Missing page {name} in {args.input}')
        form = ET.Element('ui', version='4.0')
        ET.SubElement(form, 'class').text = class_name
        page = copy.deepcopy(page)
        page.set('name', class_name)
        if name == 'MedicalPage':
            for element in page.iter():
                if element.get('name', '').startswith('Medical_'):
                    element.set('name', element.get('name')[8:])
        form.append(page)
        for section in ('customwidgets', 'resources'):
            element = document.find(section)
            if element is not None:
                form.append(copy.deepcopy(element))
        ET.SubElement(form, 'connections')
        source = Path(temporary) / (class_name + '.ui')
        ET.ElementTree(form).write(source, encoding='utf-8', xml_declaration=True)
        result = subprocess.run([args.uic, str(source)], check=True,
                                stdout=subprocess.PIPE, encoding='utf-8')
        headers.append(result.stdout)
output = Path(args.output)
output.parent.mkdir(parents=True, exist_ok=True)
content = '\n'.join(headers)
if not output.exists() or output.read_text(encoding='utf-8') != content:
    output.write_text(content, encoding='utf-8')
