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

class Figure:

    __figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 6}}

    def __init__(self, output, series, result_dir):
        self.title = output['title']
        self.file = output['file']
        self.x = output['x']
        self.y = output['y']
        self.key = output['key']
        self.series = series
        self.result_dir = result_dir

    def __xticks(self):
        # XXX
        return []

    def __label(self, arg):
        # XXX
        return arg

    def __legend(self):
        # XXX
        return ""

    def generate_png(self):
        # set output file size, padding and title
        fig = plt.figure(**Figure.__figure_kwargs)
        suptitle = "\n".join(wrap(self.title, 60))
        fig.suptitle(suptitle, fontsize='medium', y=0.90)
        # get a subplot
        ax = plt.subplot(1, 1, 1)
        # XXX bw_avg [threads=24, iodepth=2, block size=4096B]
        ax.title.set_text('')
        # XXX linear / log
        ax.set_xscale('linear')
        ax.set_xticks(self.__xticks())
        ax.set_xlabel(self.__label(self.x))
        ax.set_ylabel(self.__label(self.y))
        ax.set_ylim(bottom=0)
        ax.legend(self.__legend(), fontsize=6)
        ax.grid(True)

        output = self.file + '_' + self.key + '.png'
        plt.savefig(os.path.join(self.result_dir, output))

    def generate_html(self):
        # XXX
        pass
