#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py
#

"""a report object (EXPERIMENTAL)"""

import os
import shutil
from copy import deepcopy
import markdown2

from .part import Part
from ..common import escape

__TESTING_DATE = 'Testing Date'

def __preprocess_config_table(table, variables):
    """set type and append the testing date to a configuration table"""
    # the only correct type is 'kvtable'
    table['type'] = 'kvtable'
    table[__TESTING_DATE] = variables['test_date']

def preprocess_vars(variables):
    """XXX"""
    input_file = variables['input_file']
    variables = variables['json']
    missing = None

    if 'authors' in variables:
        variables['authors'] = "\n".join(
            ['- ' + author for author in variables['authors']])

    if 'ref' in variables:
        variables['ref'] = escape(variables['ref'])

    # XXX type validation is missing
    if 'configuration' not in variables:
        missing = "['configuration']"
    else:
        if 'common' not in variables['configuration']:
            missing = "['configuration']['common']"
        elif 'target' not in variables['configuration']:
            missing = "['configuration']['target']"
        elif 'bios' not in variables['configuration']:
            missing = "['configuration']['bios']"
        else:
            if 'settings' not in variables['configuration']['bios']:
                missing = "['configuration']['bios']['settings']"
            elif 'excerpt' not in variables['configuration']['bios']:
                missing = "['configuration']['bios']['excerpt']"

    if missing is not None:
        raise SyntaxError("{} misses {} entry".format(input_file, missing))

    # escape markdown special characters
    if 'security' in variables['configuration']:
        variables['configuration']['security'] = \
            escape(variables['configuration']['security'])
    if 'description' in variables['configuration']:
        variables['configuration']['description'] = \
            escape(variables['configuration']['description'])

    for table in [variables['configuration']['common'],
                  variables['configuration']['target']['details'],
                  variables['configuration']['bios']['settings'],
                  variables['configuration']['bios']['excerpt']]:
        __preprocess_config_table(table, variables)
    return variables

class Report:
    """A report object"""

    def __load_parts(self, env, bench):
        parts = []
        if not bench.config.get('compare', False):
            # copy schematic
            shutil.copy(self.__variables['configuration']['schematic'],
                        self.__result_dir)
            self.__variables['configuration']['schematic'] = \
                os.path.basename(self.__variables['configuration']['schematic'])
            # prepare preamble part
            preamble = Part(env, 'preamble')
            preamble.variables = self.variables
            parts.append(preamble)
        for partname in bench.parts:
            part = Part(env, partname)
            part.variables = {'figure': self.figures}
            parts.append(part)
        footer = Part(env, 'footer')
        footer.variables = self.variables
        parts.append(footer)
        return parts

    def __load_figures(self, bench):
        figures = {}
        for figure in bench.figures:
            # add to 2-level figure dictionary
            if figure.file not in figures.keys():
                figures[figure.file] = {}
            figures[figure.file][figure.key] = figure
        return figures

    # a global figure counter needed for numbering figures within the report
    figno = 0

    def __add_filters(self):
        """add custom filters to the environment"""
        def figure_filter(figure):
            """XXX"""
            Report.figno += 1
            return figure.to_html(Report.figno)
        self.__env.filters['figure'] = figure_filter

    def __init__(self, env, bench, variables):
        self.__env = env # jinja2.Environment
        self.__add_filters()
        self.__variables = preprocess_vars(variables)
        self.__result_dir = bench.result_dir
        self.__figures = self.__load_figures(bench)
        self.__parts = self.__load_parts(env, bench)

    @property
    def variables(self):
        """XXX"""
        return deepcopy(self.__variables)

    @property
    def figures(self):
        """XXX"""
        return deepcopy(self.__figures)

    def __create_menu(self):
        return "".join([part.menu() for part in self.__parts])

    def __create_content(self):
        return "".join([part.content() for part in self.__parts])

    def __create_header(self):
        # XXX both *_header.md files can be integrated directly into
        # the layout.html file so this step won't be necessary
        tmpl = self.__env.get_template('report_header.md')
        markdown = tmpl.render(self.variables)
        html = markdown2.markdown(markdown)
        return html

    def create(self, output):
        """Generate a report and write it to the output file"""
        variables = {}
        variables['menu'] = self.__create_menu()
        variables['header'] = self.__create_header()
        variables['content'] = self.__create_content()
        # render the report with the complete layout
        layout_tmpl = self.__env.get_template('layout.html')
        html = layout_tmpl.render(variables)
        # write the output file
        output_file = os.path.join(self.__result_dir, output + '.html')
        with open(output_file, 'w', encoding='utf-8') as file:
            file.write(html)
