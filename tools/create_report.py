#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# create_report.py -- generate a performance report (EXPERIMENTAL)
#

import argparse
import markdown2
import os.path

from jinja2 import Environment, FileSystemLoader, Template

PREREQUISITE = 'Before using this script generate report figures using ./create_report_figures.sh'
SEARCHPATH = 'templates'

def main():
    parser = argparse.ArgumentParser( \
        formatter_class=argparse.RawDescriptionHelpFormatter, \
        description='Generate a performance report (EXPERIMENTAL) \n\n{}'.format(PREREQUISITE))
    parser.add_argument('--report_dir', metavar='DIR', required=True, \
        help='a figures directory')
    parser.add_argument('--release', required=True, help='e.g. 0.00')
    args = parser.parse_args()

    # convert the report to HTML
    content_tmpl_in = markdown2.markdown_path(f'{SEARCHPATH}/report.md', \
        extras=['tables'])

    # render the report body
    content_tmpl = Template(content_tmpl_in)
    tmpl_vars = {'release': args.release}
    content = content_tmpl.render(tmpl_vars)

    # render the report with the complete layout
    env = Environment(loader=FileSystemLoader(SEARCHPATH))
    layout_tmpl = env.get_template('layout.html')
    tmpl_vars = {'report': content}
    report = layout_tmpl.render(tmpl_vars)

    # prepare output file and write the result
    output_file = os.path.join(args.report_dir, 'report.html')
    with open(output_file, 'w') as f:
        f.write(report)

if __name__ == "__main__":
    main()
