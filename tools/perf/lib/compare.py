#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""compare.py -- compare multiple benchmarks results (EXPERIMENTAL)"""

from lib.common import json_from_file
from lib.figure import Figure

import json
import os

class Compare:
    """a helper class allowing generating comparisons"""

    def __init__(self, names, benches, result_dir):
        if len(benches) != len(names):
            raise ArithmeticError(
                                  "# of names is not equal to # of benches provided")
        for bench in benches:
            bench.check_completed()
            # XXX make sure data for all figures is already collected
            # XXX we can collect it here if needed
            for figure in bench.figures:
                if not figure.is_done():
                    raise NotImplementedError(
                                            "XXX we have to call figure.prepare_series() first")
        self._benches = {name: bench for name, bench in zip(names, benches)}
        self._result_dir = result_dir

    @staticmethod
    def _figure_id(figure):
        """generate an identifier of the figure"""
        return "{}.{}".format(figure.file, figure.key)

    def prepare_series(self):
        """generate all comparisons required"""
        # track whether a given figure is already done
        done = {}
        # Loop over all benches and figures just in case not all figures are
        # present in all benches.
        for _, bench in self._benches.items():
            for figure in bench.figures:
                if done.get(Compare._figure_id(figure), False):
                    continue
                comparison = Comparison(self, figure)
                comparison.prepare_series()
                comparison.to_pngs()
                done[Compare._figure_id(figure)] = True

    def cache(self):
        """store the comparison to a JSON file"""
        _, bench = list(self._benches.items())[0]
        output = {
            'config': {
                'compare': True},
            'parts': bench.parts,
            'figures': [figure.cache() for figure in bench.figures],
            'requirements': {
                id: r.cache()
                for id, r in bench.requirements.items()}
        }

        output_path = os.path.join(self._result_dir, 'bench.json')
        with open(output_path, 'w', encoding="utf-8") as file:
            json.dump(output, file, indent=4)

class Comparison:
    """a comparison among the same figure present in different benches"""

    def __init__(self, compare, figure):
        self._compare = compare
        self._figure = figure # the sample figure
        self._figures = {}
        # pick all figures matching the sample one
        for name, bench in self._compare._benches.items():
            for figure in bench.figures:
                if figure == self._figure:
                    self._figures[name] = figure
                    break

    # def _prepare_benchlines(self, label):
    #     """generate an actual data comparing results from various sources"""
    #     benchlines = []
    #     for name, figure in self._figures.items():
    #         benchline = {}
    #         benchline['name'] = name
    #         for oneseries in figure.series:
    #             # looking for an actual line
    #             if oneseries['label'] != label:
    #                 continue
    #             # extract the data
    #             benchline['points'] = oneseries['points']
    #             # only a single line should match so the search is done
    #             break
    #         # append the line
    #         benchlines.append(benchline)
    #     return {
    #             'title': self._figure.title,
    #             'label': label,
    #             'x': self._figure.argx,
    #             'y': self._figure.argy,
    #             'xscale': self._figure.xscale,
    #             'benchlines': benchlines}

    def _merge(self):
        benchlines = []
        for name, figure in self._figures.items():
            for oneseries in figure.series:
                benchline = {}
                benchline['label'] = '{} {}'.format(name, oneseries['label'])
                benchline['group'] = name
                # extract the data
                benchline['points'] = oneseries['points']
                # append the line
                benchlines.append(benchline)
        return {
                'title': self._figure.title,
                'x': self._figure.argx,
                'y': self._figure.argy,
                'xscale': self._figure.xscale,
                'series': benchlines}    

    def _series_file(self):
        """generate a JSON file path"""
        return os.path.join(self._compare._result_dir,
                            self._figure.file + '.json')

    def prepare_series(self):
        """prepare JSON files with picked results for the comparison"""
        if os.path.isfile(self._series_file()):
            output = json_from_file(self._series_file())['json']
        else:
            output = {}
        # for oneseries in self._figure.series:
            # keycontent.append(self._prepare_benchlines(oneseries['label']))
        keycontent = self._merge()
        output[self._figure.key] = keycontent
        with open(self._series_file(), 'w', encoding="utf-8") as file:
            json.dump(output, file, indent=4)

    def png_path(self):
        """get a path to the output PNG file"""
        output = self._figure.file + '_' + self._figure.key + '.png'
        return os.path.join('.', output)

    def to_pngs(self):
        """generate all PNG files"""
        os.chdir(self._compare._result_dir)
        data = json_from_file(self._series_file())['json']
        xxx = data.get(self._figure.key)
        # counter = 0
        # for xxx in keycontent:
        output_path = self.png_path()
            # counter += 1
        # XXX quick and dirty
        yaxis_max = 30 if self._figure.key in \
            ["lat_avg", "lat_pctl_999", "lat_pctl_9999"] else None
        Figure.draw_png(xxx['x'], xxx['y'], xxx['series'],
                        xxx['xscale'], output_path, yaxis_max, None,
                        None)
