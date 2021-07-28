#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Part.py -- a single part object (EXPERIMENTAL)
#

import jinja2
import json
import markdown2

from lib.kvtable import lines2str, dict2kvtable

class Part:
    """A single report object"""

    def _process_variables_level(self, variables, common):
        """Process a level of template variables including:
        - concatenating multiline strings
        - replacing common variables
        - replacing kvtable definitions with their representation

        Args:
            variables (dict): a level of template variables
            common (dict): a map of common variables reused across the template
                variables
        """
        for k, v in variables.items():
            if isinstance(v, list):
                variables[k] = lines2str(v)
            elif isinstance(v, dict):
                if 'type' in v:
                    if v["type"] == "kvtable":
                        variables[k] = dict2kvtable(v, common)
                    else:
                        raise ValueError
                else:
                    self._process_variables_level(v, common)
            # replace common
            if isinstance(variables[k], str):
                variables[k] = variables[k].format(**common)

    def _load_constants(self, loader):
        """Populate self.constants with resources required to render
        the template.

        Args:
            loader (jinja2.BaseLoader subclass): allows loading resources
        """
        try:
            source, _, _ = loader.get_source(self.env,
                'part_' + self.name + '.json')
        except jinja2.exceptions.TemplateNotFound:
            self.constants = {}
            return

        constants = json.loads(source)

        # preprocess constants['common']
        # - concat lines "\n" (lines2str)
        common = constants.pop('common', {})
        for var, txt in common.items():
            common[var] = lines2str(txt)

        # generate HTML tables dict2kvtable()
        # - take into account the constants['common']
        self._process_variables_level(constants, common)

        self.constants = constants

    def _load_template(self):
        self.md_template = self.env.get_template('part_{}.md'.format(self.name))
        # XXX maybe a separate function is too much?

    def __init__(self, loader, env, name):
        """
        Args:
            loader (jinja2.BaseLoader subclass): XXX should be obtained from env
            env (jinja2.Environment): allows loading and render templates
            name (str): a name of the part
        """
        self.env = env          # jinja2.Environment
        self.name = name
        self._load_constants(loader)
        self.variables = {}
        self._load_template()

    def set_variables(self, variables):
        self.variables = variables

    def _render(self, variables, md_to_html=True):
        """
        Render the part:
        1. jinja2 markdown template
        2. markdown
        3. HTML
        """
        md = self.md_template.render(variables)
        if md_to_html:
            html = markdown2.markdown(md)
            return html
        else:
            return md

    def menu(self):
        """Render the part's menu"""
        variables = dict(**self.constants,**self.variables, **{'menu': True})
        return self._render(variables, md_to_html=False)

    def content(self):
        """Render the part's content"""
        variables = dict(**self.constants,**self.variables)
        return self._render(variables)
