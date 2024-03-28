#!/usr/bin/env python3
import sys
import argparse
from hydra.gen import load, functions, datasection, dis86

def run():
    parser = argparse.ArgumentParser(description='Generate annotation tables')
    parser.add_argument('source', type=str, help='input source annotations file')
    parser.add_argument('--output-path', type=str, help='path to write the generated output')
    parser.add_argument('--functions-hdr', action='store_true', help='generate functions')
    parser.add_argument('--datasection-hdr', action='store_true', help='generate datasection')
    parser.add_argument('--dis86-conf', action='store_true', help='generate dis86 configuration file')

    args = parser.parse_args()

    data = load.annotations(args.source)

    if args.output_path and args.output_path != '-':
        with open(args.output_path, 'w') as f:
            generate(args, data, f)
    else:
        generate(args, data, sys.stdout)

def generate(args, data, out):
    if args.functions_hdr:
        functions.gen_hdr(data['functions'], out)

    elif args.datasection_hdr:
        datasection.gen_hdr(data['data_section'], out)

    elif args.dis86_conf:
        dis86.gen_conf(data, out)

run()
