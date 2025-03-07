#!/usr/bin/env bash

cmake -B build
cmake --build build

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.
mkdir -p output
# build/PA1 testcases/quadlight.txt output/quadlight.bmp
# build/PA1 testcases/bboxtest.txt output/bboxtest.bmp
# build/PA1 testcases/test.txt output/test11.bmp
# build/PA1 testcases/Curve.txt output/Curve.bmp
# build/PA1 testcases/glossy.txt output/glossy0.bmp
# build/PA1 testcases/fieldDepth.txt output/fieldDepth.bmp
# build/PA1 testcases/move.txt output/move.bmp
# build/PA1 testcases/scene02_cube.txt output/scene02.bmp
# build/PA1 testcases/scene03_sphere.txt output/scene03.bmp
# build/PA1 testcases/scene04_axes.txt output/scene04.bmp
# build/PA1 testcases/scene05_bunny_200.txt output/scene05.bmp
# build/PA1 testcases/scene06_bunny_1k.txt output/scene06.bmp
# build/PA1 testcases/scene07_shine.txt output/scene07.bmp
build/PA1 testcases/A_curve.txt output/A_curve.bmp
build/PA1 testcases/A_rabbit.txt output/A_rabbit.bmp
build/PA1 testcases/A_fieldDepth.txt output/A_fieldDepth.bmp
build/PA1 testcases/A_fa.txt output/A_fa.bmp
build/PA1 testcases/A_move.txt output/A_move.bmp
build/PA1 testcases/A_fa.txt output/A_fa.bmp
# build/PA1 testcases/A_normal.txt output/A_normal11.bmp