#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# base.py
#

"""controlling a single figure (EXPERIMENTAL)"""

import json
import os.path
from copy import deepcopy
from deepdiff import DeepDiff

from .image import draw_png
from .html import data_table
from ..common import json_from_file, escape
from ..benchmark import get_result_path

SKIP_NO_AXIS_MSG = """SKIP: Axis '{}' is not provided by the series of id={}. Available keys are: {}"""
SKIP_NO_FILE_MSG = "SKIP: the file does not exist: {}"
SKIP_NO_ROWS_MSG = "SKIP: No row is provided by the series of id={}."

class Figure:
    """A single figure"""

    def __series_file(self, result_dir):
        return os.path.join(result_dir, self.file + '.json')

    def __init__(self, figure, result_dir):
        self.__output = figure['output']
        self.__output['done'] = self.__output.get('done', False)
        self.__yaxis_max = None
        self.__result_dir = result_dir
        self.__series = figure['series']
        if self.__output['done']:
            data = json_from_file(self.__series_file(result_dir))
            self.__results = data['json'][self.key]['series']
            self.__common_params = data['json'][self.key].get('common_params',
                                                              {})
        else:
            self.__results = None
            self.__common_params = None

    @property
    def title(self) -> str:
        """A title of the figure"""
        return self.__output['title']

    @property
    def file(self)-> str:
        """A file name (without extension) where the results of benchmarking the related data series is stored"""
        return self.__output['file']

    @property
    def key(self) -> str:
        """A key within the `lib.figure.base.Figure.file` under which the results of benchmarking the related data series is stored"""
        return self.__output['key']

    @property
    def argx(self) -> str:
        """An x-axis argument"""
        return self.__output['x']

    @property
    def argy(self) -> str:
        """An y-axis argument"""
        return self.__output['y']

    @property
    def xscale(self) -> str:
        """A x-axis scale. Either linear or log. Log by default."""
        return self.__output.get('xscale', 'log')

    @property
    def output(self) -> dict:
        """(a copy of) the whole `figure['output']` block of figure's
        definition.

        **Note** it is flattened. Please see `lib.figure.flat.flatten()` for
        details.
        """
        return deepcopy(self.__output)

    @property
    def results(self) -> dict:
        """(a copy of) the collected result. Please see `lib.figure.base.Figure.collect_results()` for details."""
        return deepcopy(self.__results)

    @property
    def series(self) -> dict:
        """(a copy of) the whole `figure['series']` block of figure's
        definition.

        **Note** it is flattened. Please see `lib.figure.flat.flatten()` for
        details.
        """
        return deepcopy(self.__series)

    @property
    def yaxis_max(self) -> int:
        """A maximum value of the y-axis"""
        return self.__yaxis_max

    @yaxis_max.setter
    def yaxis_max(self, value: int):
        self.__yaxis_max = value

    def set_series_identifier(self, series_index: int, identifier: int) -> None:
        """Set an identifier of the given series

        Args:
            series_index: an index in the `lib.figure.base.Figure.series` list.
            identifier: a unique identifier allowing to refer to the results
            of executing a particual `lib.benchmark.base.Benchmark`.
        """
        self.__series[series_index]['id'] = identifier

    def __eq__(self, other):
        if DeepDiff(self.output, other.output, exclude_paths=["root['title']"]):
            return False
        if len(self.series) != len(other.series):
            return False
        # XXX It may happen that a requirement within a figure is not done
        # despite it is 'done' globally. The exclude below is merely a WA.
        if DeepDiff(self.series, other.series,
                    exclude_regex_paths="['requirements']['done']"):
            return False
        return True

    def cache(self) -> dict:
        """generate a dict representing the current state of the object

        A cache is a dictionary with the following keys:

        - `cache['output']` which stores the `Figure.output` property
        - `cache['series']` which stores the `Figure.series` property

        **Note** this method does not create a separate cache file. It is used
        by the `lib.bench.Bench.cache()` method in order to cache the whole
        state of the execution into a single JSON file.

        Returns:
            A `dict` being a compilation of the current state of the figure.
        """
        return {'output': self.__output, 'series': self.__series}

    def is_done(self) -> bool:
        """Have the results for the figure been collected?

        Returns:
            `True` when all the results have been already collected. `False`
            otherwise.
        """
        return self.__output['done']

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
    def __get_common_params_from_rows(params: dict, rows: list) -> dict:
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

    def collect_results(self) -> None:
        """Collect all results ordered by the figure

        When all ordered bencharks are done (please see
        `lib.benchmark.base.Benchmark` for details) the figure has to collect
        all results belonging to its series. At the beginning of the process,
        each of the respective series has been assigned an identifier
        (`lib.benchmark.base.Benchmark.uniq()` and
        `Figure.set_series_identifier()`) which allows to track back
        the ordered benchmark's result file. All of these files are processed
        to extract only this information which is meant to be presented on
        the figure.

        When all the results are collected, it is written down to a JSON file
        (`Figure.file`) under `Figure.key`. The collected results are also
        available as `Figure.results`.
        """
        output = {}
        output['title'] = self.title
        output['x'] = self.argx
        output['y'] = self.argy
        output['common_params'] = {}
        output['series'] = []
        common = None
        for series in self.__series:
            idfile = get_result_path(self.__result_dir, series['id'])
            try:
                rows = json_from_file(idfile)['json']
            except FileNotFoundError:
                print(SKIP_NO_FILE_MSG.format(idfile))
                continue
            if not rows:
                print(SKIP_NO_ROWS_MSG.format(series['id']))
                continue
            # 'rows' can be:
            # - a list of dicts or
            # - a list of lists of two dicts
            if isinstance(rows[0], list):
                # rows[0] has to be a dict, but in case of fio results
                # it is a list of two dicts, so we have to take only
                # the proper one
                if series['rw_dir'] == 'read':
                    rw_dir = 0
                else:
                    rw_dir = 1
                for i, _ in enumerate(rows):
                    rows[i] = rows[i][rw_dir]
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
            common = Figure.__get_common_params_from_rows(common, rows)
            output['series'].append(
                {'label': series['label'], 'points': points})
        output['common_params'] = {} if common is None else common
        # save the series to a file
        series_path = self.__series_file(self.__result_dir)
        if os.path.exists(series_path):
            figures = json_from_file(series_path)['json']
        else:
            figures = {}
        figures[self.key] = output
        with open(series_path, 'w', encoding='utf-8') as file:
            json.dump(figures, file, indent=4)
        # mark as done
        self.__output['done'] = True
        self.__results = output['series']
        self.__common_params = output['common_params']

    def __png_path(self) -> str:
        """get a path to the output PNG file"""
        output = self.file + '_' + self.key + '.png'
        return os.path.join('.', output)

    def to_png(self, include_title: bool) -> None:
        """generate an output PNG file

        When `Figure.collect_results()` is done, the collected `Figure.results`
        can be presented as a single PNG file.
        """
        os.chdir(self.__result_dir)
        suptitle = self.title if include_title else None
        title = '[{}]'.format(', '.join(['{}={}'. \
            format(key, Figure.__COMMON_PARAMS[key]['format'].format(value))
                                         for key, value in
                                         self.__common_params.items()]))
        draw_png(self.argx, self.argy, self.__results, self.xscale,
                 self.__png_path(), self.__yaxis_max, suptitle, title)

    def to_html(self, figno: int) -> str:
        """represent the figure as a HTML snippet

        Combine a figure's PNG file (`Figure.to_png()`) and `Figure.results`
        rendered as a HTML table (`lib.figure.html.data_table()`) to create
        a single HTML snippet which can be incorporated into the final
        `lib.report.part.Part` and `lib.report.base.Report`.

        Args:
            figno: an identifier of the figure within the final
              `lib.report.base.Report`.

        Returns:
            A str containing the generated HTML.
        """
        html = "<h4 class='figure'>Figure {}. {}</h4>". \
            format(figno, escape(self.title))
        html += '<img src="' + self.__png_path() + '" alt="' + self.title + \
                '"/>'
        html += data_table(self.__results)
        return html
