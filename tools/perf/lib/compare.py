#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# compare.py
#

"""compare multiple benchmarks results (EXPERIMENTAL)

The comparison process generates a set of JSON and PNG files allowing to compare
the results between benchmarks. Benchmark results have to be generated using
`lib.bench.Bench` and have to be complete (`lib.bench.Bench.check_completed()`).
"""

from copy import deepcopy
import json
import os

from lib.common import json_from_file
from lib.figure import draw_png

class Compare:
    """a helper class allowing generating comparisons"""

    def __init__(self, names: list, benches: list, result_dir: str) -> \
        'Compare':
        """Args:
            names: a list of names for `lib.bench.Bench` objects
            benches: a list of `lib.bench.Bench` objects
            result_dir: a directory where the intermediate and final products of
              the comparing process will be stored

        Raises:
            ArithmeticError: If number of `names` is not equal to number of
              `benches` provided.
        """
        if len(benches) != len(names):
            raise ArithmeticError("""# of names is not equal to # of benches provided""")
        for bench in benches:
            bench.check_completed()
        self.__benches = dict(zip(names, benches))
        self.__result_dir = result_dir

    @property
    def benches(self) -> dict:
        """(a copy of) the list of `lib.bench.Bench` objects"""
        return deepcopy(self.__benches)

    @property
    def result_dir(self) -> str:
        """a directory where the intermediate and final products of the comparison process will be stored"""
        return self.__result_dir

    @staticmethod
    def __figure_id(figure):
        """generate the identifier of the figure"""
        return "{}.{}".format(figure.file, figure.key)

    def prepare_series(self):
        """generate all comparisons required (both JSON and PNG files)

        For details please see `Comparison`, `Comparison.prepare_series()` and
        `Comparison.to_pngs()`.
        """
        # track whether a given figure is already done
        done = {}
        # Loop over all benches and figures just in case not all figures are
        # present in all benches.
        for _, bench in self.__benches.items():
            for figure in bench.figures:
                if done.get(Compare.__figure_id(figure), False):
                    continue
                comparison = Comparison(self, figure)
                comparison.prepare_series()
                comparison.to_pngs()
                done[Compare.__figure_id(figure)] = True

    def cache(self):
        """cache the current state of execution to a JSON file

        A cache is a dictionary with the following keys:

        - `cache['config']` states simply `{'compare': True}`
        - `cache['parts']` which stores the `Bench.parts` property
        - `cache['figures']` which stores the `Bench.figures` property
        - `cache['requirements']` which stores the `Bench.requirements` property

        A cache file is always called `bench.json` and it is created in
        the `result_dir` directory.

        XXX maybe it is a good idea to rename `bench.json` as `cache.json`.

        **Note**: In the case of generation a comparison, it does not make much
        sense to cache the state of the object because this process is very
        unlikely to fail and if it will happen it is very cheap to do a retry.
        Having a `bench.json` allows combining the generated PNG files into
        a semi-report. Please see `report_create`.
        """
        # XXX assuming the first benchmark covers all figures of the comparison
        # which may not always be true
        _, bench = list(self.__benches.items())[0]
        output = {
            'config': {
                'compare': True},
            'parts': bench.parts,
            'figures': [figure.cache() for figure in bench.figures],
            'requirements': {
                id: r.cache()
                for id, r in bench.requirements.items()}
        }

        output_path = os.path.join(self.__result_dir, 'bench.json')
        with open(output_path, 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)

class Comparison:
    """a comparison between the same figure present in different benches"""

    def __init__(self, compare: Compare, figure_sample):
        """
        Args:
            compare: the parent `Compare` object
            figure: the sample figure to generate
        """
        self.__compare = compare
        self.__figure = figure_sample # the sample figure
        # pick all figures matching the sample one
        self.__figures = {}
        for name, bench in self.__compare.benches.items():
            for figure in bench.figures:
                if figure == self.__figure:
                    self.__figures[name] = figure
                    break

    def __merge(self):
        """combine series from all figures involved into a single figure"""
        benchlines = []
        for name, figure in self.__figures.items():
            for oneseries in figure.results:
                benchline = {}
                benchline['label'] = '{} {}'.format(name, oneseries['label'])
                benchline['group'] = name
                # extract the data
                benchline['points'] = oneseries['points']
                # append the line
                benchlines.append(benchline)
        return {
            'title': self.__figure.title,
            'x': self.__figure.argx,
            'y': self.__figure.argy,
            'xscale': self.__figure.xscale,
            'series': benchlines}

    def __series_file(self):
        """generate a JSON file path"""
        return os.path.join(self.__compare.result_dir,
                            self.__figure.file + '.json')

    def prepare_series(self) -> None:
        """prepare JSON files with picked results for the comparison

        Loop over all provided `benches` in order to generate all defined
        figures. For a given figure all the `lib.bench.Bench` objects
        are browsed in order to find all occurrences of the figure.
        All occurrences of the figure are compiled into a single figure
        combining all series from all figures prepended with names of
        the respective `lib.bench.Bench` (`names`).

        e.g.

        - Consider a figure composed of three series: A, B and C and
        - the figure exists in two `lib.bench.Bench` instances named: Alfa and
          Beta.
        - The resulting figure will be have the following series:
            - `Alfa A`, `Alfa B`, `Alfa C` and
            - `Beta A`, `Beta B`, `Beta C`.
        """
        if os.path.isfile(self.__series_file()):
            output = json_from_file(self.__series_file())['json']
        else:
            output = {}
        keycontent = self.__merge()
        output[self.__figure.key] = keycontent
        with open(self.__series_file(), 'w', encoding='utf-8') as file:
            json.dump(output, file, indent=4)

    def png_path(self) -> str:
        """get a path to the output PNG file

        Returns:
            The generated path.
        """
        output = self.__figure.file + '_' + self.__figure.key + '.png'
        return os.path.join('.', output)

    def to_pngs(self) -> None:
        """generate all PNG files

        Please see `lib.figure.image.draw_png()`.
        """
        os.chdir(self.__compare.result_dir)
        data = json_from_file(self.__series_file())['json']
        keycontent = data.get(self.__figure.key)
        output_path = self.png_path()
        # XXX add setters to yaxis_max for bw and lat
        draw_png(keycontent['x'], keycontent['y'], keycontent['series'],
                 keycontent['xscale'], output_path, None, None, None)
