
#define __DEBUG__ 0
#define __TRACE__ 0

#define VIEWS_MAX 8
#define SUBVIEWS_MAX 6
#define DRAWLAYERS_NUM 3


#ifndef GFX_ENV
    #define GFX_ENV
    #define GFX_MAC
    #define GFX_GL33
    // #define GFX_WEBGL2
#endif
#include "gfx/gfx.h"
#include "gfx/ren.h"
#define  STB_IMAGE_WRITE_IMPLEMENTATION
#include STB_IMAGE_WRITE__H
#undef   STB_IMAGE_WRITE_IMPLEMENTATION
#include "gfx/gpu.h"

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<time.h>

#define CONSTRUCTOR(fn)                              \
        static void fn(void) __attribute__((constructor)); \
        static void fn(void)

#if __TRACE__
    #define ___filter \
        ((__func__[0] == '_') && (__func__[1] == '_'))

    #define ___ \
        ___filter && printf ("%s\n\t|--- %s:%d \n",\
                            __func__, __FILE__, __LINE__);
    #define ____ \
        ___filter && printf ("\t|--- %s:%d \n",\
                                      __FILE__, __LINE__);
#else
    #define ___
    #define ____
#endif

typedef uint64_t U64;   typedef int64_t  I64;
typedef uint32_t U32;   typedef int32_t  I32;
typedef uint16_t U16;   typedef int16_t  I16;
typedef uint8_t  U8;    typedef int8_t  I8;

 
#define _MAP_RANGE(input, in0, in1, out0, out1) \
    (out0 + ((input - in0) / (in1 - in0)) * (out1 - out0))

#define _MAP_RANGE_01(input, out0, out1) \
    (out0 + input * (out1 - out0))

#define MAP_RANGE(input, in0, in1, out0, out1) _MAP_RANGE((input), (in0), (in1), (out0), (out1))
#define MAP_RANGE_01(input, out0, out1) MAP_RANGE_01((input), (out0), (out1))

// typedef float  F32;

typedef union 
Vec2f {
    struct { float x, y; };
    struct { float w, h; };    
    float v[2];
} Vec2f;

typedef union 
Vec4f {
    struct { float x, y, w, h; };    
    struct { Vec2f pos, size; };    
    struct { Vec2f xy, wh; };    
    float v[4];
    GPU_Rect gpu_rect;
} 
Vec4f;

#define LAYER_TOP (DRAWLAYERS_NUM - 1)
#define LAYER_BOTTOM 0
#define LAYER_OFFSCREEN -1

typedef struct 
Proto 
{
    void (*     init        )(int idx);
    void (*     measure     )(int idx);
    void (*     layout      )(int idx);
    void (*     drawlayer   )(int idx, int layer);    
    void (*     teardown    )(int idx);    
    // Proto* parent;
} 
Proto;

#define VIEW_EXISTS         (1<<0)
#define VIEW_NEEDS_MEASURE  (1<<1)
#define VIEW_NEEDS_LAYOUT   (1<<2)
#define VIEW_NEEDS_DRAW     (1<<3)
#define VIEW_NEEDS_ALL      (VIEW_NEEDS_MEASURE|VIEW_NEEDS_LAYOUT|VIEW_NEEDS_DRAW)      
#define VIEW_HIDDEN         (1<<4)

typedef I16 
View_Id;

typedef struct 
View 
{
    Vec4f       rect;    
    // float       matrix[16];
    float       x;
    float       y;
    float       w;
    float       h;
    Proto*      proto;    
    void*       props;    
    void*       state;
    I32         flags;            
    View_Id     superview;
    View_Id     subviews[SUBVIEWS_MAX];
} 
View;

typedef struct 
View_List 
{
    View_Id     _next;
    View        items[VIEWS_MAX];
    // tombstone
} View_List;

typedef struct 
Document 
{
    View_Id     root;
    View_List   view_list;
    int         layers;
    int         vp_w,       vp_h;
    float       px_ratio;
    int         m_x,        m_y;
    int         scroll_x,   scroll_y;        
    int         root_scroll_view;
    GPU_Target* target;
    vg_t*       vg;        
} Document;

static Document doc;

#define view_ref(idx) (View*)(&doc.view_list.items[idx])
#define view_read(idx) doc.view_list.items[idx]
#define view_write(idx, view) (doc.view_list.items[idx] = view)
#define view_is_valid(view) (!(view.flags & VIEW_EXISTS))

#define _VIEW_FOR_EACH_SUB(view) \
    for(int i = 0, sub = view.subviews[0]; \
        sub > 0 && i < SUBVIEWS_MAX; \
        i++, sub = view.subviews[i] )

#define VIEW_FOR_EACH_SUB(view) _VIEW_FOR_EACH_SUB((view))

    // int i = 0;
    // View_Id sub = view.subviews[i];
    // while (sub > 0 && ++i < SUBVIEWS_MAX) {
    //     if (_view_measure(sub))
    //         break;
    //     sub = view.subviews[i];
    // }    

#define _VIEW_SUPER_WALK(view) \   
    for (int i = 0, super = view.superview; \
        super > 0; \
        i++, super = view_ref(super)->superview )    

#define VIEW_SUPER_WALK(view) _VIEW_SUPER_WALK((view))

static inline View
view_req(View_Id id) 
{   
    assert(id < VIEWS_MAX);
    View view = view_read(id);
    assert(view.flags & VIEW_EXISTS);
    return view;
}

View_Id 
view_alloc(void)
{
    View_Id id = doc.view_list._next++;
    return id;
}

View_Id
view_create(View view) 
{
    Proto* proto = view.proto;
    assert(proto != NULL);    

    // GPU_MatrixIdentity(&view.matrix);
    view.flags |= VIEW_EXISTS | VIEW_NEEDS_ALL;

    View_Id id = view_alloc();
    assert(id > 0);
    doc.view_list.items[id] = view;
    
    if (proto->init)
        proto->init(id);
    return id;
}

void
view_attach_to(int id, int super)
{
    View* view = view_ref(id);
    view->superview = super;
    VIEW_FOR_EACH_SUB(*view) 
    {
        view_attach_to(sub, super);
    }    
}

void
view_set_needs_measure(View_Id id, bool needs)
{
    View* view = view_ref(id);
    int flags = view->flags;
    if (needs) 
    {
        if (flags & VIEW_NEEDS_MEASURE)
            return;
        view->flags |= VIEW_NEEDS_MEASURE;
        view_set_needs_measure(view->superview, needs); // propagate upwards
    }
    else 
    {
        if (!(flags & VIEW_NEEDS_MEASURE)) 
            return;
        view->flags ^= VIEW_NEEDS_MEASURE;
    }
}

void
view_set_needs_layout(View_Id id, bool needs)
{
    View view = view_req(id);
    if (0 == (view.flags & VIEW_NEEDS_MEASURE)) 
    {
        view.flags |= VIEW_NEEDS_MEASURE;
        // ....
    }
}

void
view_set_needs_draw(View_Id id, bool needs)
{
    View view = view_req(id);
    if (0 == (view.flags & VIEW_NEEDS_MEASURE)) 
    {
        view.flags |= VIEW_NEEDS_MEASURE;
        // ....
    }
}

// ----------------------------------------------------------------------------

static inline int
_view_measure(int id)
{   ___    
    const View view = view_read(id);        
    if (!(view.flags & VIEW_EXISTS))        
        return 1;
    if (view.flags & VIEW_HIDDEN)
        return 0;
    //
    // printf("\tview id %i \n",id);
    // printf("\tview.subviews %i %i %i \n",view.subviews[0],view.subviews[1],view.subviews[2]);
    VIEW_FOR_EACH_SUB(view) 
    {
        if (_view_measure(sub))
            break;
    }
    //
    if (view.proto->measure != NULL) 
        view.proto->measure(id);    
    return 0;
}

static inline int
_view_layout(View_Id id)
{   ___
    const View view = view_read(id);
    if (!(view.flags & VIEW_EXISTS)) 
        return 1;
        // (view.flags & VIEW_NEEDS_LAYOUT) 
    if ((view.flags & VIEW_HIDDEN)) 
        return 0;
    //
    if (view.proto->layout != NULL) 
        view.proto->layout(id);     
    //
    VIEW_FOR_EACH_SUB(view) 
    {
        if (_view_layout(sub))
            break;
    }    
    return 0;
}

static inline int
_view_drawlayer(View_Id id, int layer)
{   ___
    const View view = view_read(id);
    if (!(view.flags & VIEW_EXISTS))
        return 1;
    if (view.flags & VIEW_HIDDEN)
        return 0;
    //
    if (view.proto->drawlayer != NULL) 
        view.proto->drawlayer(id, layer);        
    //
    VIEW_FOR_EACH_SUB(view) 
    {
        if (_view_drawlayer(sub, layer))
            break;
    }    
    return 0;
}


// 

typedef struct 
VG_Image 
{
    GPU_Image*  image;
    vg_fb_t*    fb;
    void(*      draw)
    (
        vg_t*       _vg, 
        Vec4f       rect
    );
} 
VG_Image;

// ----------------------------------------------------------------------------

#include "./data/views/header.c"
#include "./data/views/scroller.h"

// ----------------------------------------------------------------------------



    
// #include <dlfcn.h>

// Proto* Header;

// void
// _init_modules(void)
// {
//     // DYLD_LIBRARY_PATH WTF https://github.com/flandr/wtf-osx-dlopen
//     void *mod;
//     mod = dlopen("./data/views/header.so", RTLD_LOCAL | RTLD_LAZY | RTLD_FIRST );
//     if (!mod) {
//         fputs (dlerror(), stderr);
//         exit(1);
//     }
//     Header = (Proto*)dlsym(mod, "Header");
// }

// ----------------------------------------------------------------------------

void
document_init(void)
{   ___

    // _init_modules();

    view_alloc();
    doc.layers = DRAWLAYERS_NUM;
    doc.vp_w = 72*20;
    doc.vp_h = 72*11;
    doc.px_ratio = 1.0f;
    doc.target = GPU_Init(doc.vp_w, doc.vp_h, GPU_DEFAULT_INIT_FLAGS);     
    doc.vg = vg_create(NULL, NVG_ANTIALIAS | NVG_STENCIL_STROKES | (NVG_DEBUG*(!!__DEBUG__)));         
}

void 
document_load(View_Id id)
{   ___
    doc.root = id;    
    view_attach_to(id, 0);
}

void
__mousemove(SDL_MouseMotionEvent e)
{
    ___
    doc.m_x = e.x;
    doc.m_y = e.y;
    // e.xrel
    // e.yrel
}


// TODO
void 
__mousewheel(SDL_MouseWheelEvent e) 
{
    ___
    bool wheel_x_active = false;
    bool wheel_y_active = false;
    float wheel_x = 0.0f;
    float wheel_y = 0.0f;
    #if GFX_WASM
        e.direction = SDL_MOUSEWHEEL_FLIPPED;
        e.preciseX *= -1.666f;
        e.preciseY *= -1.666f;
        e.x *= -1;
        e.y *= -1;        
    #else
        if (e.direction == SDL_MOUSEWHEEL_FLIPPED) {            
                e.preciseX *= -1.0f;
                e.preciseY *= -1.0f;
                e.x *= -1;
                e.y *= -1;
        }
    #endif
    if (e.preciseX != 0.0f) {
            wheel_x_active = true;
            /* "positive to the right and negative to the left"  */
            wheel_x += e.preciseX * 11.0f;
    }
    if (e.preciseY != 0.0f) {
            wheel_y_active = true;
            /* "positive away from the user and negative towards the user" */
            wheel_y -= e.preciseY * 11.0f;
    }
    
    // e.x, e.y, e.timestamp
    int idx = doc.root_scroll_view;
    assert(idx);
    if (!idx)
        return;
    View* self = view_ref(idx);
    Scroller_State* state = (Scroller_State*)self->state;
    if (wheel_y_active)
        state->scroll_dy = wheel_y; // TODO set needs ...
}
// void 
// __mousewheel(SDL_MouseWheelEvent e) 
// {
//     // int wheel_dx , int wheel_dy, unsigned int timestamp
//     // e.wheel.x, e.wheel.y, e.wheel.timestamp
// }

void
__mousebutton(SDL_MouseButtonEvent e)
{
    ___
    if (e.type == SDL_MOUSEBUTTONDOWN) {

    }
    else if (e.type == SDL_MOUSEBUTTONUP) {

    }
}

void 
__keyboard(SDL_KeyboardEvent e)
{   ___
    SDL_Keysym keysym = e.keysym;
    if (e.type == SDL_MOUSEBUTTONDOWN) {

    }
    else if (e.type == SDL_MOUSEBUTTONUP) {

    }
    // SDLK_RETURN
}

void
__textinput(SDL_TextInputEvent e)
{
    char* text = e.text;
}

void
__windowevent(SDL_WindowEvent e)
{
    switch (e.event) 
    {
        // case SDL_WINDOWEVENT_SHOWN:
        // case SDL_WINDOWEVENT_HIDDEN:
        // case SDL_WINDOWEVENT_EXPOSED: SDL_Log("Window %d shown", event->window.windowID);
        // case SDL_WINDOWEVENT_MOVED:
        // case SDL_WINDOWEVENT_RESIZED:                
        case SDL_WINDOWEVENT_SIZE_CHANGED:  break;
            // window size has changed, either as a result of an API call or through the system or user changing the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an external event, i.e. the user or the window manager
        // case SDL_WINDOWEVENT_MINIMIZED:
        // case SDL_WINDOWEVENT_MAXIMIZED:
        // case SDL_WINDOWEVENT_RESTORED: // window has been restored to normal size and position
        // case SDL_WINDOWEVENT_ENTER:   // window has gained mouse focus
        // case SDL_WINDOWEVENT_LEAVE:   // window has lost mouse focus
        // case SDL_WINDOWEVENT_FOCUS_GAINED: // window has gained keyboard focus
        // case SDL_WINDOWEVENT_FOCUS_LOST:  // window has lost keyboard focus
        // case SDL_WINDOWEVENT_CLOSE:   // the window manager requests that the window be closed
        // case SDL_WINDOWEVENT_TAKE_FOCUS: // window is being offered a focus (should SDL_SetWindowInputFocus() on itself or a subwindow, or ignore) (>= SDL 2.0.5)
    }
}

void
__displayevent(SDL_DisplayEvent e)
{

}

void
_document_input(void)
{   ___
    SDL_Event e;    
    while (SDL_PollEvent(&e)) 
    {
        ____
        switch (e.type) 
        {
        case SDL_QUIT:              exit(0);                    break;
        case SDL_MOUSEMOTION:       __mousemove(e.motion);      break;                
        case SDL_MOUSEWHEEL:        __mousewheel(e.wheel);      break;
        case SDL_MOUSEBUTTONDOWN:   
        case SDL_MOUSEBUTTONUP:     __mousebutton(e.button);    break;
        case SDL_KEYDOWN:         
        case SDL_KEYUP:             __keyboard(e.key);          break;
        case SDL_TEXTINPUT:         __textinput(e.text);        break;                                    
        case SDL_WINDOWEVENT:       __windowevent(e.window);    break;
        case SDL_DISPLAYEVENT:      __displayevent(e.display);  break;        
        }                
    }
}

void
_document_measure(void) 
{   ___
    _view_measure(doc.root);
}

void
_document_layout(void) 
{   ___
    _view_layout(doc.root);
}

void
_document_draw(void)
{   ___
    const View_Id root = doc.root;    
    _view_drawlayer(root, -1); // layer for preload/render to texture without clobbering gl blit state
    
    /* SDL_gpu + NanoVG Rendering */
    glViewport(0, 0, doc.vp_w, doc.vp_h);
    GPU_ClearRGBA(doc.target, 0x00, 0x00, 0x00, 0xFF); // GPU_ClearRGBA clears GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
    glClear(GL_STENCIL_BUFFER_BIT); // IMPORTANT: GPU_ClearRGBA does not clear GL_STENCIL_BUFFER_BIT
        // glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    int layer = 0;
    const int layers = doc.layers;
    while (layer < layers) 
    {
        _view_drawlayer(root, layer++);
    }

    /* Finish */
    GPU_Flip(doc.target); // Render to screen
}

void 
document_tick(void)
{   ___
    _document_input();
    _document_measure();
    _document_layout();
    _document_draw();
}





static void setup(ren_ctx_t* ctx) 
{

    // win_max_size(ctx->win);
    // ren_install_font_defaults();
    // demo_img = ren_load_img(DEMO_IMG,
    //     // #ifndef GFX_WASM ?
    //     NVG_IMAGE_GENERATE_MIPMAPS
    // );
}

static void teardown(void)
{
    // ren_destroy();
}

static void loop()
{
    document_tick();
    // ren_ctx_t* ctx
    // if (ren_INPUT() != 0) {
    // //  ren_delay(16);
    // //   continue;
    //     ren_BEGIN();
    //     render();
    //     ren_END();
    //     ren_present();
    // }
    // REN_DELAY_FRAME()
}

int font_install(const char* name, const char* file)
{
    int id = nvgCreateFont(doc.vg, name, file);
    if (id == -1) {
        printf("font_install : Could not install \"%s\" font at %s\n", name, file);        
        assert(id > -1);
    }
    return id;
}

// #include "jsomd/example.c"
int main(int argc, char **argv) 
{
    
    #if __DEBUG__ > 0
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    GPU_Log("register_all\n");
    #endif

    // ren_install_font("sans", "data/font/InterDisplay-SemiBold.otf");
    gfx_init();
    document_init();
    font_install("display","data/font/AG/AkzidenzGrotesk-Bold.otf");        
    
    View_Id root = view_create((View){
        .proto = &Scroller,
        .subviews = {
            view_create((View){
                .proto = &Header
            }),
            view_create((View){
                .proto = &Header
            }),            
            view_create((View){
                .proto = &Header
            })           
            // view_create((View){
            //     .proto = &Header
            // })            
        }
    });

    /*
    

    */

    // View_Id root = view_create((View){
    //     .proto = &Header
    // });
    

    document_load(root);

    // ren_ctx_t* ctx = ren_init_ctx();
    // setup(ctx);
    REN_LOOP(loop)
    teardown();
    // return 0;
}


