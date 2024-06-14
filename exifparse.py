#!/usr/bin/env python3
# -*- coding: utf-8 -*
import json
import xml.etree.ElementTree as ET

def FlattenTagToText(tag, delim = ''):
	text = [tag.text]
	if tag.tag == 'br': text += ['\\n']
	for child in tag:
		text += [FlattenTagToText(child, delim)]
		text += [child.tail]
	text += [tag.tail]
	return delim.join([t.strip() if t is not None else '' for t in text])

def ParseExifHtmlTable(filepath):
	exif = {}
	# 对输入的文件的要求：
	# - 提取于 https://exiftool.org/TagNames/EXIF.html 的主要 table
	# - 必须先将所有的 &nbsp 替换为空格
	# - 必须将所有的<br>替换为<br/>
	table = ET.parse(filepath).getroot()
	assert table.tag == 'table'
	tbody = table[0]
	assert tbody.tag == 'tbody'

	# 开始解析 table
	# 第一行为列名
	colnames = []
	first = True
	for tr in tbody:
		if first:
			for th in tr:
				assert th.tag == 'th'
				colnames += [FlattenTagToText(th)]
			first = False
			continue
		rowdata = []
		for td in tr:
			rowdata += [FlattenTagToText(td)]
		item = {}
		for i in range(max(len(colnames), len(rowdata))):
			if i >= len(colnames):
				colname = f'Key{i}'
			else:
				colname = colnames[i]
			item[colname] = rowdata[i].replace('\\n', '\n')
		exif[item['Tag ID']] = item
	return exif

if __name__ == '__main__':
	parsed = ParseExifHtmlTable('exif.xml')
	with open('exif.json', 'w', encoding='utf-8') as f:
		json.dump(parsed, f, indent=4)

	for tid, tdata in parsed.items():
		if '-->' in tdata['Values /Notes']:
			print(tid)
