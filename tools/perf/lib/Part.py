#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Part.py -- a single part object (EXPERIMENTAL)
#

import json
import markdown2

from .common import *
from .kvtable import *

class Part:
    """A single report object"""

    def _process_variables_level(self, variables, common):
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
            # replace templates
            if isinstance(variables[k], str):
                variables[k] = variables[k].format(**common)

    def _load_variables(self, loader):
        source, _, _ = loader.get_source(self.env,
            'part_' + self.name + '.json')
        variables = json.loads(source)

        # preprocess variables['common']
        # - concat lines "\n" (lines2str)
        common = variables.pop('common', {})
        for var, txt in common.items():
            common[var] = lines2str(txt)

        # generate HTML tables dict2kvtable()
        # - take into account the variables['common']
        self._process_variables_level(variables, common)

        self.variables = variables

    def _load_template(self):
        self.md_template = self.env.get_template(f'part_{self.name}.md')
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
        self._load_variables(loader)
        self._load_template()

    def _render(self, variables):
        """
        Render the part:
        1. jinja2 markdown template
        2. markdown
        3. HTML
        """
        md = self.md_template.render(variables)
        html = markdown2.markdown(md)
        return html

    def menu(self):
        """Render the part's menu"""
        variables = dict(**self.variables, **{'menu': True})
        return self._render(variables)

    def content(self, figures):
        """Render the part's content"""
        variables = dict(**self.variables, **{'figure': figures})
        return self._render(variables)
