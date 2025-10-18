#pragma once
// Quick and dirty attempt at adding "automatic garbage collection" or "immediate mode assets/resources" to the GUI
// Resources may include
#include <nanovg2.h>
#include <xhl/debug.h>

enum ResourceType
{
    RESOURCE_TYPE_PIPELINE,
    RESOURCE_TYPE_FRAMEBUFFER,
    RESOURCE_TYPE_STORAGEBUFFER,
    // sg_image + sg_view + width _ height
    RESOURCE_TYPE_TEXTURE, // Nanovg have their own 'Image' type, but I want to replace it with this
};

enum
{
    RESOURCE_FLAG_NODELETE, // Will not delete resource at end of frame when not used
};

typedef struct ResourceHeader
{
    enum ResourceType type;
    uint32_t          id;
    uint32_t          size;
    uint32_t          flags;

    unsigned char payload[];
} ResourceHeader;

typedef struct
{
    // Cache the idx of the previous item we searched for.
    // In most cases, the item we will need next will be at the next index
    uint32_t count;
    uint32_t prev_idx;
    void*    prev_header;

    // List or variably sized objects
    ResourceHeader* data
} ResourceList;

typedef struct ResourceManager
{
    ResourceList list_a;
    ResourceList list_b;

    ResourceList* list_current; // a or b
} ResourceManager;

// Private
void* _resource_find(ResourceManager* rm, uint32_t id)
{
    ResourceList*   list = rm->list_current;
    ResourceHeader* head = list->prev_header;
    if (head == NULL)
        head = list->data;

    xassert(head);
    uint32_t       i    = 0;
    uint32_t       idx  = list->prev_idx;
    const uint32_t prev = list->prev_idx;
    const uint32_t N    = list->count;

    while (i < N && head->id != id)
    {
        i++;
        head = (void*)head + head->size;

        idx = i + prev;
        if (idx == N)
            head = list->data;
        if (idx >= N)
            idx -= N;
    }
    list->prev_idx    = idx;
    list->prev_header = head;

    return i < N ? &head->payload : NULL;
}

// TODO
void resource_get_pipeline();
void resource_get_framebuffer();
void resource_get_storagebuffer();
void resource_get_texture();

void resources_drop_type(ResourceManager* rm, enum ResourceType type)
{
    // TODO: drop everything matching that type
}

void resources_frame_end(ResourceManager* rm)
{
    // TODO: Defrag & destroy unused resources
}