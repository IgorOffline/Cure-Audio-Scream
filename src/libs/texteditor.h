#pragma once
#include "../common.h"

#include "linked_arena.h"
#include <cplug_extensions/window.h>
#include <stdbool.h>
#include <stdint.h>
#include <xhl/vector.h>

enum TextEditorAction
{
    TEXT_ACTION_MOVE,
    TEXT_ACTION_WRITE,
    TEXT_ACTION_DELETE,
};

typedef struct TextEditor
{
    xvec4f dimensions;

    int* codepoints; // UTF32 format

    int selection_start;
    int selection_end;
    int ibeam_idx;

    float text_offset;

    float font_size;

    uint64_t time_last_ibeam_move;

    enum TextEditorAction last_action;
    LinkedArena*          undo_arena;
    // Nullable reference to data in the above arena.
    // Will be set to NULL on init
    struct TextEditorUndoState* ref_current_state;

    int active_param; // enum ParamID
} TextEditor;

void ted_init(TextEditor*);
void ted_deinit(TextEditor*);

void ted_activate(TextEditor* ted, xvec4f dimensions, xvec2f pos, float _font_size, int param_id);
void ted_deactivate(TextEditor* ted);

// Returns true if event is consumed/used by our app
bool ted_handle_key_down(TextEditor* ted, const PWEvent* event);
void ted_handle_text(TextEditor* ted, int codepoint); // utf32 codepoint
void ted_handle_mouse_down(TextEditor* ted);
void ted_handle_mouse_drag(TextEditor* ted);

// Returns number of bytes copied into 'buf', excluding the NULL terminating byte
size_t ted_get_text(TextEditor*, char* buf, size_t buflen);
void   ted_clear(TextEditor*);

void ted_draw(TextEditor*);
