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

# all *.md files used for the report
REPORT_CONTENTS_MD = [ \
    'report_header', \
    'audience', 'authors', 'bios', 'configuration_common', \
    'configuration_target', 'disclaimer', 'report', 'security', \
    'introduction', \
    'tc_read_lat_config', 'tc_read_bw_config', 'tc1_read', \
    'tc_write_lat_config', 'tc_write_bw_config', 'tc2_write', \
    'tc_mix_lat_config', 'tc_mix_bw_config', 'tc3_mix', \
    'tc_cpu_lat_config', 'tc_cpu_bw_config', 'tc4_cpu' ]

# all *.html files used for the report
REPORT_CONTENTS_HTML = [
    'report_menu'
]

# all *.md files used for the appendices
APPENDICES_CONTENTS_MD = [ \
    'appendices_header', 'appendices' ]

# all *.html files used for the appendices
APPENDICES_CONTENTS_HTML = [
    'appendices_menu'
]

# all variables on a per-content basis
CONTENTS_VARIABLES = { \
    'report_header': [ 'release'], \
    'introduction': [ 'hl_ext'], \
    'tc1_read': [ 'tc_read_lat_config', 'tc_read_bw_config' ], \
    'tc2_write': [ 'tc_write_lat_config', 'tc_write_bw_config' ], \
    'tc3_mix': [ 'tc_mix_lat_config', 'tc_mix_bw_config' ], \
    'tc4_cpu': [ 'tc_cpu_lat_config', 'tc_cpu_bw_config' ], \
    'report_menu': [ 'cpu' ], \
    'report': [ \
        'test_date', 'ref', 'audience', 'disclaimer', 'authors', 'bios', \
        'configuration_common', 'configuration_target', 'security', \
        'introduction', 'tc1_read', 'tc2_write', 'tc3_mix', \
        'cpu', 'tc4_cpu' ], \
    'appendices_header': [ 'release'], \
    'appendices': [ 'ref' ] \
}

# defaults
VARIABLES_DEFAULTS = {
    'cpu': False
}

def is_a_file(parser, arg):
    if not os.path.exists(arg):
        parser.error(f"The file {arg} does not exist!")
    else:
        return arg

def populate_contents(contents, parts_html, parts_md, args):
    # load the HTML content parts
    for name in parts_html:
        tmpl = args['env'].get_template(f'{name}.html')
        tmpl_vars = {}
        if name in CONTENTS_VARIABLES.keys():
            var_names = CONTENTS_VARIABLES[name]
            # get variable values from either args or defaults
            tmpl_vars = {k: args.get(k, VARIABLES_DEFAULTS.get(k, 'no data')) \
                for k in var_names}
        contents[name] = tmpl.render(tmpl_vars)

    # convert content parts from markdown to HTML
    for name in parts_md:
        # possible paths
        paths = [ \
            f'{args["report_dir"]}/{name}.md', \
            f'{SEARCHPATH}/{name}.md' ]
        # lookup
        for path in paths:
            if not os.path.isfile(path):
                continue
            # the file is found
            contents[name] = markdown2.markdown_path(path, extras=['tables'])
        # if the file is not found
        if name not in contents.keys():
            contents[name] = f'<i>no {name}.md</i><br/>'

    # replace the variables with appropriate values
    for name, var_names in CONTENTS_VARIABLES.items():
        # if name is not in contents there is nothing to render
        # if name is in parts_html it was already rendered
        if name not in contents.keys() or name in parts_html:
            continue
        tmpl = Template(contents[name])
        # get variable values from either args or contents or defaults
        tmpl_vars = {k: args.get(k, contents.get(k, \
            VARIABLES_DEFAULTS.get(k, 'no data'))) for k in var_names}
        contents[name] = tmpl.render(tmpl_vars)

    return contents

def render(contents, args, output):
    # render the report with the complete layout
    layout_tmpl = args['env'].get_template('layout.html')
    report = layout_tmpl.render(contents)

    # prepare output file and write the result
    output_file = os.path.join(args['report_dir'], f'{output}.html')
    with open(output_file, 'w') as f:
        f.write(report)

def report(args):
    contents = {}

    # copy the high-level setup figure
    _, ext = os.path.splitext(args['high_level_setup_figure'])
    hl_figure = os.path.join(args['report_dir'], f"Figure_000{ext}")
    shutil.copyfile(args['high_level_setup_figure'], hl_figure)
    contents['hl_ext'] = ext

    contents = populate_contents(contents, REPORT_CONTENTS_HTML, \
        REPORT_CONTENTS_MD, args)

    # attach the correct header, menu and content for the report
    contents['header'] = contents['report_header']
    contents['menu'] = contents['report_menu']
    contents['content'] = contents['report']

    render(contents, args, 'report')

def appendices(args):
    contents = populate_contents({}, APPENDICES_CONTENTS_HTML, \
        APPENDICES_CONTENTS_MD, args)

    # attach the correct header, menu and content for the appendices
    contents['header'] = contents['appendices_header']
    contents['menu'] = contents['appendices_menu']
    contents['content'] = contents['appendices']

    render(contents, args, 'appendices')

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
    parser_r.add_argument('--ref', required=True, help='e.g. 2021_04_21_CLX')
    parser_r.add_argument('--high_level_setup_figure', required=True, \
        help='e.g. Figure_0.png', type=lambda x: is_a_file(parser, x))
    parser_r.add_argument('--include_cpu_load', dest='cpu', action='store_true')

    # appendices-specifc arguments
    parser_a = subparsers.add_parser('appendices', help='generate the appendices')
    parser_a.add_argument('--ref', required=True, help='e.g. 2021_04_21_CLX')

    # parse the command line
    args = vars(parser.parse_args())

    # a common env object
    args['env'] = Environment(loader=FileSystemLoader( \
        [SEARCHPATH, args['report_dir']]))

    # call a command-specific function by its name
    globals()[args['command']](args)

if __name__ == "__main__":
    main()
