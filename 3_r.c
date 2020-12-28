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
// #define REN_MICROUI
#include "gfx/ren.h"

static void draw(ren_ctx_t* ctx)
{
    ren_draw_rect(
            (ren_rect_t) { .x = 20, .y = 20, .w = 300, .h = 200 },
            (ren_color_t) { .r = 128, .g = 0, .b = 180, .a = 50 });
        ren_draw_rect(
            (ren_rect_t) { .x = 20, .y = 200, .w = 300, .h = 200 },
            (ren_color_t) { .r = 0, .g = 128, .b = 180, .a = 50 });
        ren_draw_text(
            "Hello World",
            (ren_vec2_t){.x=(ctx->win_w>>1), .y=(ctx->win_h>>1)},
            (ren_color_t){.r=255, .g=255, .b=255, .a=255});
        ren_set_clip_rect(
            (ren_rect_t) { .x = 0, .y = 0, .w = 100, .h = ctx->win_h });
            ren_draw_rect(
                (ren_rect_t) { .x = 80, .y = 0, .w = 300, .h = ctx->win_h },
                (ren_color_t) { .r = 0, .g = 255, .b = 0, .a = 128 });
            ren_draw_rect(
                (ren_rect_t) { .x = 80, .y = 200, .w = 300, .h = 100 },
                (ren_color_t) { .r = 255, .g = 0, .b = 0, .a = 128 });
        ren_set_clip_rect(
            (ren_rect_t) { .x = 120, .y = 100, .w = 100, .h = 200 });
            ren_draw_rect(
                (ren_rect_t) { .x = 80, .y = 120, .w = 300, .h = 100 },
                (ren_color_t) { .r = 0, .g = 255, .b = 0, .a = 128 });
            ren_draw_rect(
                (ren_rect_t) { .x = 80, .y = 200, .w = 300, .h = 100 },
                (ren_color_t) { .r = 255, .g = 0, .b = 0, .a = 128 });
            ren_draw_text(
                "(ren_vec2_t){.x=(ctx->win_w>>1), .y=(ctx->win_h>>2)}",
                (ren_vec2_t){.x=(ctx->win_w>>1), .y=(ctx->win_h>>2)},
                (ren_color_t){.r=255, .g=255, .b=255, .a=255});
}

int main(int argc, char* args[])
{
    ren_ctx_t* ctx = ren_init_ctx();
    ren_install_font_defaults();
    while (true) {
        ren_INPUT();
        // ...
        ren_BEGIN();
        draw(ctx);
        ren_END();
        ren_present();
    }
    ren_destroy();
    return 0;
}