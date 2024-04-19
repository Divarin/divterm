#!/bin/bash
/home/tom/git/cc65/bin/cc65 -t c64 $1.c
/home/tom/git/cc65/bin/cl65 -t c64 $1.s -o $1.prg
