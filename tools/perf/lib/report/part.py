#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Part.py
#

"""a single part object (EXPERIMENTAL)"""

from copy import deepcopy
import json

import jinja2
import markdown2

from .variable import lines2str, process_level

class Part:
    """A single report object"""

    def __load_constants(self) -> dict:
        try:
            source, _, _ = \
                self.__env.loader.get_source(self.__env,
                                             'part_{}.json'.format(self.__name))
        except jinja2.exceptions.TemplateNotFound:
            return {}
        constants = json.loads(source)
        # preprocess commons
        common = constants.pop('common', {})
        for var, txt in common.items():
            common[var] = lines2str(txt)
        # generate HTML tables dict2kvtable()
        process_level(constants, common)
        return constants

    def __init__(self, env: jinja2.Environment, name: str):
        """
        Args:
            env: allows loading and render templates
            name: a name of the part
        """
        self.__env = env
        self.__name = name
        self.__constants = self.__load_constants()
        self.__variables = {}
        self.md_template = \
            self.__env.get_template('part_{}.md'.format(self.__name))

    @property
    def variables(self):
        """XXX"""
        return deepcopy(self.__variables)

    @variables.setter
    def variables(self, value):
        process_level(value, {})
        self.__variables = value

    @property
    def constants(self):
        """XXX"""
        return deepcopy(self.__constants)

    def __render(self, variables, md_to_html=True):
        """
        Render the part:
        1. jinja2 markdown template
        2. markdown
        3. HTML
        """
        markdown = self.md_template.render(variables)
        if md_to_html:
            html = markdown2.markdown(markdown)
            return html
        return markdown

    def menu(self):
        """Render the part's menu"""
        variables = dict(**self.__constants, **self.variables, **{'menu': True})
        return self.__render(variables, md_to_html=False)

    def content(self):
        """Render the part's content"""
        variables = dict(**self.__constants, **self.variables)
        return self.__render(variables)
