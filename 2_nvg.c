//
// cc sdl-metal-example.m `sdl2-config --cflags --libs` -framework Metal -framework QuartzCore && ./a.out
//
// https://gist.github.com/TheSpydog/e8c6ce4854cb2ebdc1a69fb3dd5c978c
# define GFX_MAC
# define GFX_DARWIN
# define GFX_SDL
# define GFX_GL33
#include "gfx/gfx.h"

#include "nanovg/example/demo.h"
#include "nanovg/example/perf.h"
#include "nanovg/example/demo.c"
#include "nanovg/example/perf.c"


double gfx_time() {
    // static Uint64 freq; = SDL_GetPerformanceFrequency()
    return ((double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency());
}

int blowup = 0;
int screenshot = 0;
int premult = 0;
DemoData data;

int main (int argc, char *args[])
{
    win_t* win;
    vg_t* vg;
    // GPUtimer gpuTimer;
    PerfGraph fps, cpuGraph; //, gpuGraph;
    double prevt = 0, cpuTime = 0;

    gfx_init();

    initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
	// initGraph(&gpuGraph, GRAPH_RENDER_MS, "GPU Time");

// #   ifdef DEMO_MSAA
// 	    glfwWindowHint(GLFW_SAMPLES, 4);
// #   endif

    if ( !(win = gfx_create_win(WIN_RESIZEABLE)) ) {
        puts("Failed to create window");
        return -1;
    }
    win_set_size(win, 720, 720);
    win_set_pos(win, WINPOS_CENTER, WINPOS_CENTER);
    win_show(win);


// #ifdef DEMO_MSAA
// 	vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
// #else
// 	vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
// #endif

    vg = win_create_vg(win, 0
        // | VG_ACCEL
        // | VG_VSYNC
    );

    if (loadDemoData(vg, &data) == -1)
		return -1;

    // glfwSwapInterval(0);

    // initGPUTimer(&gpuTimer);
    // glfwSetTime(0);
	// prevt = gfx_time();
    gfx_set_time(0);
	prevt = gfx_get_time();

    color_t clear = {0.f,0.f,0.f,0.f};
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        #ifdef GFX_METAL
            @autoreleasepool {
        #endif

        double t, dt;
		int win_w, win_h;
        int fb_w, fb_h;
        int mx = 0;
        int my = 0;
		float px_ratio;
		// float gpuTimes[3];
		int i, n;

		// t = gfx_time();
        t = gfx_get_time();
		dt = t - prevt;
		prevt = t;
        // startGPUTimer(&gpuTimer);

        win_get_size(win, &win_w, &win_h);
        win_get_fbsize(win, &fb_w, &fb_h);
        px_ratio = (float)fb_w / (float)win_w;

        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: quit = true; break;
            }
        }
        // SDL_GetMouseState(&mx, &my);

        vg_viewport(vg,0,0,fb_w,fb_h);
        if (premult)
			clear.r = clear.g = clear.b = clear.a = 0.f;
		else {
            clear.r = 0.3f;
            clear.g = 0.3f;
            clear.b = 0.32f;
            clear.a = 1.f;
        }
		vg_clear(vg, clear);
        vg_frame_begin(vg, win_w, win_h, px_ratio);
            renderDemo(vg, mx,my, win_w,win_h, t, blowup, &data);
            renderGraph(vg, 5,5, &fps);
            renderGraph(vg, 5+200+5,5, &cpuGraph);
            // if (gpuTimer.supported)
            //     renderGraph(vg, 5+200+5+200+5,5, &gpuGraph);
		vg_frame_end(vg);

        // cpuTime = gfx_time() - t;
        cpuTime = gfx_get_time() - t;
        // printf("%fs\n",dt);
        updateGraph(&fps, dt);
		updateGraph(&cpuGraph, cpuTime);


		// We may get multiple results.
		// n = stopGPUTimer(&gpuTimer, gpuTimes, 3);
		// for (i = 0; i < n; i++)
		// 	updateGraph(&gpuGraph, gpuTimes[i]);

		// if (screenshot) {
		// 	screenshot = 0;
		// 	saveScreenShot(fbWidth, fbHeight, premult, "dump.png");
		// }

        // vg_did_render(vg);
        win_did_render(win);
        gfx_did_render();

        #ifdef GFX_METAL
            }
        #endif
    }

	printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
	printf("          CPU Time: %.2f ms\n", getGraphAverage(&cpuGraph) * 1000.0f);
	// printf("          GPU Time: %.2f ms\n", getGraphAverage(&gpuGraph) * 1000.0f);

    freeDemoData(vg, &data);
    vg_destroy(vg);
    win_destroy(win);
    SDL_Quit();
    return 0;
}