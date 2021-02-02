#!/usr/bin/env python3
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2021, Intel Corporation
#

#
# create_report.py -- generate the performance report and appendices
#                     (EXPERIMENTAL)
#

import argparse
import markdown2
import os.path
import shutil

from jinja2 import Environment, FileSystemLoader, Template

PREREQUISITE = 'Before using this script generate report figures using ./create_report_figures.sh'
SEARCHPATH = 'templates'

# all used SEARCHPATH/*.md files
CONTENTS = [
     \
    'audience', 'authors', 'bios', 'configuration_common', \
    'configuration_target', 'header', 'report', 'security', 'introduction', \
    'tc_read_lat_config', 'tc_read_bw_config', 'tc1_read', 'tc2_write', \
    'tc3_mix' ]

# all variables on a per-content basis
CONTENTS_VARIABLES = { \
    'header': [ 'release'], \
    'introduction': [ 'hl_ext'], \
    'tc1_read': [ 'tc_read_lat_config', 'tc_read_bw_config' ], \
    'tc2_write': [ 'tc_config_dummy' ], \
    'tc3_mix': [ 'tc_config_dummy' ], \
    'report': [ \
        'test_date', 'audience', 'authors', 'bios', \
        'configuration_common', 'configuration_target', 'security', \
        'introduction', 'tc1_read', 'tc2_write', 'tc3_mix' ] \
}

# all used SEARCHPATH/*.html files
CONTENTS_HTML = {
    'report_menu'
}

def is_a_file(parser, arg):
    if not os.path.exists(arg):
        parser.error(f"The file {arg} does not exist!")
    else:
        return arg

def report(args):
    contents = {}

    # copy the high-level setup figure
    _, ext = os.path.splitext(args['high_level_setup_figure'])
    hl_figure = os.path.join(args['report_dir'], f"Figure_0{ext}")
    shutil.copyfile(args['high_level_setup_figure'], hl_figure)
    contents['hl_ext'] = ext

    # load the HTML content parts
    env = Environment(loader=FileSystemLoader(SEARCHPATH))
    for name in CONTENTS_HTML:
        tmpl = env.get_template(f'{name}.html')
        contents[name] = tmpl.render()

    # convert content parts from markdown to HTML
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

    # attach the correct menu for the report
    contents['menu'] = contents['report_menu']

    # render the report with the complete layout
    layout_tmpl = env.get_template('layout.html')
    report = layout_tmpl.render(contents)

    # prepare output file and write the result
    output_file = os.path.join(args['report_dir'], 'report.html')
    with open(output_file, 'w') as f:
        f.write(report)

def appendices(args):
    print("XXX")

def main():
    # common arguments
    parser = argparse.ArgumentParser( \
        formatter_class=argparse.RawDescriptionHelpFormatter, \
        description=( \
            'Generate the performance report and appendices (EXPERIMENTAL) ' \
            f'\n\n{PREREQUISITE}'))
    parser.add_argument('--report_dir', metavar='DIR', required=True, \
        help='a figures directory')
    parser.add_argument('--release', required=True, help='e.g. 0.00')

    subparsers = parser.add_subparsers(dest='command')

    # report-specifc arguments
    parser_r = subparsers.add_parser('report', help='generate the report')
    parser_r.add_argument('--test_date', required=True, help='e.g. December 2020')
    parser_r.add_argument('--high_level_setup_figure', required=True, \
        help='e.g. Figure_0.png', type=lambda x: is_a_file(parser, x))

    # appendices-specifc arguments
    parser_a = subparsers.add_parser('appendices', help='generate the appendices')

    # parse the command line
    args = vars(parser.parse_args())

    # call a command-specific function by its name
    globals()[args['command']](args)

if __name__ == "__main__":
    main()
