//
// cc sdl-metal-example.m `sdl2-config --cflags --libs` -framework Metal -framework QuartzCore && ./a.out
//
// https://gist.github.com/TheSpydog/e8c6ce4854cb2ebdc1a69fb3dd5c978c
#ifndef GFX_ENV
    #define GFX_ENV
    #define GFX_MAC
    #define GFX_GL33
#endif
#include "gfx/gfx.h"

int main (int argc, char *args[])
{
    gfx_init();
    win_t* win = win_create(WIN_FLOAT|WIN_BORDERLESS|WIN_TRANSPARENT);

    win_set_size(win, 720, 720);
    win_set_pos(win, WINPOS_CENTER, WINPOS_CENTER);
    win_show(win);

    vg_t* vg = win_create_vg(win, VG_VSYNC|VG_ACCEL|VG_TRANSPARENT);

    int win_w, win_h;
    int fb_w, fb_h;
    int mx, my;
    float px_ratio;

    win_get_size(win, &win_w, &win_h);
    win_get_fbsize(win, &fb_w, &fb_h);
    px_ratio = (float)fb_w / (float)win_w;
    printf("win_w %i, fb_w %i, px_ratio %f\n",win_w,fb_w,px_ratio);

    color_t clear = {0.f,0.f,0.f,0.f};

    bool quit = false;
    SDL_Event e;

    int x = 1;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: quit = true; break;
            }
        }
        SDL_GetMouseState(&mx, &my);
        vg_viewport(vg,0,0,fb_w,fb_h);
        vg_clear(vg, clear);
        vg_frame_begin(vg, win_w, win_h, px_ratio);
			vg_path(vg);
            vg_fill_color(vg, nvgRGBA(0,192,0,70));
            vg_rect(vg, 10, 10, win_w-20,win_h-20);
			vg_fill(vg);
            vg_path(vg);
            vg_rect(vg, mx-60,my, 120,30);
            vg_fill(vg);
		vg_frame_end(vg);

        // // vg_did_render(vg);
        win_did_render(win);
        gfx_did_render();

        // @autoreleasepool {
        //     id<CAMetalDrawable> surface = [metallayer nextDrawable];

        //     color.red = (color.red > 1.0) ? 0 : color.red + 0.01;

        //     MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
        //     pass.colorAttachments[0].clearColor = color;
        //     pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
        //     pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        //     pass.colorAttachments[0].texture = surface.texture;

        //     id<MTLCommandBuffer> buffer = [queue commandBuffer];
        //     id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
        //     [encoder endEncoding];
        //     [buffer presentDrawable:surface];
        //     [buffer commit];
        // }
    }
    // vg_destroy(vg);
    win_destroy(win);
    SDL_Quit();
    return 0;
}