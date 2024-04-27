#!/bin/bash
/home/tom/git/cc65/bin/cc65 -t c128 $1.c
/home/tom/git/cc65/bin/cl65 -t c128 $1.s -o $1.prg
