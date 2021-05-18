#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# Figure.py -- generating a figure related products (EXPERIMENTAL)
#

import matplotlib.pyplot as plt

class Figure:

    __figure_kwargs = {'figsize': [6.4, 4.8], 'dpi': 200, \
        'tight_layout': {'pad': 6}}

    def __init__(self, figure):
        self.output_title = figure.output_title
        self.output_file = figure.output_file
        self.x = figure.x
        self.y = figure.y
        self.data = figure.data

    def __xticks(self):
        # XXX
        return []

    def __label(self):
        # XXX
        return ""

    def __legend(self):
        # XXX
        return ""

    def generate_png(self):
        # set output file size, padding and title
        fig = plt.figure(**Figure.__figure_kwargs)
        suptitle = "\n".join(wrap(self.output_title, 60))
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

        output = self.output_file + '_' + self.key + ".png"
        plt.savefig(output)

    def generate_html(self):
        # XXX
        pass
