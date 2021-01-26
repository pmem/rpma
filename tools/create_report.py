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

# all used SEARCHPATH/*.md files
CONTENTS = ['audience', 'authors', 'bios', 'configuration_common', \
    'configuration_target', 'header', 'report', 'security']

# all variables on a per-content basis
CONTENTS_VARIABLES = { \
    'header': [ \
        'release'], \
    'report': [ \
        'test_date', 'audience', 'authors', 'bios', \
        'configuration_common', 'configuration_target', 'security'] \
}

def main():
    parser = argparse.ArgumentParser( \
        formatter_class=argparse.RawDescriptionHelpFormatter, \
        description='Generate a performance report (EXPERIMENTAL) \n\n{}'.format(PREREQUISITE))
    parser.add_argument('--report_dir', metavar='DIR', required=True, \
        help='a figures directory')
    parser.add_argument('--release', required=True, help='e.g. 0.00')
    parser.add_argument('--test_date', required=True, help='e.g. December 2020')
    args = vars(parser.parse_args())

    # convert content parts from markdown to HTML
    contents = {}
    for name in CONTENTS:
        if not os.path.isfile(f'{SEARCHPATH}/{name}.md'):
            contents[name] = f'<i>no {SEARCHPATH}/{name}.md</i><br/>'
            continue
        contents[name] = markdown2.markdown_path(f'{SEARCHPATH}/{name}.md', \
                extras=['tables'])

    # replace the variables with appropriate values
    for name, var_names in CONTENTS_VARIABLES.items():
        tmpl = Template(contents[name])
        # get variable values from either args or contents
        tmpl_vars = {k: args.get(k, contents.get(k, 'no data')) for k in var_names}
        contents[name] = tmpl.render(tmpl_vars)

    # render the report with the complete layout
    env = Environment(loader=FileSystemLoader(SEARCHPATH))
    layout_tmpl = env.get_template('layout.html')
    report = layout_tmpl.render(contents)

    # prepare output file and write the result
    output_file = os.path.join(args['report_dir'], 'report.html')
    with open(output_file, 'w') as f:
        f.write(report)

if __name__ == "__main__":
    main()
