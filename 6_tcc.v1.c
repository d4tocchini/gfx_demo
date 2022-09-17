//
// cc sdl-metal-example.m `sdl2-config --cflags --libs` -framework Metal -framework QuartzCore && ./a.out
//
// https://gist.github.com/TheSpydog/e8c6ce4854cb2ebdc1a69fb3dd5c978c
#ifndef GFX_ENV
    #define GFX_ENV
    #define GFX_MAC
    #define GFX_GL3
#endif
#include "gfx/gfx.h"
// #define REN_MICROUI
#include "gfx/ren.h"

#include <stdio.h>
#include <stdlib.h> // getenv
#include <assert.h>
#include "libtcc.h"
#include <string.h> // memcpy
#include <inttypes.h> // ptr types

#include <unistd.h> // getpagesize
#include <sys/types.h>
#include <sys/mman.h> // mprotect

#include <time.h>
#define TIME_BEGIN(name) \
    clock_t name##_time_ = clock();
#define TIME_END(name) \
    double name##_elapsed_ = ((double)(clock() - name##_time_))/CLOCKS_PER_SEC; \
    printf("    - %s total time: %f sec \n", #name, name##_elapsed_);

#define align_up(num, align) \
	(((num) + ((align) - 1)) & ~((align) - 1))

/**
* aligned_malloc takes in the requested alignment and size
*	We will call malloc with extra bytes for our header and the offset
*	required to guarantee the desired alignment.
*/
void * aligned_malloc(size_t align, size_t size)
{
	void * ptr = NULL;

	//We want it to be a power of two since align_up operates on powers of two
	assert((align & (align - 1)) == 0);

	if(align && size)
	{
		/*
		 * We know we have to fit an offset value
		 * We also allocate extra bytes to ensure we can meet the alignment
		 */
		uint32_t hdr_size = sizeof(size_t) + (align - 1);
		void * p = malloc(size + hdr_size);

		if(p)
		{
			/*
			 * Add the offset size to malloc's pointer (we will always store that)
			 * Then align the resulting value to the arget alignment
			 */
			ptr = (void *) align_up(((uintptr_t)p + sizeof(size_t)), align);

			//Calculate the offset and store it behind our aligned pointer
			*((uintptr_t *)ptr - 1) = (uintptr_t)((uintptr_t)ptr - (uintptr_t)p);

		} // else NULL, could not malloc
	} //else NULL, invalid arguments

	return ptr;
}

/**
* aligned_free works like free(), but we work backwards from the returned
* pointer to find the correct offset and pointer location to return to free()
* Note that it is VERY BAD to call free() on an aligned_malloc() pointer.
*/
void aligned_free(void * ptr)
{
	assert(ptr);

	/*
	* Walk backwards from the passed-in pointer to get the pointer offset
	* We convert to an uintptr_t pointer and rely on pointer math to get the data
	*/
	uintptr_t offset = *((uintptr_t *)ptr - 1);

	/*
	* Once we have the offset, we can get our original pointer and call free
	*/
	void * p = (void *)((uint8_t *)ptr - offset);
	free(p);
}

#define STRINGIFY(...) #__VA_ARGS__

void tcc_on_error(void *opaque, const char *msg)
{
    fprintf(opaque, "%s\n", msg);
}
// TCCState* tcc_init(void)
// {

// }


char src0[] =
    ""
;
    // "# define VG_EXTERN_API \n"
    // "# include \"nanovg/src/nanovg.h\" \n"
char src[] = STRINGIFY(
    typedef struct vg_t 			vg_t;
    typedef struct vg_color_t 		vg_color_t;
    struct vg_color_t {
    	union {
    		float rgba[4];
    		struct {
    			float r,g,b,a;
    		};
    	};
    };
    void vg_path(vg_t* ctx);
    void vg_rect(vg_t* ctx, float x, float y, float w, float h);
    void vg_fill_color(vg_t* ctx, vg_color_t color);
    void vg_fill(vg_t* ctx);
    vg_color_t vg_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    // # include <tcclib.h> /* include the "Simple libc header for TCC" */
    // typedef struct vg_t 			vg_t;
    void getmouse(int*m);
    int* m;
    void init(void)
    {
        getmouse(&m);
    }
    void render(vg_t* vg, int win_w, int win_h)
    {
        vg_path(vg);
        vg_fill_color(vg, vg_rgba(0,192,0,70));
        vg_rect(vg, 10, 10, win_w-20,win_h-20);
        vg_fill(vg);
        vg_path(vg);
        vg_rect(vg, m[0] - 60, m[1], 120, 30);
        vg_fill(vg);
    }
    void render2(vg_t* vg, int win_w, int win_h, int mx, int my)
    {
        vg_path(vg);
        vg_fill_color(vg, vg_rgba(0,192,0,70));
        vg_rect(vg, 10, 10, win_w-20,win_h-20);
        vg_fill(vg);
        vg_path(vg);
        vg_rect(vg, mx-60,my, 120,30);
        vg_fill(vg);
    }
);

int mouse[3];
void getmouse(int**m) {
    *m = mouse;
}

int main (int argc, char *args[])
{
    int win_w, win_h;
    int fb_w, fb_h;
    float px_ratio;


    TCCState* s;
    s = tcc_new();
    if (!s) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }
    // assert( tcc_get_error_func(s) == NULL );
    // assert( tcc_get_error_opaque(s) == NULL );

    tcc_set_error_func(s, stderr, tcc_on_error);

    // assert(tcc_get_error_func(s) == tcc_on_error);
    // assert(tcc_get_error_opaque(s) == stderr);
    TIME_BEGIN(TCC)
    const char* TCC_FLAGS = getenv("TCC_FLAGS");
    if (TCC_FLAGS) {
        tcc_set_options(s, TCC_FLAGS);
        // printf("TCC_FLAGS %s\n",TCC_FLAGS);
    }
    else {
        tcc_set_options(s, "-nostdlib -nostdinc -m64 -Wno-implicit-function-declaration");
        // tcc_set_options(ctx, "  -O2 -pthread -nostdlib -nostdinc -m64 "
        //                     "-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include "
        //                     "-I/usr/local/d4/code/d4/pkg/c/dep/lemongraph/lib "
        //                     "-I/usr/local/d4/code/d4/pkg/c/usr/lmdb/include "
        //                 );
        // tcc_set_lib_path(s, "/usr/local/d4/code/d4/usr/lib/tcc/include");
        // tcc_add_include_path(s, "/usr/local/d4/code/d4/usr/lib/tcc/include");
        // tcc_add_include_path(s, "/usr/local/d4/code/d4/usr/include");
        // tcc_add_include_path(s, "/usr/local/d4/code/d4/src");
        // tcc_add_include_path(s, "/usr/local/d4/code/d4/usr/include");
        // tcc_add_library_path(s, "/usr/local/d4/code/d4/usr/lib");
    }


    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    // tcc_output_file(s, path)
    // if (tcc_compile_string(s, src0) == -1)  return 1;
    if (tcc_compile_string(s, src) == -1)   return 1;

    tcc_add_symbol(s, "vg_path", vg_path);
    tcc_add_symbol(s, "vg_fill_color", vg_fill_color);
    tcc_add_symbol(s, "vg_fill", vg_fill);
    tcc_add_symbol(s, "vg_rect", vg_rect);
    tcc_add_symbol(s, "vg_rgba", vg_rgba);
    tcc_add_symbol(s, "getmouse", getmouse);


    // if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
    //     return 1;

    void* lib;
    int lib_size = tcc_relocate(s, NULL);
    if (lib_size == -1)
        return 1;
    int pgsize = getpagesize();
    lib = aligned_malloc(64, lib_size);
        // lib = malloc(lib_size);
    if (tcc_relocate(s, lib) < 0)
        return 1;

    void (*render)(vg_t* vg, int win_w, int win_h);
    void (*init)(void);

    init = tcc_get_symbol(s, "init");
    render = tcc_get_symbol(s, "render");
    if (!render)
        return 1;

    uintptr_t fun_offset = (uintptr_t)render - (uintptr_t)lib;
    assert((uintptr_t)render == (uintptr_t)lib + fun_offset);
    render = lib + fun_offset;

    /*
    testing if compiled code is safe to copy/persist
        * https://stackoverflow.com/questions/60654834/using-mprotect-to-make-text-segment-writable-on-macos
        * https://developer.apple.com/forums/thread/133183
        * https://github.com/tobegit3hub/hotpatch
    */

    TIME_END(TCC)

    printf("lib size = %i \n", lib_size);
    printf("lib addr = %" PRIXPTR " \n", (uintptr_t)lib);
    printf("fun addr = %" PRIXPTR " \n", (uintptr_t)render);
    printf("fun offset = %i \n", fun_offset);

    // void* mem = aligned_malloc(64, lib_size);
    // // // void* mem = malloc(lib_size);
    // memcpy(mem, lib, lib_size);
    // int remainder = (uintptr_t)render % pgsize;
    // printf("pgsize = %i \n", pgsize);
    // printf("pgsize remainder = %i \n", pgsize);
    // mprotect(mem + fun_offset - remainder, lib_size - fun_offset + remainder, PROT_EXEC|PROT_READ);
    // render = mem + fun_offset;

    // // testing reading/writing compiled code
    // FILE *lib_file = fopen ("6.dat", "w+");
    // if (!lib_file)
    //     fprintf(stderr, "ERROR opening file");
    // fread(&mem, lib_size, 1, lib_file);
    // // fwrite(lib, lib_size, 1, lib_file);
    // fclose(lib_file);

    // no longer need compiler state
    tcc_delete(s);

    // free(lib);


    init();

    gfx_init();
    win_t* win = win_create(WIN_FLOAT|WIN_BORDERLESS|WIN_TRANSPARENT);

    win_set_size(win, 720, 720);
    win_set_pos(win, WINPOS_CENTER, WINPOS_CENTER);
    win_show(win);

    vg_t* vg = win_create_vg(win, VG_VSYNC|VG_ACCEL|VG_TRANSPARENT);

    printf("--\n");
    win_get_size(win, &win_w, &win_h);
    win_get_fbsize(win, &fb_w, &fb_h);
    px_ratio = (float)fb_w / (float)win_w;
    printf("win_w %i, fb_w %i, px_ratio %f\n",win_w,fb_w,px_ratio);
    printf("---\n");
    color_t clear = {0.f,0.f,0.f,0.f};

    int quit = 0;
    SDL_Event e;
    printf("----\n");
    // printf("-------------\n");
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT: quit = 1; break;
            }
        }
        mouse[2] = SDL_GetMouseState(&mouse[0], &mouse[1]);
        // printf("mx=%i\n",mouse[0]);
        vg_viewport(vg,0,0,fb_w,fb_h);
        vg_clear(vg, clear);
        vg_frame_begin(vg, win_w, win_h, px_ratio);
        render(vg, win_w, win_h);
		vg_frame_end(vg);
        // // vg_did_render(vg);
        win_did_render(win);
        gfx_did_render();
    }



    // vg_destroy(vg);
    win_destroy(win);
    SDL_Quit();

    aligned_free(lib);
    // aligned_free(mem);
    return 0;
}