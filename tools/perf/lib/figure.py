#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

"""figure.py -- generate figure-related products (EXPERIMENTAL)"""

import json
import os.path
from textwrap import wrap
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter

from lib.common import json_from_file, str2key
from lib.flat import make_flat, process_fstrings

# XXX allow not breaking long strings
SKIP_NO_AXIS_MSG = "SKIP: Axis '{}' is not provided by the series of id={}. " \
    "Available keys are: {}"

class Figure:
    """A single figure"""

    _figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 6}}

    def _series_file(self, result_dir):
        return os.path.join(result_dir, self.file + '.json')

    def __init__(self, f, result_dir=""):
        self.output = f['output']
        self.output['done'] = self.output.get('done', False)
        # copies for convenience
        self.title = self.output['title']
        self.file = self.output['file']
        self.argx = self.output['x']
        self.argy = self.output['y']
        self.key = self.output['key']
        self.xscale = self.output.get('xscale', 'log')
        self.result_dir = result_dir
        # find the latest series
        if not self.output['done']:
            self.series = f['series']
        else:
            data = json_from_file(self._series_file(result_dir))
            self.series = data['json'][self.key]['series']

    def __eq__(self, other):
        """A comparison function"""
        if self.output != other.output:
            return False
        if self.result_dir != other.result_dir:
            return False
        if self.series != other.series:
            return False
        return True

    def cache(self):
        """Cache the current state of execution"""
        return {'output': self.output, 'series': self.series}

    def is_done(self):
        """Are all steps completed?"""
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
            output['rw_order'] = \
                'rand' if oneseries['rw'] in ['randread', 'randwrite'] \
                else 'seq'
        if 'x' in oneseries.keys():
            output['x_key'] = str2key(oneseries['x'])
        if 'y' in oneseries.keys():
            output['y_key'] = str2key(oneseries['y'])
        return output

    @staticmethod
    def escape(string):
        return string.replace('_', '\\_')

    @classmethod
    def flatten(cls, figures):
        """Flatten the figures list"""
        figures = make_flat(figures, cls.get_figure_desc)
        figures = process_fstrings(figures, cls.get_figure_desc,
                                   cls.oneseries_derivatives)
        output = []
        for figure in figures:
            # flatten series
            common = figure.get('series_common', {})
            figure['series'] = make_flat(
                figure['series'], cls.get_oneseries_desc, common)
            figure['series'] = process_fstrings(
                figure['series'], cls.get_oneseries_desc,
                cls.oneseries_derivatives)
            output.append(cls(figure))
        return output

    def prepare_series(self, result_dir):
        """
        Extract all series from the respective benchmark files and append them
        to the series file.
        """
        output = {}
        output['title'] = self.title
        output['x'] = self.argx
        output['y'] = self.argy
        output['series'] = []
        for series in self.series:
            idfile = os.path.join(result_dir,
                                  'benchmark_' + str(series['id']) + '.json')
            rows = json_from_file(idfile)['json']
            # If it is dict() it indicates a mix workload consisting of
            # two parts: 'read' and 'write'. In this case, the series
            # has to provide 'rw_dir' to pick one of them.
            if isinstance(rows, dict):
                rw_dir = series['rw_dir']
                rows = rows[rw_dir]
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
            output['series'].append(
                {'label': series['label'], 'points': points})
        # save the series to a file
        series_path = self._series_file(result_dir)
        if os.path.exists(series_path):
            figures = json_from_file(series_path)['json']
        else:
            figures = {}
        figures[self.key] = output
        with open(series_path, 'w', encoding="utf-8") as file:
            json.dump(figures, file, indent=4)
        # mark as done
        self.output['done'] = True

    def _points_to_xy(self, points):
        xslist = [p[0] for p in points]
        yslist = [p[1] for p in points]
        return xslist, yslist

    def _label(self, column, with_better=False):
        """Translate the name of a column to a label with a unit"""
        label_by_column = {
            'threads': '# of threads',
            'iodepth': 'iodepth',
            'bs': 'block size [B]',
            'lat_avg': 'latency [usec]',
            'lat_pctl_99.9': 'latency [usec]',
            'lat_pctl_99.99': 'latency [usec]',
            'bw_avg': 'bandwidth [Gb/s]',
            'cpuload': 'CPU load [%]'
        }
        lower = 'lower is better'
        higher = 'higher is better'
        better_by_column = {
            'lat_avg': lower,
            'lat_pctl_99.9': lower,
            'lat_pctl_99.99': lower,
            'bw_avg': higher
        }
        # If the column is not in the dictionary
        # the default return value is the raw name of the column.
        output = label_by_column.get(column, column)
        if with_better:
            output += '\n(' + better_by_column.get(column, column) + ')'
        return output

    def png_path(self):
        """get a path to the output PNG file"""
        output = self.file + '_' + self.key + '.png'
        return os.path.join('.', output)

    def to_png(self, include_title):
        """generate an output PNG file"""
        # set output file size, padding and title
        fig = plt.figure(**Figure._figure_kwargs)
        if include_title:
            suptitle = "\n".join(wrap(self.title, 60))
            fig.suptitle(suptitle, fontsize='medium', y=0.90)
        # get a subplot
        plot = plt.subplot(1, 1, 1)
        # XXX bw_avg [threads=24, iodepth=2, block size=4096B]
        plot.title.set_text('')
        xticks = []
        for oneseries in self.series:
            # draw series ony-by-one
            xslist, yslist = self._points_to_xy(oneseries['points'])
            plot.plot(xslist, yslist, marker='.', label=oneseries['label'])
            # collect all existing x values
            xticks.extend(xslist)
        # make values unique (set) and sort them
        xticks = sorted(list(set(xticks)))
        # set the x-axis scale
        if self.xscale == "linear":
            plot.set_xscale(self.xscale)
        else:
            plot.set_xscale(self.xscale, base=2)
            plot.xaxis.set_major_formatter(ScalarFormatter())

        plot.set_xticks(xticks)
        plt.setp(plot.get_xticklabels(), rotation=45, ha='right')
        plot.set_xlabel(self._label(self.argx))
        plot.set_ylabel(self._label(self.argy, with_better=True))
        plot.set_ylim(bottom=0)
        plot.legend(fontsize=6)
        plot.grid(True)

        os.chdir(self.result_dir)
        plt.savefig(self.png_path())

    def html_data_table(self):
        """
        Create an HTML snippet string with a table containing the Figure data.
        """
        # header
        html = "<table><thead><tr><th></th>"
        for point in self.series[0]['points']:
            html += "<th>" + str(point[0]) + "</th>"
        html += "</tr></thead><tbody>"

        # rows
        for oneseries in self.series:
            # Since the output is processed as markdown,
            # special characters have to be escaped.
            html += "<tr><td>" + Figure.escape(oneseries['label']) + "</td>"
            for point in oneseries['points']:
                html += "<td>" + str(point[1]) + "</td>"
            html += "</tr>"

        # end the table
        html += "</tbody></table>"
        return html

    def to_html(self, figno):
        """Combine a Figure's png and data table into a single HTML snippet"""
        html = "<h4>Figure {}. {}</h4>".format(figno, Figure.escape(self.title))
        html += '<img src="' + self.png_path() + '" alt="' + self.title + '"/>'
        html += self.html_data_table()
        return html
