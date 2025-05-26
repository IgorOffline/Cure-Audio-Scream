"""
Quick and dirty SVG to NanoVG converter

Prints to screen a C like function for drawing SVG paths

Usage: python {script.py} path/to/file.svg

"""

import sys
from pathlib import Path
from svgpathtools import svg2paths, CubicBezier, QuadraticBezier, Line


def create_function(filename: str):
    paths, attributes = svg2paths(filename)

    p = Path(filename)
    name = p.parts[-1].replace('.', '_').replace(' ', '')

    print(
        'void draw_%s(NVGcontext* nvg, const float scale, float x, float y) {' % name)
    print('// clang-format off')
    for path in paths:
        print('nvgBeginPath(nvg);')
        M = path[0].start
        print('nvgMoveTo(nvg, x + scale * %ff, y + scale * %ff);' %
              (M.real, M.imag))
        for seg in path:
            if (isinstance(seg, CubicBezier)):
                a2 = 'x + scale * %ff' % seg.control1.real
                a3 = 'y + scale * %ff' % seg.control1.imag
                a4 = 'x + scale * %ff' % seg.control2.real
                a5 = 'y + scale * %ff' % seg.control2.imag
                a6 = 'x + scale * %ff' % seg.end.real
                a7 = 'y + scale * %ff' % seg.end.imag
                print('nvgBezierTo(nvg, %s, %s, %s, %s, %s, %s);' %
                      (a2, a3, a4, a5, a6, a7))
            elif (isinstance(seg, QuadraticBezier)):
                a2 = 'x + scale * %ff' % seg.control.real
                a3 = 'y + scale * %ff' % seg.control.imag
                a4 = 'x + scale * %ff' % seg.end.real
                a5 = 'y + scale * %ff' % seg.end.imag
                print('nvgQuadTo(nvg, %s, %s, %s, %s);' % (a2, a3, a4, a5))
            elif (isinstance(seg, Line)):
                a2 = 'x + scale * %ff' % seg.end.real
                a3 = 'y + scale * %ff' % seg.end.imag
                print('nvgLineTo(nvg, %s, %s);' % (a2, a3))
        print('nvgClosePath(nvg);')
        print('nvgFill(nvg);')
    print('// clang-format on')
    print('}')


if __name__ == '__main__':
    create_function(sys.argv[1])
    sys.stdout.flush()
    exit(0)
