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
import shutil

from jinja2 import Environment, FileSystemLoader, Template

PREREQUISITE = 'Before using this script generate report figures using ./create_report_figures.sh'
SEARCHPATH = 'templates'

# all used SEARCHPATH/*.md files
CONTENTS = ['audience', 'authors', 'bios', 'configuration_common', \
    'configuration_target', 'header', 'report', 'security', 'introduction', \
    'tc_read_lat_config', 'tc_read_bw_config', 'tc1_read', \
    'tc_write_lat_config', 'tc_write_bw_config', 'tc2_write', \
    'tc_mix_lat_config', 'tc_mix_bw_config', 'tc3_mix' ]

# all variables on a per-content basis
CONTENTS_VARIABLES = { \
    'header': [ 'release'], \
    'introduction': [ 'hl_ext'], \
    'tc1_read': [ 'tc_read_lat_config', 'tc_read_bw_config' ], \
    'tc2_write': [ 'tc_write_lat_config', 'tc_write_bw_config' ], \
    'tc3_mix': [ 'tc_mix_lat_config', 'tc_mix_bw_config' ], \
    'report': [ \
        'test_date', 'audience', 'authors', 'bios', \
        'configuration_common', 'configuration_target', 'security', \
        'introduction', 'tc1_read', 'tc2_write', 'tc3_mix' ] \
}

def is_a_file(parser, arg):
    if not os.path.exists(arg):
        parser.error(f"The file {arg} does not exist!")
    else:
        return arg

def main():
    parser = argparse.ArgumentParser( \
        formatter_class=argparse.RawDescriptionHelpFormatter, \
        description='Generate a performance report (EXPERIMENTAL) \n\n{}'.format(PREREQUISITE))
    parser.add_argument('--report_dir', metavar='DIR', required=True, \
        help='a figures directory')
    parser.add_argument('--release', required=True, help='e.g. 0.00')
    parser.add_argument('--test_date', required=True, help='e.g. December 2020')
    parser.add_argument('--high_level_setup_figure', required=True, \
        help='e.g. Figure_0.png', type=lambda x: is_a_file(parser, x))
    args = vars(parser.parse_args())

    contents = {}

    # copy the high-level setup figure
    _, ext = os.path.splitext(args['high_level_setup_figure'])
    hl_figure = os.path.join(args['report_dir'], f"Figure_0{ext}")
    shutil.copyfile(args['high_level_setup_figure'], hl_figure)
    contents['hl_ext'] = ext

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
