#!/usr/bin/python3
# Copyright (C) Caterpillar Inc. All Rights Reserved.


import argparse, os, sys, shutil, textwrap
import xml.etree.ElementTree as ET
from subprocess import call, PIPE
from pathlib import Path
import shutil

__version__ = '1.0.0'
__script_dir__ = os.path.dirname(os.path.realpath(__file__))

main_parser = argparse.ArgumentParser(prog='print_units', description='version ' + __version__)
main_parser.add_argument('--file', '-f', help='[Required] Path to UNITS file.', dest='xml_file', type=os.path.abspath, metavar='[UNIT FILE]', required=True)

filename = main_parser.parse_args(sys.argv[1:]).xml_file

tree = ET.parse(filename)
root = tree.getroot()

metric_names = list()
english_names = list()

for child in root:
    metric = child.attrib.get("SIName").upper().replace('/','_').replace('-','_').replace('(','_').replace(')','').replace('#','').replace('%','PERCENT').replace('[HH:MM:SS]', 'TIME_FORMAT')
    metric_names.append(metric)
    english = child.attrib.get("ISName").upper().replace('/','_').replace('-','_').replace('(','_').replace(')','').replace('#','').replace('%','PERCENT').replace('[HH:MM:SS]', 'TIME_FORMAT')
    english_names.append(english)
    #print(f'metric: {metric} english: {english}')

metric_names = dict.fromkeys(metric_names)
english_names = dict.fromkeys(english_names)
common_names = list()
metric_alpha_names = list()
english_alpha_names = list()
final_names = list()

count = 0
for x in metric_names:
    #val = f'UNIT_METRIC_{x}'
    #print(val)
    common=False
    for y in english_names:
        #val = f'UNIT_ENGLISH_{x}'
        if x == y:
            common_names.append(f'UNIT_{x}')
            common=True
        else:
            english_alpha_names.append(f'UNIT_ENGLISH_{y}')
    if not common:
        metric_alpha_names.append(f'UNIT_METRIC_{x}')

final_names.append('UNIT_NONE')
final_names = final_names + metric_alpha_names + english_alpha_names
final_names = dict.fromkeys(final_names)

#final_names.pop('UNIT_')
final_names.pop('UNIT_ENGLISH_')
#final_names.pop('UNIT_METRIC_')

for x in final_names:
    print(f'{x} = {count};')
    count+=1
#print(english_names)
