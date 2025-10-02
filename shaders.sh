#!/bin/sh
sokol-shdc -i src/shaders/knob.glsl -o src/shaders/knob.glsl.h -l metal_macos
sokol-shdc -i src/shaders/lfo.glsl -o src/shaders/lfo.glsl.h -l metal_macos
sokol-shdc -i modules/wip_libs/shaders/nanovg_sokol.glsl -o src/shaders/nanovg_sokol.glsl.h -l metal_macos
sokol-shdc -i modules/wip_libs/shaders/dualfilter.glsl -o src/shaders/dualfilter.glsl.h -l metal_macos