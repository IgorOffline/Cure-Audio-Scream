#ifndef GUI_LFO_POINTS_H
#define GUI_LFO_POINTS_H

#include "dsp.h"
#include <imgui.h>
#include <linked_arena.h>
#include <sort.h>
#include <xhl/vector.h>

enum
{
    LFO_POINT_CLICK_RADIUS = 12,
    LFO_POINT_RADIUS       = 4,
    LFO_SKEW_POINT_RADIUS  = 3,
    LFO_SKEW_DRAG_RANGE    = 250,

    LFO_POINT_DRAG_ERASE_DISTANCE = 24,
};

enum ShapeButtonType
{
    SHAPE_POINT,
    SHAPE_FLAT,
    SHAPE_LINEAR_ASC,
    SHAPE_LINEAR_DESC,
    SHAPE_CONVEX_ASC,
    SHAPE_CONCAVE_DESC,
    SHAPE_CONCAVE_ASC,
    SHAPE_CONVEX_DESC,
    SHAPE_COSINE_ASC,
    SHAPE_COSINE_DESC,
    SHAPE_TRIANGLE_UP,
    SHAPE_TRIANGLE_DOWN,
    SHAPE_COUNT,
};

typedef struct GUILFOPoints
{
    void*        pw;    // not owned
    LinkedArena* arena; // not owned

    imgui_rect area;
    // xvec4f area;

    // If false, should copy over the points array from the audio thread
    bool gui_lfo_points_valid;
    // Used to queue changes made to LFO points on the audio thread
    // Coordinates are in beat time, exactly like the lfo
    xvec3f* main_lfo_points;

    // Draggable points (widgets)
    // Cordinates are in window space
    bool    points_valid; // Set to false to copy main_lfo_points > points
    xvec2f* points;
    xvec2f* skew_points;
    // Used as backup while doing non-destructive preview editing of points
    xvec2f* points_copy;
    xvec2f* skew_points_copy;

    // Point multiselect
    xvec2f selection_start;
    xvec2f selection_end;
    int*   selected_point_indexes;
    // Used for hacks to make the current selection & hover work properly when previewing edits to points with the
    // drag-auto-erase feature
    int selected_point_idx;

    xvec2f* lfo_cached_path;
} GUILFOPoints;

typedef struct GUILFOFrameState
{
    bool should_update_cached_path;
    bool should_update_gui_lfo_points_with_points;
    int  pt_hover_idx;
    int  pt_hover_skew_idx;
    int  delete_pt_idx;
} GUILFOFrameState;

static GUILFOFrameState gui_lfo_framestate_new()
{
    GUILFOFrameState framestate  = {0};
    framestate.pt_hover_idx      = -1;
    framestate.pt_hover_skew_idx = -1;
    framestate.delete_pt_idx     = -1;
    return framestate;
}

void gui_lfo_deinit(GUILFOPoints*);

void  gui_lfo_clear_selection(GUILFOPoints* glfo);
void  gui_lfo_insert_point(GUILFOPoints* glfo, xvec2f pos, int idx);
void  gui_lfo_update_point(GUILFOPoints* glfo, xvec2f pos, int idx);
void  gui_lfo_update_skew_point(GUILFOPoints* glfo, int i, float skew);
void  gui_lfo_delete_point(GUILFOPoints* glfo, int idx);
float gui_lfo_calculate_point_skew(GUILFOPoints* glfo, int idx);
void  gui_lfo_save_points_to_copy(GUILFOPoints* glfo);
void  gui_lfo_save_skew_points_to_copy(GUILFOPoints* glfo);
void  gui_lfo_add_to_selection(GUILFOPoints* glfo, int idx);
void  gui_lfo_drag_and_draw(
     GUILFOPoints*              glfo,
     imgui_pt                   pos,
     bool                       snap_to_grid,
     const enum ShapeButtonType shape_type,
     int                        num_grid_x,
     int                        num_grid_y);

void gui_lfo_handle_point_events(
    GUILFOPoints*     glfo,
    GUILFOFrameState* fstate,
    imgui_context*    im,
    int               num_grid_x,
    int               num_grid_y);

#endif // GUI_LFO_POINTS_H

/*
██╗███╗   ███╗██████╗ ██╗     ███████╗███╗   ███╗███████╗███╗   ██╗████████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
██║████╗ ████║██╔══██╗██║     ██╔════╝████╗ ████║██╔════╝████╗  ██║╚══██╔══╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
██║██╔████╔██║██████╔╝██║     █████╗  ██╔████╔██║█████╗  ██╔██╗ ██║   ██║   ███████║   ██║   ██║██║   ██║██╔██╗ ██║
██║██║╚██╔╝██║██╔═══╝ ██║     ██╔══╝  ██║╚██╔╝██║██╔══╝  ██║╚██╗██║   ██║   ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
██║██║ ╚═╝ ██║██║     ███████╗███████╗██║ ╚═╝ ██║███████╗██║ ╚████║   ██║   ██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝
*/

#ifdef GUI_LFO_POINTS_IMPL
#undef GUI_LFO_POINTS_IMPL

#include <xhl/array.h>
#include <xhl/maths.h>

void gui_lfo_deinit(GUILFOPoints* glfo)
{
    xarr_free(glfo->main_lfo_points);
    xarr_free(glfo->points);
    xarr_free(glfo->skew_points);
    xarr_free(glfo->lfo_cached_path);
    xarr_free(glfo->selected_point_indexes);
    xarr_free(glfo->points_copy);
    xarr_free(glfo->skew_points_copy);
}

float gui_lfo_calculate_point_skew(GUILFOPoints* glfo, int idx)
{
    size_t num_points      = xarr_len(glfo->points);
    size_t num_skew_points = xarr_len(glfo->skew_points);
    size_t next_idx        = idx + 1;

    xassert(idx >= 0 && idx < num_points);
    xassert(idx >= 0 && next_idx < num_points);
    xassert(idx >= 0 && idx < num_skew_points);

    if (next_idx >= num_points)
        next_idx = 0;

    const xvec2f* pt      = glfo->points + idx;
    const xvec2f* next_pt = glfo->points + next_idx;
    const xvec2f* skew_pt = glfo->skew_points + idx;

    float skew = 0.5f;
    if (pt->y != next_pt->y)
        skew = xm_normf(skew_pt->y, next_pt->y, pt->y);
    if (pt->y > next_pt->y)
        skew = 1 - skew;

    skew = xm_clampf(skew, 0, 1);
    return skew;
}

void gui_lfo_add_to_selection(GUILFOPoints* glfo, int idx)
{
    int num_points = xarr_len(glfo->points);
    if (idx == num_points - 1)
        idx = 0;

    int       i;
    const int N = xarr_len(glfo->selected_point_indexes);
    for (i = 0; i < N; i++)
    {
        if (glfo->selected_point_indexes[i] == idx)
            break;
    }
    if (i == N) // idx not in array
    {
        xarr_push(glfo->selected_point_indexes, idx);
        sort_int(glfo->selected_point_indexes, N + 1);

        int num_selected_pts = xarr_len(glfo->selected_point_indexes);
        xassert(num_selected_pts > 0);
        if (num_selected_pts == 1)
            glfo->selected_point_idx = idx;
        else
            glfo->selected_point_idx = -1;
    }
}

void gui_lfo_update_skew_point(GUILFOPoints* glfo, int i, float skew)
{
    xassert(i < xarr_len(glfo->skew_points));
    xassert(i < xarr_len(glfo->points) - 1);
    if (glfo->points[i].x == glfo->points[i + 1].x) // the line between point & next point is vertical
    {
        glfo->skew_points[i].x = glfo->points[i].x;
        // display skew point vertically, halfway between points
        // skew amount not considered
        glfo->skew_points[i].y = (glfo->points[i].y + glfo->points[i + 1].y) * 0.5f;
    }
    else
    {
        // x is always halfway between points
        glfo->skew_points[i].x = (glfo->points[i].x + glfo->points[i + 1].x) * 0.5f;
        // skew amount controls y coord
        const xvec2f* pt1 = glfo->points + i;
        const xvec2f* pt2 = glfo->points + i + 1;
        const float   y   = interp_points(0.5f, 1 - skew, pt1->y, pt2->y);

        glfo->skew_points[i].y = y;
    }
}

// Clamps target_pos to boundaries. Updates relevant skew points. Updates LFO points on audio thread
void gui_lfo_update_point(GUILFOPoints* glfo, xvec2f pos, int idx)
{
    const size_t num_points = xarr_len(glfo->points);

    xvec2f range_horizontal;

    if (idx == 0)
        range_horizontal.left = glfo->area.x;
    else
        range_horizontal.left = glfo->points[idx - 1].x;

    if (idx == 0)
        range_horizontal.right = glfo->area.x;
    else if (idx == num_points - 1)
        range_horizontal.right = glfo->area.r;
    else
        range_horizontal.right = glfo->points[idx + 1].x;

    float i_skew        = gui_lfo_calculate_point_skew(glfo, idx);
    float prev_skew     = 0;
    float last_skew     = 0;
    int   last_skew_idx = (int)xarr_len(glfo->skew_points) - 1;
    if (idx > 0)
        prev_skew = gui_lfo_calculate_point_skew(glfo, idx - 1);
    if (idx == 0)
        last_skew = gui_lfo_calculate_point_skew(glfo, last_skew_idx);

    xvec2f* pt = &glfo->points[idx];
    pt->x      = xm_clampf(pos.x, range_horizontal.l, range_horizontal.r);
    pt->y      = xm_clampf(pos.y, glfo->area.y, glfo->area.b);

    if (idx == 0)
        glfo->points[num_points - 1].y = pt->y;

    gui_lfo_update_skew_point(glfo, idx, i_skew);
    if (idx > 0)
        gui_lfo_update_skew_point(glfo, idx - 1, prev_skew);

    if (idx == 0)
    {
        gui_lfo_update_skew_point(glfo, last_skew_idx, last_skew);
    }
}

void gui_lfo_insert_point(GUILFOPoints* glfo, xvec2f pos, int idx)
{
    const int prev_idx = idx - 1;

#ifndef NDEBUG
    size_t gui_pt_len = xarr_len(glfo->points);
    xassert(idx > 0);
    xassert(idx < gui_pt_len);
    xassert(prev_idx >= 0);
    xvec2f prev_pt = glfo->points[prev_idx];
    xassert(prev_pt.x <= pos.x);
#endif

    float skew = gui_lfo_calculate_point_skew(glfo, prev_idx);

    // add points locally
    xarr_insert(glfo->points, idx, pos);
    xarr_insert(glfo->skew_points, prev_idx, pos);

    gui_lfo_update_skew_point(glfo, prev_idx, skew);
    if (idx < xarr_len(glfo->skew_points))
        gui_lfo_update_skew_point(glfo, idx, 0.5f);
}

void gui_lfo_delete_point(GUILFOPoints* glfo, int idx)
{
    xassert(idx > 0);
    xassert(idx != xarr_len(glfo->skew_points));
    xassert(xarr_len(glfo->skew_points) == (xarr_len(glfo->points) - 1));

    xarr_delete(glfo->points, idx);
    xarr_delete(glfo->skew_points, idx - 1);
    // when user clears the "last point", reset neighbouring skew amounts
    gui_lfo_update_skew_point(glfo, idx - 1, 0.5f);
}

void gui_lfo_save_points_to_copy(GUILFOPoints* glfo)
{
    int N = xarr_len(glfo->points);
    xarr_setlen(glfo->points_copy, N);
    memcpy(glfo->points_copy, glfo->points, N * sizeof(*glfo->points_copy));
}

void gui_lfo_save_skew_points_to_copy(GUILFOPoints* glfo)
{
    int N = xarr_len(glfo->skew_points);
    xarr_setlen(glfo->skew_points_copy, N);
    memcpy(glfo->skew_points_copy, glfo->skew_points, N * sizeof(*glfo->skew_points_copy));
}

void gui_lfo_clear_selection(GUILFOPoints* glfo)
{
    xarr_setlen(glfo->selected_point_indexes, 0);
    glfo->selected_point_idx = -1;
}

void gui_lfo_drag_and_draw(
    GUILFOPoints*              glfo,
    imgui_pt                   pos,
    bool                       snap_to_grid,
    const enum ShapeButtonType shape_type,
    int                        num_grid_x,
    int                        num_grid_y)
{
    glfo->selection_start.u64 = 0;
    glfo->selection_end.u64   = 0;
    gui_lfo_clear_selection(glfo);

    // const enum ShapeButtonType shape_type = gui->plugin->lfo_shape_idx;
    xassert(shape_type >= 0 && shape_type < SHAPE_COUNT);

    // const int lfo_idx     = gui->plugin->selected_lfo_idx;
    // const int pattern_idx = main_get_lfo_pattern_idx(gui->plugin);
    // const int lfo_grid_x = gui->plugin->lfos[lfo_idx].grid_x[pattern_idx];
    // const int lfo_grid_y = gui->plugin->lfos[lfo_idx].grid_y[pattern_idx];
    // const bool snap_to_grid = gui->imgui.frame.modifiers_mouse_move & PW_MOD_PLATFORM_KEY_ALT;

    const float area_width  = glfo->area.r - glfo->area.x;
    const float area_height = glfo->area.b - glfo->area.y;

    const float x_inc = area_width / (float)num_grid_x;
    const float y_inc = area_height / (float)num_grid_y;

    int grid_idx_left    = (int)((pos.x - glfo->area.x) / x_inc);
    grid_idx_left        = xm_clampi(grid_idx_left, 0, num_grid_x - 1);
    float boundary_left  = glfo->area.x + grid_idx_left * x_inc;
    float boundary_right = glfo->area.x + (grid_idx_left + 1) * x_inc;

    float y = pos.y;
    if (snap_to_grid)
    {
        for (int j = 0; j <= num_grid_y; j++)
        {
            float snap_y = glfo->area.y + j * y_inc;
            if (snap_y - LFO_POINT_CLICK_RADIUS <= pos.y && pos.y <= snap_y + LFO_POINT_CLICK_RADIUS)
            {
                y = snap_y;
                break;
            }
        }
    }

    y = xm_clampf(y, glfo->area.y, glfo->area.b);

    // New points at grid boundary
    float pt_y_left = y, pt_y_right = y;
    switch (shape_type)
    {
    case SHAPE_POINT:
    case SHAPE_FLAT:
        break;
    case SHAPE_LINEAR_ASC:
    case SHAPE_CONVEX_ASC:
    case SHAPE_CONCAVE_ASC:
    case SHAPE_COSINE_ASC:
        pt_y_left = glfo->area.b;
        break;
    case SHAPE_LINEAR_DESC:
    case SHAPE_CONVEX_DESC:
    case SHAPE_CONCAVE_DESC:
    case SHAPE_COSINE_DESC:
        pt_y_right = glfo->area.b;
        break;

    case SHAPE_TRIANGLE_UP:
        pt_y_left  = glfo->area.b;
        pt_y_right = glfo->area.b;
        break;
    case SHAPE_TRIANGLE_DOWN:
        pt_y_left  = glfo->area.y;
        pt_y_right = glfo->area.y;
        break;
    case SHAPE_COUNT:
        xassert(false);
        break;
    }

    // Delete points inside boundary range
    {
        const int num_points =
            xarr_len(glfo->points); // note, len(glfo->points) is the same as len(lfo->skew_points) + 1
        for (int i = num_points - 1; i-- != 1;)
        {
            xassert(i > 0);
            bool between  = boundary_left < glfo->points[i].x;
            between      &= glfo->points[i].x < boundary_right;

            if (between)
                gui_lfo_delete_point(glfo, i);
        }
    }

    // Count points at boundaries
    int num_points                   = xarr_len(glfo->points);
    int right_idx                    = -1;
    int num_points_at_right_boundary = 0;
    int left_idx                     = -1;
    int num_points_at_left_boundary  = 0;

    // Count right
    {
        for (int i = num_points; i-- != 0;)
        {
            if (glfo->points[i].x == boundary_right)
            {
                num_points_at_right_boundary++;
            }
            if (glfo->points[i].x < boundary_right)
            {
                right_idx = i + 1;
                break;
            }
        }
        xassert(right_idx >= 0);
    }

    // Count left
    {
        for (int i = 0; i < num_points; i++)
        {
            if (glfo->points[i].x == boundary_left)
            {
                num_points_at_left_boundary++;
            }
            if (glfo->points[i].x > boundary_left)
            {
                left_idx = i - 1;
                break;
            }
        }
        xassert(left_idx >= 0);
    }

    // If there are no points at the left & right boundaries, calculate the existing Y values at those points and insert
    // them as points
    {
        float interp_y_right = 0, interp_y_left = 0;
        if (num_points_at_right_boundary == 0)
        {
            float skew = gui_lfo_calculate_point_skew(glfo, right_idx - 1);

            const xvec2f* pt      = glfo->points + right_idx - 1;
            const xvec2f* next_pt = glfo->points + right_idx;
            const xvec2f* skew_pt = glfo->skew_points + right_idx - 1;

            float amt      = xm_normf(boundary_right, pt->x, next_pt->x);
            interp_y_right = interp_points(amt, 1 - skew, pt->y, next_pt->y);
        }

        if (num_points_at_left_boundary == 0)
        {
            float skew = gui_lfo_calculate_point_skew(glfo, left_idx);

            const xvec2f* pt      = glfo->points + left_idx;
            const xvec2f* next_pt = glfo->points + left_idx + 1;
            const xvec2f* skew_pt = glfo->skew_points + left_idx;

            float amt     = xm_normf(boundary_left, pt->x, next_pt->x);
            interp_y_left = interp_points(amt, 1 - skew, pt->y, next_pt->y);
        }

        if (num_points_at_right_boundary == 0)
        {
            xvec2f pt = {boundary_right, interp_y_right};
            gui_lfo_insert_point(glfo, pt, right_idx);
            num_points++;
            num_points_at_right_boundary++;
        }
        if (num_points_at_left_boundary == 0)
        {
            xvec2f pt = {boundary_left, interp_y_left};
            gui_lfo_insert_point(glfo, pt, left_idx + 1);
            num_points++;
            num_points_at_left_boundary++;
            left_idx++;
            right_idx++;
        }
    }

    if (num_points_at_right_boundary == 1)
    {
        xvec2f pt = {boundary_right, pt_y_right};
        gui_lfo_insert_point(glfo, pt, right_idx);
        num_points++;
        num_points_at_right_boundary++;
    }
    else
    {
        xassert(num_points_at_right_boundary >= 2);
        xvec2f pt = {boundary_right, pt_y_right};
        gui_lfo_update_point(glfo, pt, right_idx);
    }

    if (num_points_at_left_boundary == 1)
    {
        xvec2f prev_pt = glfo->points[left_idx];
        xvec2f pt      = {boundary_left, pt_y_left};
        if (prev_pt.x != pt.x || prev_pt.y != pt.y)
        {
            gui_lfo_insert_point(glfo, pt, left_idx + 1);
            num_points++;
            num_points_at_left_boundary++;
            left_idx++;
        }
    }
    else
    {
        xassert(num_points_at_left_boundary >= 2);
        xvec2f pt = {boundary_left, pt_y_left};
        gui_lfo_update_point(glfo, pt, left_idx);
    }
    // xassert(num_points_at_right_boundary == 2);

    if (shape_type == SHAPE_LINEAR_ASC || shape_type == SHAPE_LINEAR_DESC)
    {
        gui_lfo_update_skew_point(glfo, left_idx, 0.5);
    }
    if (shape_type == SHAPE_CONVEX_ASC || shape_type == SHAPE_CONVEX_DESC)
    {
        gui_lfo_update_skew_point(glfo, left_idx, 0.85);
    }
    if (shape_type == SHAPE_CONCAVE_ASC || shape_type == SHAPE_CONCAVE_DESC)
    {
        gui_lfo_update_skew_point(glfo, left_idx, 0.15);
    }

    if (shape_type == SHAPE_TRIANGLE_UP || shape_type == SHAPE_TRIANGLE_DOWN)
    {
        xvec2f pt = {boundary_left + x_inc * 0.5f, y};
        gui_lfo_insert_point(glfo, pt, left_idx + 1);
        num_points++;
        left_idx++;
    }

    if (shape_type == SHAPE_COSINE_ASC || shape_type == SHAPE_COSINE_DESC)
    {
        // Approximation for a descening cosing shape
        // x = (1 / 7), y = 0.95, skew = 0.731994
        // x = 0.3333,  y = 0.75, skew = 0.56
        // x = 0.6666,  y = 0.25, skew = 0
        // x = (6 / 7), y = 0.05, skew = 0.44
        // x = 1,       y = 0,    skew = 1 - 0.731994
        xvec2f pts[] = {
            {(1.0f / 7.0f), 0.95f},
            {(1.0f / 3.0f), 0.75f},
            {(2.0f / 3.0f), 0.25f},
            {(6.0f / 7.0f), 0.05f},
        };
        float skews[] = {
            0.731994f,
            0.56,
            0.5,
            0.46,
            1 - 0.731994,
        };
        _Static_assert(ARRLEN(pts) == ARRLEN(skews) - 1, "");
        if (shape_type == SHAPE_COSINE_ASC)
        {
            for (int i = 0; i < ARRLEN(pts); i++)
            {
                xvec2f* pt = pts + i;
                pt->y      = 1 - pt->y;
            }
            for (int i = 0; i < ARRLEN(skews); i++)
                skews[i] = 1 - skews[i];
        }

        for (int i = 0; i < ARRLEN(pts); i++)
        {
            xvec2f* pt = pts + i;
            pt->x      = xm_lerpf(pt->x, boundary_left, boundary_right);
            pt->y      = xm_lerpf(pt->y, glfo->area.b, y);
            gui_lfo_insert_point(glfo, *pt, left_idx + 1);
            gui_lfo_update_skew_point(glfo, left_idx, skews[i]);
            num_points++;
            left_idx++;
        }
        gui_lfo_update_skew_point(glfo, left_idx, skews[ARRLEN(skews) - 1]);
    }

    gui_lfo_save_points_to_copy(glfo);
    gui_lfo_save_skew_points_to_copy(glfo);
}

void gui_lfo_handle_point_events(
    GUILFOPoints*     glfo,
    GUILFOFrameState* fstate,
    imgui_context*    im,
    int               num_grid_x,
    int               num_grid_y)

{
    xassert(glfo->pw);
    xassert(glfo->arena);
    xassert(glfo->area.r > glfo->area.x);
    xassert(glfo->area.b > glfo->area.y);
    LINKED_ARENA_LEAK_DETECT_BEGIN(glfo->arena);

    println("%f %f", im->pos_mouse_move.x, im->pos_mouse_move.y);

    // Point events
    {
        const int num_points    = xarr_len(glfo->points_copy);
        bool      backup_points = false;

        for (int pt_idx = 0; pt_idx < num_points; pt_idx++)
        {
            unsigned uid = 'lfop' + pt_idx;

            // const xvec2f pt        = glfo->points_copy[pt_idx];

            // Properly track point position
            // We're reading out of an array of points cached at the beginning of a drag, and not the points that are
            // actually displayed
            imgui_pt       pt        = (im->uid_mouse_hold == uid || im->frame.uid_mouse_up == uid)
                                           ? im->pos_mouse_move
                                           : (imgui_pt){glfo->points_copy[pt_idx].x, glfo->points_copy[pt_idx].y};
            const unsigned pt_events = imgui_get_events_circle(im, uid, pt, LFO_POINT_CLICK_RADIUS);

            if (pt_events == 0)
                continue;

            if (pt_events & IMGUI_EVENT_MOUSE_HOVER)
            {
                fstate->pt_hover_idx = pt_idx;
                if (glfo->selected_point_idx != -1 && pt_events & IMGUI_EVENT_MOUSE_LEFT_HOLD)
                    fstate->pt_hover_idx = glfo->selected_point_idx;
            }

            if (pt_events & IMGUI_EVENT_MOUSE_ENTER)
            {
                pw_set_mouse_cursor(glfo->pw, PW_CURSOR_HAND_DRAGGABLE);
            }

            if (pt_events & IMGUI_EVENT_MOUSE_LEFT_DOWN)
            {
                int select_idx = pt_idx;
                if ((pt_idx + 1) == num_points)
                    select_idx = 0;

                if (im->left_click_counter == 1)
                {
                    bool shift_click     = (PW_MOD_KEY_SHIFT | PW_MOD_LEFT_BUTTON) == im->frame.modifiers_mouse_down;
                    bool idx_is_selected = false;
                    for (int i = 0; i < xarr_len(glfo->selected_point_indexes); i++)
                    {
                        if (select_idx == glfo->selected_point_indexes[i])
                        {
                            idx_is_selected = true;
                            break;
                        }
                    }
                    if (shift_click == false && idx_is_selected == false)
                    {
                        gui_lfo_clear_selection(glfo);
                    }

                    gui_lfo_add_to_selection(glfo, select_idx);
                    fstate->pt_hover_idx = select_idx;
                    int num_selected_pts = xarr_len(glfo->selected_point_indexes);
                    xassert(num_selected_pts > 0);
                    pw_set_mouse_cursor(glfo->pw, PW_CURSOR_HAND_DRAGGING);
                }
                else if (im->left_click_counter == 2 && select_idx > 0)
                {
                    fstate->delete_pt_idx = pt_idx;
                    imgui_clear_widget(im);
                }
            }
            if (pt_events & IMGUI_EVENT_DRAG_BEGIN)
            {
                xarr_setlen(glfo->points_copy, num_points);
                xarr_setlen(glfo->skew_points_copy, (num_points - 1));
                xassert(0 == memcmp(glfo->points_copy, glfo->points, sizeof(*glfo->points_copy) * num_points));
                xassert(
                    0 == memcmp(
                             glfo->skew_points_copy,
                             glfo->skew_points,
                             sizeof(*glfo->skew_points_copy) * (num_points - 1)));
            }
            if (pt_events & IMGUI_EVENT_DRAG_MOVE)
            {
                const int num_selected = xarr_len(glfo->selected_point_indexes);

                if (num_selected == 1)
                {
                    const bool alt_drag     = im->frame.modifiers_mouse_move & PW_MOD_PLATFORM_KEY_ALT;
                    const bool snap_to_grid = alt_drag && (num_selected == 1);

                    xvec2f drag_pos = (xvec2f){im->pos_mouse_move.x, im->pos_mouse_move.y};
                    drag_pos.y      = xm_clampf(drag_pos.y, glfo->area.y, glfo->area.b);
                    drag_pos.x      = xm_clampf(drag_pos.x, glfo->area.x, glfo->area.r);
                    if (snap_to_grid)
                    {
                        float x_inc = (glfo->area.r - glfo->area.x) / num_grid_x;
                        float y_inc = (glfo->area.b - glfo->area.y) / num_grid_y;
                        for (int j = 0; j <= num_grid_x; j++)
                        {
                            float x = glfo->area.x + j * x_inc;
                            if (x - LFO_POINT_CLICK_RADIUS <= drag_pos.x && drag_pos.x <= x + LFO_POINT_CLICK_RADIUS)
                            {
                                drag_pos.x = x;
                                break;
                            }
                        }
                        for (int j = 0; j <= num_grid_y; j++)
                        {
                            float y = glfo->area.y + j * y_inc;
                            if (y - LFO_POINT_CLICK_RADIUS <= drag_pos.y && drag_pos.y <= y + LFO_POINT_CLICK_RADIUS)
                            {
                                drag_pos.y = y;
                                break;
                            }
                        }
                    }

                    // Points on edges cannot be dragged past other points
                    const int sel_idx = glfo->selected_point_indexes[0];
                    if (sel_idx == 0 || sel_idx == num_points - 1)
                    {
                        gui_lfo_update_point(glfo, drag_pos, sel_idx);
                    }
                    else
                    {
                        // Rebuid points array, skipping any points between the beginning and current drag position
                        float range_l = xm_minf(drag_pos.x, im->pos_mouse_down.x);
                        float range_r = xm_maxf(drag_pos.x, im->pos_mouse_down.x);
                        range_l       = xm_maxf(range_l, glfo->area.x);
                        range_r       = xm_minf(range_r, glfo->area.r);

                        const float clamp_range_l = range_l + LFO_POINT_DRAG_ERASE_DISTANCE;
                        const float clamp_range_r = range_r - LFO_POINT_DRAG_ERASE_DISTANCE;

                        float* skew_amts = linked_arena_alloc(glfo->arena, sizeof(*skew_amts) * num_points);

                        int npoints              = 0;
                        glfo->points[npoints++]  = glfo->points_copy[0];
                        glfo->selected_point_idx = -1;

                        xvec2f(*view_pts)[512]      = (void*)glfo->points;
                        xvec2f(*view_skew_pts)[512] = (void*)glfo->skew_points;
                        xvec2f(*view_src_pts)[512]  = (void*)glfo->points_copy;

                        for (int j = 1; j < num_points; j++)
                        {
                            float skew = 0.5f;
                            // Calc skew amount from cached points
                            const xvec2f* p1 = glfo->points_copy + j - 1;
                            const xvec2f* p2 = glfo->points_copy + j;
                            const xvec2f* sp = glfo->skew_points_copy + j - 1;
                            if (p1->y != p2->y)
                                skew = xm_normf(sp->y, p2->y, p1->y);
                            if (p1->y > p2->y)
                                skew = 1 - skew;

                            // Update displayed points
                            if (j == sel_idx)
                            {
                                // Defer adding this point so we can give it an opportunity to get clamped
                                glfo->selected_point_idx = npoints;
                                npoints++;
                            }
                            else if (j < sel_idx && p2->x <= clamp_range_l)
                            {
                                glfo->points[npoints++] = *p2;

                                if (p2->x >= range_l) // Clamp dragged point to nearby point
                                    drag_pos.x = xm_maxf(p2->x, drag_pos.x);
                                xassert(drag_pos.x >= p2->x);
                            }
                            else if (j > sel_idx && p2->x >= clamp_range_r)
                            {
                                glfo->points[npoints++] = *p2;
                                if (p2->x <= range_r) // Clamp dragged point to nearby point
                                    drag_pos.x = xm_minf(p2->x, drag_pos.x);
                                xassert(drag_pos.x <= p2->x);
                            }
                            else
                            {
                                continue;
                            }

                            xassert((npoints - 2) >= 0);
                            skew_amts[npoints - 2] = skew;
                        }
                        xassert(glfo->selected_point_idx != -1);
                        if (glfo->selected_point_indexes >= 0)
                        {
                            fstate->pt_hover_idx = glfo->selected_point_idx;

                            glfo->points[glfo->selected_point_idx] = drag_pos;
                        }

                        // Update displayed skew point
                        for (int j = 0; j < npoints - 1; j++)
                        {
                            xassert((j + 1) < npoints);
                            const xvec2f* p1   = glfo->points + j;
                            const xvec2f* p2   = glfo->points + j + 1;
                            xvec2f*       sp   = glfo->skew_points + j;
                            float         skew = skew_amts[j];

                            xassert(p1->x <= p2->x);

                            if (p1->x == p2->x) // the line between point & next_p is vertical
                            {
                                sp->x = p1->x;
                                // display skew point vertically, halfway between points
                                // skew amount not considered
                                sp->y = (p1->y + p2->y) * 0.5f;
                            }
                            else
                            {
                                float y = interp_points(0.5f, 1 - skew, p1->y, p2->y);

                                // x is always halfway between points
                                sp->x = (p1->x + p2->x) * 0.5f;
                                // skew amount controls y coord
                                sp->y = y;
                            }
                        }

                        xarr_header(glfo->points)->length      = npoints;
                        xarr_header(glfo->skew_points)->length = npoints - 1;

                        linked_arena_release(glfo->arena, skew_amts);
                    }
                }
                else if (num_selected > 1)
                {
                    const xvec2f delta = {
                        im->pos_mouse_move.x - im->pos_mouse_down.x,
                        im->pos_mouse_move.y - im->pos_mouse_down.y};

                    bool has_moved_first_point = false;

                    for (int i = 0; i < num_selected; i++)
                    {
                        int idx = glfo->selected_point_indexes[i];

                        xassert(idx != (num_points - 1));

                        bool is_first_point = idx == 0 || idx == (num_points - 1);
                        if (is_first_point && has_moved_first_point)
                            continue;

                        has_moved_first_point |= is_first_point;

                        xvec2f translate_pos  = glfo->points_copy[idx];
                        translate_pos.x      += delta.x;
                        translate_pos.y      += delta.y;

                        gui_lfo_update_point(glfo, translate_pos, idx);
                    }
                }

                fstate->should_update_cached_path                = true;
                fstate->should_update_gui_lfo_points_with_points = true;
            } // IMGUI_EVENT_DRAG_MOVE

            if (pt_events & IMGUI_EVENT_MOUSE_LEFT_UP)
            {
                pw_set_mouse_cursor(glfo->pw, PW_CURSOR_HAND_DRAGGABLE);
                backup_points = true;
            }
        } // end loop points

        if (fstate->delete_pt_idx > 0)
        {
            gui_lfo_delete_point(glfo, fstate->delete_pt_idx);
            gui_lfo_clear_selection(glfo);

            fstate->pt_hover_idx      = -1;
            fstate->pt_hover_skew_idx = -1;
            backup_points             = true;

            fstate->should_update_gui_lfo_points_with_points = true;
            fstate->should_update_cached_path                = true;
        }

        if (backup_points)
        {
            gui_lfo_save_points_to_copy(glfo);
            gui_lfo_save_skew_points_to_copy(glfo);
        }
    }

    // Skew point events
    {
        const int num_skew_points = xarr_len(glfo->skew_points);
        for (int pt_idx = 0; pt_idx < num_skew_points; pt_idx++)
        {
            unsigned       uid       = 'lskp' + pt_idx;
            const xvec2f   pt        = glfo->skew_points[pt_idx];
            const unsigned pt_events = imgui_get_events_circle(im, uid, (imgui_pt){pt.x, pt.y}, LFO_POINT_CLICK_RADIUS);
            if (pt_events == 0)
                continue;

            if (pt_events & IMGUI_EVENT_MOUSE_HOVER)
                fstate->pt_hover_skew_idx = pt_idx;

            if (pt_events & IMGUI_EVENT_MOUSE_ENTER)
            {
                pw_set_mouse_cursor(glfo->pw, PW_CURSOR_RESIZE_NS);
            }

            if (pt_events & IMGUI_EVENT_MOUSE_LEFT_DOWN)
            {
                gui_lfo_clear_selection(glfo);

                const bool ctrl = im->frame.modifiers_mouse_down & PW_MOD_PLATFORM_KEY_CTRL;
                if (im->left_click_counter == 2 && !ctrl)
                {
                    gui_lfo_update_skew_point(glfo, pt_idx, 0.5f);
                    fstate->pt_hover_skew_idx                        = -1;
                    fstate->should_update_gui_lfo_points_with_points = true;
                    fstate->should_update_cached_path                = true;
                }
            }
            if (pt_events & IMGUI_EVENT_DRAG_MOVE)
            {
                float delta = 0;
                imgui_drag_value(im, &delta, -1, 1, LFO_SKEW_DRAG_RANGE * 2, IMGUI_DRAG_VERTICAL);

                float skew      = gui_lfo_calculate_point_skew(glfo, pt_idx);
                float next_skew = skew + delta;
                next_skew       = xm_clampf(next_skew, 0.0f, 1.0f);
                xassert(next_skew >= 0 && next_skew <= 1);

                gui_lfo_update_skew_point(glfo, pt_idx, next_skew);
                fstate->should_update_gui_lfo_points_with_points = true;
                fstate->should_update_cached_path                = true;
            }
            if (pt_events & IMGUI_EVENT_MOUSE_LEFT_UP)
            {
                gui_lfo_save_skew_points_to_copy(glfo);
            }
        }
    }

    LINKED_ARENA_LEAK_DETECT_END(glfo->arena);
}

#endif // GUI_LFO_POINTS_IMPL