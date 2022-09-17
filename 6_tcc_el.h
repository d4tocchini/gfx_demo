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
extern void vg_path(vg_t* ctx);
extern void vg_rect(vg_t* ctx, float x, float y, float w, float h);
extern void vg_fill_color(vg_t* ctx, vg_color_t color);
extern void vg_fill(vg_t* ctx);
extern vg_color_t vg_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
// # include <tcclib.h> /* include the "Simple libc header for TCC" */
// typedef struct vg_t 			vg_t;
extern void getmouse(int*m);
int* m;
void init(void)
{
    getmouse(&m);
}
// Rect_f layout(Rect_f rect)
// {
//     return rect;
// }
void render(vg_t* vg, int win_w, int win_h)
{
    vg_path(vg);
    vg_fill_color(vg, vg_rgba(192,192,0,70));
    vg_rect(vg, 10, 10, win_w-20,win_h-20);
    vg_fill(vg);
    vg_path(vg);
    vg_rect(vg, m[0] - 60, m[1], 120, 30);
    vg_fill(vg);
}
