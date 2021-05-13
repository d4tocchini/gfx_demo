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
    \#include "gfx_demo/6_tcc_el.h"
);

int mouse[3];
__attribute__ ((visibility ("default")))
void getmouse(int**m) {
    *m = mouse;
}

int main (int argc, char *args[])
{
    int win_w, win_h;
    int fb_w, fb_h;
    float px_ratio;

    TCCState* el_cc;
    el_cc = tcc_new();
    if (!el_cc) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }
    // assert( tcc_get_error_func(s) == NULL ); // assert( tcc_get_error_opaque(s) == NULL );
    tcc_set_error_func(el_cc, stderr, tcc_on_error);
    // assert(tcc_get_error_func(s) == tcc_on_error); // assert(tcc_get_error_opaque(s) == stderr);

    TIME_BEGIN(TCC)
    const char* TCC_FLAGS = getenv("TCC_FLAGS");
    if (TCC_FLAGS) {
        tcc_set_options(el_cc, TCC_FLAGS);
        // printf("TCC_FLAGS %s\n",TCC_FLAGS);
    }
    else {
        tcc_set_options(el_cc, STRINGIFY(
            -nostdlib -nostdinc -m64
            -Wno-implicit-function-declaration
        ));
        // tcc_set_options(el_cc, "  -O2 -pthread -nostdlib -nostdinc -m64 "
        //                     "-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include "
        //                     "-I/usr/local/d4/code/d4/pkg/c/dep/lemongraph/lib "
        //                     "-I/usr/local/d4/code/d4/pkg/c/usr/lmdb/include "
        //                 );
        // tcc_set_lib_path(el_cc, "/usr/local/d4/code/d4/usr/lib/tcc/include");
        // tcc_add_include_path(el_cc, "/usr/local/d4/code/d4/usr/lib/tcc/include");
        // tcc_add_include_path(el_cc, "/usr/local/d4/code/d4/usr/include");
        // tcc_add_include_path(el_cc, "/usr/local/d4/code/d4/src");
        // tcc_add_include_path(el_cc, "/usr/local/d4/code/d4/usr/include");
        // tcc_add_library_path(el_cc, "/usr/local/d4/code/d4/usr/lib");
    }
    tcc_set_output_type(el_cc, TCC_OUTPUT_MEMORY);
    // tcc_output_file(el_cc, path)
    if (tcc_compile_string(el_cc, src) == -1)   return 1;

    tcc_add_symbol(el_cc, "vg_path", vg_path);
    tcc_add_symbol(el_cc, "vg_fill_color", vg_fill_color);
    tcc_add_symbol(el_cc, "vg_fill", vg_fill);
    tcc_add_symbol(el_cc, "vg_rect", vg_rect);
    tcc_add_symbol(el_cc, "vg_rgba", vg_rgba);
    tcc_add_symbol(el_cc, "getmouse", getmouse);

    // if (tcc_relocate(el_cc, TCC_RELOCATE_AUTO) < 0)
    //     return 1;

    void* el_ob;
    int el_ob_size = tcc_relocate(el_cc, NULL);
    assert(el_ob_size > 0);
    el_ob = aligned_malloc(64, el_ob_size);
    assert(tcc_relocate(el_cc, el_ob) >= 0);

    void (*el_render)(vg_t* vg, int win_w, int win_h);
    void (*el_init)(void);
    el_init = tcc_get_symbol(el_cc, "init");
    el_render = tcc_get_symbol(el_cc, "render");

    assert(el_render);
    TIME_END(TCC)
    printf("el_ob size = %i \n", el_ob_size);
    printf("el_ob addr = %" PRIXPTR " \n", (uintptr_t)el_ob);

    tcc_delete(el_cc); // no longer need compiler state

    /*
    testing if compiled code is safe to copy/persist
        * https://stackoverflow.com/questions/60654834/using-mprotect-to-make-text-segment-writable-on-macos
        * https://developer.apple.com/forums/thread/133183
        * https://github.com/tobegit3hub/hotpatch
    */

    /*
    uintptr_t fun_offset = (uintptr_t)el_render - (uintptr_t)el_ob;
    assert((uintptr_t)el_render == (uintptr_t)el_ob + fun_offset);
    el_render = el_ob + fun_offset;
    printf("fun addr = %" PRIXPTR " \n", (uintptr_t)el_render);
    printf("fun offset = %i \n", fun_offset);
    */

    // int pgsize = getpagesize();
    // void* mem = aligned_malloc(64, el_ob_size);
    // // // void* mem = malloc(el_ob_size);
    // memcpy(mem, el_ob, el_ob_size);
    // int remainder = (uintptr_t)el_render % pgsize;
    // printf("pgsize = %i \n", pgsize);
    // printf("pgsize remainder = %i \n", pgsize);
    // mprotect(mem + fun_offset - remainder, el_ob_size - fun_offset + remainder, PROT_EXEC|PROT_READ);
    // el_render = mem + fun_offset;

    // // testing reading/writing compiled code
    // FILE *el_ob_file = fopen ("6.dat", "w+");
    // if (!el_ob_file)
    //     fprintf(stderr, "ERROR opening file");
    // fread(&mem, el_ob_size, 1, el_ob_file);
    // // fwrite(el_ob, el_ob_size, 1, el_ob_file);
    // fclose(el_ob_file);




    el_init();

    gfx_init();
    win_t* win = gfx_create_win(WIN_FLOAT|WIN_BORDERLESS|WIN_TRANSPARENT);

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
        el_render(vg, win_w, win_h);
		vg_frame_end(vg);
        // // vg_did_render(vg);
        win_did_render(win);
        gfx_did_render();
    }



    // vg_destroy(vg);
    win_destroy(win);
    SDL_Quit();

    aligned_free(el_ob);
    // aligned_free(mem);
    return 0;
}