#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Figure.py -- generating a figure related products (EXPERIMENTAL)
#

import matplotlib.pyplot as plt
import os.path

from textwrap import wrap
from .common import *

class Figure:

    _figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 6}}

    def _hash(self, oneseries):
        """Calculate and write the hash of a single series"""
        if 'hash' in oneseries.keys():
            return
        # XXX series require automatic hashing
        oneseries['hash'] = 'XXX'

    def _series_file(self, result_dir):
        return os.path.join(result_dir, self.file + '.json')

    def __init__(self, f, result_dir = ""):
        self.output = f['output']
        self.output['done'] = self.output.get('done', False)
        # copies for convenience
        self.title = self.output['title']
        self.file = self.output['file']
        self.x = self.output['x']
        self.y = self.output['y']
        self.key = self.output['key']
        # find the latest series
        if not self.output['done']:
            for oneseries in f['series']:
                self._hash(oneseries)
            self.series = f['series']
        else:
            self.series = json_from_file(self._series_file(result_dir)) \
                [self.key]['series']

    def cache(self):
        return {'output': self.output, 'series': self.series}

    def is_done(self):
        return self.output['done']

    @classmethod
    def flatten(cls, figures):
        """Flatten the descriptors list to a list of figures"""
        # - Prepare an empty set B == all benchmarks to be run
        # - Loop over all figure files provided
        #     - Loop over all figures in each of the files
        #         - Loop over all possible combinations of layout and arg_axis
        #             - Generate output_title and output_file (if required)
        #             - Generate filter (if required)
        #             - Loop over all data points defined for the figure
        #                 - Create all possible combinations of the data point keys
        #                 - Loop over all data combinations
        #                     - Populate the B set
        output = []
        for f in figures:
            output.append(cls(f))
        return output

    def prepare_series(self, result_dir):
        """Extract all series from the respective hash files and store"""
        output = {}
        output['title'] = self.title
        output['x'] = self.x
        output['y'] = self.y
        output['series'] = []
        for series in self.series:
            hash = os.path.join(result_dir, series['hash'] + '.json')
            rows = json_from_file(hash)
            points = [[row[self.x], row[self.y]] for row in rows]
            output['series'].append({'label': series['label'], 'points': points})
        # save the series to a file
        series_path = self._series_file(result_dir)
        if os.path.exists(series_path):
            figures = json_from_file(series_path)
        else:
            figures = {}
        figures[self.key] = output
        with open(series_path, 'w') as file:
            json.dump(figures, file, indent=4)
        # mark as done
        self.output['done'] = True

    def _points_to_xy(self, points):
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        return xs, ys

    def _xticks(self):
        # XXX
        return []

    def _label(self, arg):
        # XXX
        return arg

    def _legend(self):
        # XXX
        return ""

    def to_png(self, result_dir):
        # set output file size, padding and title
        fig = plt.figure(**Figure._figure_kwargs)
        suptitle = "\n".join(wrap(self.title, 60))
        fig.suptitle(suptitle, fontsize='medium', y=0.90)
        # get a subplot
        ax = plt.subplot(1, 1, 1)
        # XXX bw_avg [threads=24, iodepth=2, block size=4096B]
        ax.title.set_text('')
        xticks = []
        for oneseries in self.series:
            # draw series ony-by-one
            xs, ys = self._points_to_xy(oneseries['points'])
            ax.plot(xs, ys, marker='.', label=oneseries['label'])
            # collect all existing x values
            xticks.extend(xs)
        # make values unique (set) and sort them
        xticks = sorted(list(set(xticks)))
        # XXX linear / log
        ax.set_xscale('linear')
        ax.set_xticks(xticks)
        plt.setp(ax.get_xticklabels(), rotation=45, ha='right')
        ax.set_xlabel(self._label(self.x))
        ax.set_ylabel(self._label(self.y))
        ax.set_ylim(bottom=0)
        ax.legend(fontsize=6)
        ax.grid(True)

        output = self.file + '_' + self.key + '.png'
        plt.savefig(os.path.join(result_dir, output))

    def to_html(self, result_dir):
        # XXX
        pass
