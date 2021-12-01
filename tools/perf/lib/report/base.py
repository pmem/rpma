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

from .variable import preprocess_vars
from .part import Part

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

    def __add_filters(self, env):
        """add custom filters to the environment"""
        def figure_filter(figure):
            """XXX"""
            Report.figno += 1
            return figure.to_html(Report.figno)
        env.filters['figure'] = figure_filter

    def __init__(self, env, bench, variables):
        self.__add_filters(env)
        self.__env = env # jinja2.Environment
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
