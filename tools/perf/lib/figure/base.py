#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""base.py -- generate figure-related products (EXPERIMENTAL)"""

import json
import os.path
from deepdiff import DeepDiff

from .image import draw_png
from .html import data_table
from ..common import json_from_file, escape

SKIP_NO_AXIS_MSG = """SKIP: Axis '{}' is not provided by the series of id={}. Available keys are: {}"""
SKIP_NO_FILE_MSG = "SKIP: the file does not exist: {}"
SKIP_NO_ROWS_MSG = "SKIP: No row is provided by the series of id={}."

class Figure:
    """A single figure"""

    def __series_file(self, result_dir):
        return os.path.join(result_dir, self.file + '.json')

    def __init__(self, figure, result_dir=""):
        self.output = figure['output']
        self.output['done'] = self.output.get('done', False)
        # copies for convenience
        self.title = self.output['title']
        self.file = self.output['file']
        self.argx = self.output['x']
        self.argy = self.output['y']
        self.yaxis_max = None
        self.key = self.output['key']
        self.xscale = self.output.get('xscale', 'log')
        self.result_dir = result_dir
        self.series_in = figure['series']
        if self.output['done']:
            data = json_from_file(self.__series_file(result_dir))
            self.series = data['json'][self.key]['series']
            self.common_params = data['json'][self.key].get('common_params', {})

    def __eq__(self, other):
        """A comparison function"""
        if DeepDiff(self.output, other.output, exclude_paths=["root['title']"]):
            return False
        if len(self.series_in) != len(other.series_in):
            return False
        # XXX It may happen that a requirement within a figure is not done
        # despite it is 'done' globally. The exclude below is merely a WA.
        if DeepDiff(self.series_in, other.series_in,
                    exclude_regex_paths="['requirements']['done']"):
            return False
        return True

    def cache(self):
        """Cache the current state of execution"""
        return {'output': self.output, 'series': self.series_in}

    def is_done(self):
        """Are all steps completed?"""
        return self.output['done']

    def get_series_in(self):
        """Get a series input list"""
        return self.series_in

    def set_yaxis_max(self, max_y):
        """Set y-axis max"""
        self.yaxis_max = max_y

    # a list of possible common params
    __COMMON_PARAMS = {
        'threads': {
            'default': 1,
            'format': '{}'
        },
        'iodepth': {
            'default': 1,
            'format': '{}'
        },
        'bs': {
            'default': None,
            'format': '{}B'
        },
    }

    @staticmethod
    def _get_common_params(params, rows):
        """lookup common parameters"""
        if params is None:
            # When no initial common parameters dict is provided (params)
            # use values from the first of the rows. If a given key is missing
            # the default value is used instead.
            params = {key: rows[0].get(key, value['default'])
                      for key, value in Figure.__COMMON_PARAMS.items()}
        for row in rows:
            # Look row-by-row and create a subset of parameters that are common
            # to all of the rows. If a given key is missing in any of the rows
            # the default value is used instead.
            params = {key: value \
                for key, value in params.items() \
                if value == row.get(key, \
                                    Figure.__COMMON_PARAMS[key]['default'])}
        return params

    def prepare_series(self, result_dir=None):
        """
        Extract all series from the respective benchmark files and append them
        to the series file.
        """
        if result_dir is None:
            result_dir = self.result_dir
        output = {}
        output['title'] = self.title
        output['x'] = self.argx
        output['y'] = self.argy
        output['common_params'] = {}
        output['series'] = []
        common = None
        for series in self.series_in:
            idfile = os.path.join(result_dir,
                                  'benchmark_' + str(series['id']) + '.json')
            try:
                rows = json_from_file(idfile)['json']
            except FileNotFoundError:
                print(SKIP_NO_FILE_MSG.format(idfile))
                continue
            # If it is dict() it indicates a mix workload consisting of
            # two parts: 'read' and 'write'. In this case, the series
            # has to provide 'rw_dir' to pick one of them.
            if isinstance(rows, dict):
                rw_dir = series['rw_dir']
                rows = rows[rw_dir]
            if not rows:
                print(SKIP_NO_ROWS_MSG.format(series['id']))
                continue
            # it is assumed each row has the same names of columns
            keys = rows[0].keys()
            # skip the series if it does not have required keys
            if self.argx not in keys:
                print(SKIP_NO_AXIS_MSG.format(
                    self.argx, series['id'], str(keys)))
                continue
            if self.argy not in keys:
                print(SKIP_NO_AXIS_MSG.format(
                    self.argy, series['id'], str(keys)))
                continue
            points = [[row[self.argx], row[self.argy]] for row in rows]
            common = Figure._get_common_params(common, rows)
            output['series'].append(
                {'label': series['label'], 'points': points})
        output['common_params'] = {} if common is None else common
        # save the series to a file
        series_path = self.__series_file(result_dir)
        if os.path.exists(series_path):
            figures = json_from_file(series_path)['json']
        else:
            figures = {}
        figures[self.key] = output
        with open(series_path, 'w', encoding='utf-8') as file:
            json.dump(figures, file, indent=4)
        # mark as done
        self.output['done'] = True
        self.series = output['series']
        self.common_params = output['common_params']

    @property
    def xcommon(self):
        """generate an ordered list of common x-values"""
        xlist = [p[0] for oneseries in self.series for p in oneseries['points']]
        return sorted(list(set(xlist)))

    def png_path(self):
        """get a path to the output PNG file"""
        output = self.file + '_' + self.key + '.png'
        return os.path.join('.', output)

    def to_png(self, include_title):
        """generate an output PNG file"""
        os.chdir(self.result_dir)
        suptitle = self.title if include_title else None
        title = '[{}]'.format(', '.join(['{}={}'. \
            format(key, Figure.__COMMON_PARAMS[key]['format'].format(value))
                                         for key, value in
                                         self.common_params.items()]))
        draw_png(self.argx, self.argy, self.series, self.xscale,
                 self.png_path(), self.yaxis_max, suptitle, title)

    def to_html(self, figno):
        """Combine a Figure's png and data table into a single HTML snippet"""
        html = "<h4 class='figure'>Figure {}. {}</h4>". \
            format(figno, escape(self.title))
        html += '<img src="' + self.png_path() + '" alt="' + self.title + '"/>'
        html += data_table(self.xcommon, self.series)
        return html
