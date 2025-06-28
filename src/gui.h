#pragma once
#include "imgui.h"
#include "plugin.h"
#include <nanovg.h>
#include <sokol_gfx.h>

#include "libs/texteditor.h"

typedef struct
{
    float   x, y;
    int16_t u, v;
} vertex_t;

typedef struct GUI
{
    Plugin*     plugin;
    void*       pw;
    void*       sg;
    NVGcontext* nvg;
    int         font_id;
    float       scale;

    TextEditor texteditor;

    struct imgui_context imgui;

    float input_gain_peaks_slow[2];
    float input_gain_peaks_fast[2];

    sg_pipeline knob_pip;
    sg_buffer   knob_vbo;
    sg_buffer   knob_ibo;

    // TODO: fix whatever is wrong with NanoVG sokol so we can use that for drawing the logo...
    sg_pipeline logo_pip;
    sg_buffer   logo_vbo;
    sg_buffer   logo_ibo;
    sg_image    logo_img;
    sg_sampler  logo_smp;

    // int         logo_img_id;
    int logo_img_width;
    int logo_img_height;

    uint64_t frame_start_time;
    uint64_t frame_end_time;
} GUI;

// Nanovg helpers
#define NVG_ALIGN_TL (NVG_ALIGN_TOP | NVG_ALIGN_LEFT)
#define NVG_ALIGN_TC (NVG_ALIGN_TOP | NVG_ALIGN_CENTER)
#define NVG_ALIGN_TR (NVG_ALIGN_TOP | NVG_ALIGN_RIGHT)

#define NVG_ALIGN_CL (NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT)
#define NVG_ALIGN_CC (NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER)
#define NVG_ALIGN_CR (NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT)

#define NVG_ALIGN_BL (NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT)
#define NVG_ALIGN_BC (NVG_ALIGN_BOTTOM | NVG_ALIGN_CENTER)
#define NVG_ALIGN_BR (NVG_ALIGN_BOTTOM | NVG_ALIGN_RIGHT)

// clang-format off
// Fake english to real english helpers
typedef struct NVGcolor NVGcolour;
static inline void nvgFillColour(NVGcontext* ctx, NVGcolour col) { nvgFillColor(ctx, col); }
static inline void nvgStrokeColour(NVGcontext* ctx, NVGcolour col) { nvgStrokeColor(ctx, col); }
#define nvgHexColour(hex) (NVGcolour){( hex >> 24)         / 255.0f,\
                                     ((hex >> 16) & 0xff) / 255.0f,\
                                     ((hex >>  8) & 0xff) / 255.0f,\
                                     ( hex        & 0xff) / 255.0f}
// clang-format on

static const NVGcolour COLOUR_TEXT = nvgHexColour(0x858C94FF);

static const NVGcolour COLOUR_BG_LIGHT = nvgHexColour(0xC9D3DDFF);
static const NVGcolour COLOUR_BG_DARK  = nvgHexColour(0x151B32FF);

static const NVGcolour COLOUR_GREY_1 = nvgHexColour(0xB5BEC7FF);
static const NVGcolour COLOUR_GREY_2 = nvgHexColour(0x636A78FF);