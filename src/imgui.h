#pragma once
#include <cplug_extensions/window.h>

struct imgui_widget
{
    float x, y, r, b;
};

typedef struct imgui_context
{
    bool  mouse_left_down;
    float mouse_x, mouse_y;
} imgui_context;

bool imgui_hittest(imgui_context* ctx, struct imgui_widget* widget)
{
    return ctx->mouse_x >= widget->x && ctx->mouse_y >= widget->y && ctx->mouse_x <= widget->r &&
           ctx->mouse_y <= widget->b;
}

bool imgui_button(imgui_context* ctx, struct imgui_widget* widget)
{
    return ctx->mouse_left_down && imgui_hittest(ctx, widget);
}

void imgui_end_frame(imgui_context* ctx) { ctx->mouse_left_down = false; }

void imgui_send_event(imgui_context* ctx, const PWEvent* e)
{
    if (e->type == PW_EVENT_MOUSE_LEFT_DOWN)
    {
        ctx->mouse_left_down = true;
        ctx->mouse_x         = e->mouse.x;
        ctx->mouse_y         = e->mouse.y;
    }
}