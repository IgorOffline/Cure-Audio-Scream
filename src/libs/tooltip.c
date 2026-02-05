#include "tooltip.h"
#include <string.h>
#include <xhl/maths.h>

void tooltip_draw(
    Tooltip*     tt,
    XVG*         xvg,
    LinkedArena* arena,
    uint64_t     time_ns,
    float        gui_width,
    float        gui_height,
    float        scale)
{
    LINKED_ARENA_TAGGED_LEAK_DETECT_BEGIN(arena, _param_arena);
    LINKED_ARENA_TAGGED_LEAK_DETECT_BEGIN(xvg->arena, _nvg_arena);

    xassert(tt->state.text != NULL);

    const uint64_t duration_ns         = time_ns - tt->state.time_shown_ns;
    const uint64_t one_second          = 1000000000LLU;
    const bool     should_show_tooltip = duration_ns > one_second;

    if (!should_show_tooltip)
        return;

    xassert(tt->settings.font_size);

    const float TOOLTIP_BOX_HEIGHT     = scale * tt->settings.box_height;
    const float TOOLTIP_ARROW_LENGTH   = scale * tt->settings.arrow_length;
    const float TOOLTIP_ARROW_WIDTH    = scale * tt->settings.arrow_width;
    const float TOOLTIP_TEXT_PADDING_X = scale * tt->settings.text_padding_x;
    const float TOOLTIP_TEXT_PADDING_Y = scale * tt->settings.text_padding_y;
    const float TOOLTIP_GAP            = scale * tt->settings.gap;
    const float FONT_SIZE              = scale * tt->settings.font_size;
    const float STROKE_WIDTH           = 1;

    // nvgSetTextAlign(nvg, NVG_ALIGN_TL);
    // nvgSetFontSize(nvg, FONT_SIZE);
    // nvgSetTextLineHeight(nvg, 1.5);

    // measure dimensions
    float line_height      = 1.5;
    float line_break_width = gui_width * 0.25f;
    line_break_width       = xm_maxf(line_break_width, 500);
    line_break_width       = xm_minf(line_break_width, gui_width);
    const XVGTextLayout* layout =
        xvg_create_text_layout(xvg, tt->state.text, 0, FONT_SIZE, line_break_width, line_height);
    xassert(layout->num_rows >= 1);

    float width  = layout->xmax / xvg->backingScaleFactor;
    width       += TOOLTIP_TEXT_PADDING_X * 2;

    imgui_rect  d             = tt->state.target_widget_dimensions;
    const float widget_cx     = (d.x + d.r) * 0.5f;
    const float widget_top    = d.y;
    const float widget_bottom = d.b;

    float tt_height  = layout->total_height / xvg->backingScaleFactor;
    tt_height       += TOOLTIP_TEXT_PADDING_Y * 2;
    d.x              = widget_cx - width * 0.5f;
    d.r              = d.x + width;
    d.y             -= tt_height + TOOLTIP_ARROW_LENGTH + TOOLTIP_GAP;
    d.b              = d.y + tt_height;

    const float boundary_padding = scale * scale * tt->settings.window_boundary;
    const float boundary_right   = gui_width - boundary_padding;
    const float boundary_bottom  = gui_height - boundary_padding;
    if (d.x < boundary_padding)
    {
        float delta  = boundary_padding - d.x;
        d.x         += delta;
        d.r         += delta;
    }
    if (d.y < boundary_padding)
    {
        float delta  = boundary_padding - d.y;
        d.y         += delta;
        d.b         += delta;

        // Default the tooltip displays above the widget.
        // In the rarer case we run out of room above the widget, display the tooltip below the widget
        if (d.b > widget_top)
        {
            d.y = widget_bottom + TOOLTIP_ARROW_LENGTH + TOOLTIP_GAP;
            d.b = d.y + tt_height;
        }
    }
    if (d.r > boundary_right)
    {
        float delta  = d.r - boundary_right;
        d.x         -= delta;
        d.r         -= delta;
    }
    if (d.b > boundary_bottom)
    {
        float delta  = d.b - boundary_bottom;
        d.y         -= delta;
        d.b         -= delta;
    }

    d.x = floorf(d.x) + STROKE_WIDTH * 0.5f;
    d.y = floorf(d.y) + STROKE_WIDTH * 0.5f;
    d.r = floorf(d.r) - STROKE_WIDTH * 0.5f;
    d.b = floorf(d.b) - STROKE_WIDTH * 0.5f;

    xassert(d.y >= 0);
    xassert(d.b >= d.y);

    float shadow_offset = 2;
    float shadow_blur   = 4;
    xvg_draw_rectangle_with_gradient(
        xvg,
        d.x + shadow_offset - shadow_blur,
        d.y + shadow_offset - shadow_blur,
        d.r - d.x + shadow_blur * 2,
        d.b - d.y + shadow_blur * 2,
        0,
        0,
        xvg_make_shadow(0x0, 0x40, 0, 0, shadow_blur, 0, false));

    // nvgBeginPath(nvg);
    // nvgMoveTo(nvg, d.x, d.y);
    // nvgLineTo(nvg, d.x, d.b);
    // if (d.b < widget_top) // tooltip above widget
    // {
    //     nvgLineTo(nvg, widget_cx - TOOLTIP_ARROW_WIDTH * 0.5f, d.b);
    //     nvgLineTo(nvg, widget_cx, d.b + TOOLTIP_ARROW_LENGTH);
    //     nvgLineTo(nvg, widget_cx + TOOLTIP_ARROW_WIDTH * 0.5f, d.b);
    // }
    // nvgLineTo(nvg, d.r, d.b);
    // nvgLineTo(nvg, d.r, d.y);
    // if (d.y > widget_bottom) // tooltip below widget
    // {
    //     nvgLineTo(nvg, widget_cx + TOOLTIP_ARROW_WIDTH * 0.5f, d.y);
    //     nvgLineTo(nvg, widget_cx, d.y - TOOLTIP_ARROW_LENGTH);
    //     nvgLineTo(nvg, widget_cx - TOOLTIP_ARROW_WIDTH * 0.5f, d.y);
    // }
    // nvgClosePath(nvg);
    unsigned col_tt = tt->settings.colour_bg;
    // unsigned col_tt = 0xff0000ff;
    float offset = TOOLTIP_ARROW_LENGTH * 0.28;
    xvg_draw_rectangle(xvg, d.x, d.y, d.r - d.x, d.b - d.y, 0, 0, col_tt);
    if (d.b < widget_top) // tooltip above widget
    {
        xvg_draw_triangle(xvg, widget_cx, d.b - offset, TOOLTIP_ARROW_WIDTH, TOOLTIP_ARROW_LENGTH, 0.5f, 0, col_tt);
    }
    else // tooltip below widget
    {
        xvg_draw_triangle(
            xvg,
            widget_cx,
            d.y + offset - TOOLTIP_ARROW_LENGTH,
            TOOLTIP_ARROW_WIDTH,
            TOOLTIP_ARROW_LENGTH,
            0.0f,
            0,
            col_tt);
    }

    // nvgSetColour(nvg, tt->settings.colour_bg);
    // nvgFill(nvg);
    // nvgSetColour(nvg, tt->settings.colour_border);
    // nvgStroke(nvg, STROKE_WIDTH);

    // nvgSetColour(nvg, tt->settings.colour_text);

    float text_x = d.x + TOOLTIP_TEXT_PADDING_X;
    float text_y = d.y + TOOLTIP_TEXT_PADDING_Y;
    // float text_y = (d.b + d.y) * 0.5f;
    // text_y += layout->metrics.ascender / nvg->backingScaleFactor;
    // nvgSetTextAlign(nvg, NVG_ALIGN_CL);
    // nvgDrawLayout(nvg, layout, text_x, text_y);
    // nvgReleaseLayout(nvg, layout);
    xvg_draw_text_layout(xvg, layout, text_x, text_y, XVG_ALIGN_CL, tt->settings.colour_text);
    xvg_release_text_layout(xvg, layout);

    LINKED_ARENA_TAGGED_LEAK_DETECT_END(xvg->arena, _nvg_arena);
    LINKED_ARENA_TAGGED_LEAK_DETECT_END(arena, _param_arena);
}
