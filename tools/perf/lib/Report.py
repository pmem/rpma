#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Report.py -- a report object (EXPERIMENTAL)
#

import os

from .Part import *

class Report:
    """A report object"""

    def _load_parts(self, loader, env, bench):
        self.parts = []
        # XXX add an ordered list of parts to the Bench and bench.json
        for part in bench.parts:
            self.parts.append(Part(loader, env, part))

    def _load_figures(self, bench):
        self.figures = {}
        for f in bench.figures:
            html = f.to_html(bench.result_dir)
            # add to 2-level figure dictionary
            if f.file not in self.figures.keys():
                self.figures[f.file] = {}
            self.figures[f.file][f.key] = html

    def __init__(self, loader, env, bench):
        self.env = env # jinja2.Environment
        self.result_dir = bench.result_dir
        self._load_parts(loader, env, bench)
        self._load_figures(bench)
        self.config = bench.config

    def _create_menu(self):
        return "".join([part.menu() for part in self.parts])

    def _create_content(self):
        return "".join([part.content(self.figures) for part in self.parts])

    def _create_header(self):
        # XXX both *_header.md files can be integrated directly into
        # the layout.html file so this step won't be necessary
        tmpl = self.env.get_template('report_header.md')
        md = tmpl.render(self.config['report'])
        html = markdown2.markdown(md)
        return html

    def create(self, output):
        """Generate a report and write it to the output file"""
        variables = {}
        variables['menu'] = self._create_menu()
        variables['header'] = self._create_header()
        variables['content'] = self._create_content()
        # render the report with the complete layout
        layout_tmpl = self.env.get_template('layout.html')
        html = layout_tmpl.render(variables)
        # write the output file
        output_file = os.path.join(self.result_dir, output + '.html')
        with open(output_file, 'w') as f:
            f.write(html)
