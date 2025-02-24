#pragma once
#include <cplug.h>

enum Parameter
{
    PARAM_LP_CUTOFF,
    PARAM_LP_RESONANCE,
    PARAM_HP_CUTOFF,
    PARAM_HP_RESONANCE,
    PARAM_FEEDBACK_GAIN,
};
enum
{
    NUM_PARAMS = PARAM_FEEDBACK_GAIN + 1,

    GUI_INIT_WIDTH  = 800,
    GUI_INIT_HEIGHT = GUI_INIT_WIDTH / 2,

    GUI_RATIO_X = 2,
    GUI_RATIO_Y = 1,

    GUI_MIN_WIDTH  = GUI_RATIO_X * 100,
    GUI_MIN_HEIGHT = GUI_RATIO_Y * 100,
};

typedef struct Plugin
{
    CplugHostContext* cplug_ctx;

    void* gui;
    int   width, height;

    double   sample_rate;
    uint32_t max_block_size;

    double params[NUM_PARAMS];

    struct FilterState
    {
        float lp[2];
        float hp[2];

        float prev_sample;
    } state[2];

    // Data for GUI
    bool  is_clipping;
    float peak_gain;

} Plugin;

void param_change_begin(Plugin* p, uint32_t param_idx);
void param_change_end(Plugin* p, uint32_t param_idx);
void param_change_update(Plugin* p, uint32_t param_idx, double value);

static void param_set(Plugin* p, uint32_t param_idx, double value)
{
    param_change_begin(p, param_idx);
    param_change_update(p, param_idx, value);
    param_change_end(p, param_idx);
}