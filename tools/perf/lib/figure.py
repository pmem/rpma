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

from lib.common import json_from_file, str2key, escape
from lib.flat import make_flat, process_fstrings

# XXX allow not breaking long strings
SKIP_NO_AXIS_MSG = "SKIP: Axis '{}' is not provided by the series of id={}. " \
    "Available keys are: {}"

class Figure:
    """A single figure"""

    _figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 1}}

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
        self.series_in = f['series']
        if self.output['done']:
            data = json_from_file(self._series_file(result_dir))
            self.series = data['json'][self.key]['series']
            self.common_params = data['json'][self.key]['common_params']

    def __eq__(self, other):
        """A comparison function"""
        if self.output != other.output:
            return False
        if self.result_dir != other.result_dir:
            return False
        if self.series_in != other.series_in:
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

    # a list of possible common params
    COMMON_PARAMS = {
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
            params = {key: rows[0].get(key, value['default'])
                      for key, value in Figure.COMMON_PARAMS.items()}
        for row in rows:
            params = {key: value for key, value in params.items()
                      if value == row.get(key,
                                          Figure.COMMON_PARAMS[key]['default'])}
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
        output['common_params'] = []
        output['series'] = []
        common = None
        for series in self.series_in:
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
            common = Figure._get_common_params(common, rows)
            output['series'].append(
                {'label': series['label'], 'points': points})
        output['common_params'] = common
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
        self.series = output['series']
        self.common_params = output['common_params']

    def _points_to_xy(self, points):
        xslist = [p[0] for p in points]
        yslist = [p[1] for p in points]
        return xslist, yslist

    def _get_xcommon(self):
        """generate an ordered list of common x-values"""
        xlist = [p[0] for oneseries in self.series for p in oneseries['points']]
        return sorted(list(set(xlist)))

    @staticmethod
    def _points_to_dict(points):
        """transform list of [x, y] into a dict() where {x: y}"""
        return {p[0]: p[1] for p in points}

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
        plot.title.set_text('[{}]'.format(
            ', '.join(['{}={}'. \
            format(key, Figure.COMMON_PARAMS[key]['format'].format(value))
                       for key, value in self.common_params.items()])))
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
        plt.close(fig)

    def html_data_table(self):
        """
        Create an HTML snippet string with a table containing the Figure data.
        """
        # header
        xcommon = self._get_xcommon()
        html = '<table class="data"><thead><tr><th></th><th>{}</th></tr>' \
            '</thead><tbody>'. \
            format('</th><th>'.join([str(x) for x in xcommon]))
        # rows
        for oneseries in self.series:
            # Since the output is processed as markdown,
            # special characters have to be escaped.
            html += "<tr><td>" + escape(oneseries['label']) + "</td>"
            points = Figure._points_to_dict(oneseries['points'])
            points = {k: '{0:.2f}'.format(v) for k, v in points.items()}
            for xarg in xcommon:
                html += '<td>{}</td>'.format(points.get(xarg, '-'))
            html += "</tr>"

        # end the table
        html += "</tbody></table>"
        return html

    def to_html(self, figno):
        """Combine a Figure's png and data table into a single HTML snippet"""
        html = "<h4 class='figure'>Figure {}. {}</h4>". \
            format(figno, escape(self.title))
        html += '<img src="' + self.png_path() + '" alt="' + self.title + '"/>'
        html += self.html_data_table()
        return html
