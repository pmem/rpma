#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Figure.py -- generate figure-related products (EXPERIMENTAL)
#

import matplotlib.pyplot as plt
import os.path

from textwrap import wrap
from .common import *
from .flat import *
from .Benchmark import *
from matplotlib.ticker import ScalarFormatter

class Figure:

    _figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 6}}

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
        try:
            self.xscale = self.output['xscale']
        except KeyError:
            self.xscale = 'log'
        # find the latest series
        if not self.output['done']:
            self.series = f['series']
        else:
            data = json_from_file(self._series_file(result_dir))
            self.series = data['json'][self.key]['series']

    def cache(self):
        """Cache the current state of execution"""
        return {'output': self.output, 'series': self.series}

    def is_done(self):
        return self.output['done']

    @staticmethod
    def get_figure_desc(figure):
        """Getter for accessing the core descriptor of a figure"""
        return figure['output']

    @staticmethod
    def get_oneseries_desc(oneseries):
        """Getter for accessing the core descriptor of a series"""
        return oneseries

    @staticmethod
    def oneseries_derivatives(oneseries):
        """Generate all derived variables of a series"""
        output = {}
        if 'rw' in oneseries.keys():
            output['rw_order'] = 'rand' if oneseries['rw'] else 'seq'
        return output

    @classmethod
    def flatten(cls, figures):
        """Flatten the figures list"""
        figures = make_flat(figures, cls.get_figure_desc)
        figures = process_fstrings(figures, cls.get_figure_desc)
        output = []
        for f in figures:
            # flatten series
            common = f.get('series_common', {})
            f['series'] = make_flat(f['series'], cls.get_oneseries_desc, common)
            f['series'] = process_fstrings(f['series'], cls.get_oneseries_desc,
                cls.oneseries_derivatives)
            output.append(cls(f))
        return output

    def prepare_series(self, result_dir):
        """
        Extract all series from the respective benchmark files and append them
        to the series file.
        """
        output = {}
        output['title'] = self.title
        output['x'] = self.x
        output['y'] = self.y
        output['series'] = []
        for series in self.series:
            idfile = os.path.join(result_dir, 'benchmark_' + str(series['id']) +
                '.json')
            rows = json_from_file(idfile)['json']
            # it is assumed each row has the same names of columns
            keys = rows[0].keys()
            # skip the series if it does not have required keys
            if self.x not in keys or self.y not in keys:
                continue
            points = [[row[self.x], row[self.y]] for row in rows]
            output['series'].append({'label': series['label'], 'points': points})
        # save the series to a file
        series_path = self._series_file(result_dir)
        if os.path.exists(series_path):
            figures = json_from_file(series_path)['json']
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

    def _label(self, column):
        """Translate the name of a column to a label with a unit"""
        label_by_column={
            'threads': '# of threads',
            'iodepth': 'iodepth',
            'bs': 'block size [B]',
            'lat_avg': 'latency [usec]',
            'lat_pctl_99.9': 'latency [usec]',
            'lat_pctl_99.99': 'latency [usec]',
            'bw': 'bandwidth [Gb/s]',
            'cpuload': 'CPU load [%]'
        }
        # If the column is not in the dictionary the default return value is the raw name of the column.
        return label_by_column.get(column, column)

    def png_path(self, result_dir):
        output = self.file + '_' + self.key + '.png'
        return os.path.join(result_dir, output)

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
        # set the x-axis scale
        if self.xscale == "linear":
            ax.set_xscale(self.xscale)
        else:
            ax.set_xscale(self.xscale, base=2)
            ax.xaxis.set_major_formatter(ScalarFormatter())

        ax.set_xticks(xticks)
        plt.setp(ax.get_xticklabels(), rotation=45, ha='right')
        ax.set_xlabel(self._label(self.x))
        ax.set_ylabel(self._label(self.y))
        ax.set_ylim(bottom=0)
        ax.legend(fontsize=6)
        ax.grid(True)

        plt.savefig(self.png_path(result_dir))

    def html_data_table(self):
        """
        Create an HTML snippet string with a table containing the Figure data.
        """
        # header
        html = "<table><tr><th></th>"
        for point in self.series[0].points:
            html += "<th>" + point[0] + "</th>"
        html += "</tr>"

        # rows
        for oneseries in self.series:
            html += "<tr><td>" + oneseries.label + "</td>"
            for point in oneseries.points:
                html += "<td>" + point[1] + "</td>"
            html += "</tr>"

        # end the table
        html += "</table>"
        return html

    def to_html(self, result_dir):
        """Combine a Figure's png and data table into a single HTML snippet"""
        html = '<img src="' + self.png_path(result_dir) + '" alt="' + self.title + '"/>'
        html += self.html_data_table()
        return html
