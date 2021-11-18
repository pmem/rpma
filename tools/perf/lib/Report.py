#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Report.py -- a report object (EXPERIMENTAL)
#

import os
import shutil

from .Part import *
from lib.common import escape

class Report:
    """A report object"""

    TESTING_DATE = 'Testing Date'

    @staticmethod
    def _preprocess_config_table(table, vars):
        """set type and append the testing date to a configuration table"""
        # the only correct type is 'kvtable'
        table['type'] = 'kvtable'
        table[Report.TESTING_DATE] = vars['test_date']

    def _set_variables(self, vars):
        input_file = vars['input_file']
        vars = vars['json']
        missing = None

        if 'authors' in vars:
            vars['authors'] = "\n".join(
                ['- ' + author for author in vars['authors']])

        if 'ref' in vars:
            vars['ref'] = escape(vars['ref'])

        # XXX type validation is missing
        if 'configuration' not in vars:
            missing = "['configuration']"
        else:
            if 'common' not in vars['configuration']:
                missing = "['configuration']['common']"
            elif 'target' not in vars['configuration']:
                missing = "['configuration']['target']"
            elif 'bios' not in vars['configuration']:
                missing = "['configuration']['bios']"
            else:
                if 'settings' not in vars['configuration']['bios']:
                    missing = "['configuration']['bios']['settings']"
                elif 'excerpt' not in vars['configuration']['bios']:
                    missing = "['configuration']['bios']['excerpt']"

        if missing is not None:
            raise SyntaxError("{} misses {} entry".format(input_file, missing))

        # escape markdown special characters
        if 'security' in vars['configuration']:
            vars['configuration']['security'] = escape(vars['configuration']['security'])
        if 'description' in vars['configuration']:
            vars['configuration']['description'] = escape(vars['configuration']['description'])

        for table in [vars['configuration']['common'],
                      vars['configuration']['target']['details'],
                      vars['configuration']['bios']['settings'],
                      vars['configuration']['bios']['excerpt']]:
            Report._preprocess_config_table(table, vars)

        self.variables = vars

    def _load_parts(self, loader, env, bench):
        self.parts = []
        if not bench.config.get('compare', False):
            # copy schematic
            shutil.copy(self.variables['configuration']['schematic'],
                self.result_dir)
            self.variables['configuration']['schematic'] = \
                os.path.basename(self.variables['configuration']['schematic'])
            # prepare preamble part
            preamble = Part(loader, env, 'preamble')
            # XXX Part.process_variables_level() should be a function
            # not a method this way self.variables may be processed
            # in Report._set_variables() where it belongs.
            preamble.process_variables_level(self.variables, {})
            preamble.set_variables(self.variables)
            self.parts.append(preamble)
        for partname in bench.parts:
            part = Part(loader, env, partname)
            part.set_variables({'figure': self.figures})
            self.parts.append(part)
        footer = Part(loader, env, 'footer')
        footer.set_variables(self.variables)
        self.parts.append(footer)

    def _load_figures(self, bench):
        self.figures = {}
        for f in bench.figures:
            # add to 2-level figure dictionary
            if f.file not in self.figures.keys():
                self.figures[f.file] = {}
            self.figures[f.file][f.key] = f

    # a global figure counter needed for numbering figures within the report
    figno = 0

    def _add_filters(self, env):
        """add custom filters to the environment"""
        def figure_filter(figure):
            Report.figno += 1
            return figure.to_html(Report.figno)
        env.filters['figure'] = figure_filter

    def __init__(self, loader, env, bench, vars):
        self._add_filters(env)
        self.env = env # jinja2.Environment
        self._set_variables(vars)
        self.result_dir = bench.result_dir
        self._load_figures(bench)
        self._load_parts(loader, env, bench)

    def _create_menu(self):
        return "".join([part.menu() for part in self.parts])

    def _create_content(self):
        return "".join([part.content() for part in self.parts])

    def _create_header(self):
        # XXX both *_header.md files can be integrated directly into
        # the layout.html file so this step won't be necessary
        tmpl = self.env.get_template('report_header.md')
        md = tmpl.render(self.variables)
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
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(html)
