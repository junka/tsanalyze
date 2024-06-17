#!/usr/bin/env python

import tsana
import sys


def analyze(args):
    tsana.init(args)
    tsana.run()
    tsana.result()
    tsana.deinit()


def main():
    analyze(sys.argv)

if __name__ == "__main__":
    main()
