#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022, Intel Corporation
#

import sys
from io import StringIO

filename = sys.argv[1]
file = open(sys.argv[1], 'r')

oldcontent = file.readlines()
newcontent = ""

frontmatter = False

newcontent = StringIO()

for line in oldcontent:
	if not frontmatter and line.startswith("---"):
		newcontent.write("---")
		newcontent.write(
"""
draft: false
layout: "library"
slider_enable: true
description: ""
disclaimer: "The contents of this web site and the associated <a href=\\"https://github.com/pmem\\">GitHub repositories</a> are BSD-licensed open source."
"""
)
		newcontent.write("aliases: [\"" + filename.split("/")[-1].rstrip(".md") + ".html\"]\n")
		frontmatter = True
	elif frontmatter:
		if line.startswith("...") or line.startswith("---"):
			newcontent.write("---\n")
			newcontent.write("{{< manpages >}}\n")
			frontmatter = False
		if line.startswith("date: "):
				lines = line.split(": ")
				newcontent.write("header: \"" + lines[1].rstrip()+"\"\n")
		if line.startswith("collection: "):
				lines = line.split(": ")
				newcontent.write("title: \"" + lines[1].rstrip() + " | PMDK\"\n")
	else:
		newcontent.write(line)

file.close()

file = open(sys.argv[1], 'w')
file.write(newcontent.getvalue())

file.close()
