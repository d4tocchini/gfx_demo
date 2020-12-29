

#ifndef GFX_ENV
    #define GFX_ENV
    #define GFX_MAC
    #define GFX_GL33
    // #define GFX_WEBGL2
#endif
#include "gfx/gfx.h"
#define REN_MICROUI
#include "gfx/ren.h"

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<time.h>

#ifndef DEMO_LG
    #define DEMO_LG 0
#endif
#ifndef DEMO_IMG
    #define DEMO_IMG "./data/img/d5.png"
#endif

static  char logbuf[64000];
static   int logbuf_updated = 1;
static float bg[3] = { 90, 95, 100 };

// #define USE_LG_DB

#if DEMO_LG
    #define	O_RDWR		0x0002
    #define	O_CREAT		0x0200
    #include "lemongraph.h"

    const char * const  path = "./.tmp/graph";
    const int           mode = 0760;
    const int           os_flags = O_RDWR | O_CREAT;
        int           db_flags = 0;

    static graph_t g;
    static logID_t msg_logid = 0;
#endif

static void read_log();

int data_init() {
    #if DEMO_LG
        g = graph_open(path,
            os_flags, mode,
            db_flags//|DB_NOSYNC|DB_NORDAHEAD
        );
        if (g == NULL) {
            fprintf(stderr, graph_strerror(errno));
            return 1;
        }
        read_log();
    #endif
    return 0;

}

static void read_log() {
    #if DEMO_LG
        logbuf[0] = '\0';
        // txn
        graph_txn_t txn = graph_txn_begin(g, NULL, DB_RDONLY);
        if (txn == NULL) {
            return;
        }
        // iter
        graph_iter_t nodes = graph_nodes_type(txn, "msg",3, msg_logid);
        node_t node;
        int i = 0;
        while ((node = (node_t) graph_iter_next(nodes))) {
            i++;
            // graph_node_print(nodes->txn, node, nodes->beforeID);
            size_t vlen;
            char * val = graph_string(txn, node->val, &vlen);
            strcat(logbuf, val);
            strcat(logbuf, "\n");
            free(node);
        }
        // end iter
        graph_iter_close(nodes);
        free(nodes);
        // end txn
        msg_logid = graph_log_nextID(txn);
        printf("msg_logid=%i, added:%i\n",msg_logid,i);
        graph_txn_abort(txn);
        //
        // logbuf_updated = 1;
    #endif
}

static void write_log(const char *text) {
    #if DEMO_LG
        logbuf_updated = 1;
        graph_txn_t txn = graph_txn_begin(g, NULL, 0);
        if (txn == NULL) {
            return ;
        }
        node_t n1 = graph_node_resolve(txn, "msg",3, text,strlen(text));
        free(n1);
        msg_logid = graph_log_nextID(txn);
        graph_txn_commit(txn);
    #else
      if (logbuf[0]) { strcat(logbuf, "\n"); }
      strcat(logbuf, text);
    #endif
}

static void test_window(mu_Context *ctx, ren_rect_t rect) {
  /* do window */
  if (mu_begin_window(ctx, "Demo Window", rect)) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

    /* window info */
    if (mu_header(ctx, "Window Info")) {
      mu_Container *win = mu_get_current_container(ctx);
      char buf[64];
      mu_layout_row(ctx, 2, (int[]) { 54, -1 }, 0);
      mu_label(ctx,"Position:");
      sprintf(buf, "%d, %d", win->rect.x, win->rect.y); mu_label(ctx, buf);
      mu_label(ctx, "Size:");
      sprintf(buf, "%d, %d", win->rect.w, win->rect.h); mu_label(ctx, buf);
    }

    /* labels + buttons */
    if (mu_header_ex(ctx, "Test Buttons", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 3, (int[]) { 86, -110, -1 }, 0);
      mu_label(ctx, "Test buttons 1:");
      if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
      if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
      mu_label(ctx, "Test buttons 2:");
      if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
      if (mu_button(ctx, "Popup")) { mu_open_popup(ctx, "Test Popup"); }
      if (mu_begin_popup(ctx, "Test Popup")) {
        mu_button(ctx, "Hello");
        mu_button(ctx, "World");
        mu_end_popup(ctx);
      }
    }

    /* tree */
    if (mu_header_ex(ctx, "Tree and Text", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { 140, -1 }, 0);
      mu_layout_begin_column(ctx);
      if (mu_begin_treenode(ctx, "Test 1")) {
        if (mu_begin_treenode(ctx, "Test 1a")) {
          mu_label(ctx, "Hello");
          mu_label(ctx, "world");
          mu_end_treenode(ctx);
        }
        if (mu_begin_treenode(ctx, "Test 1b")) {
          if (mu_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
          if (mu_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
          mu_end_treenode(ctx);
        }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 2")) {
        mu_layout_row(ctx, 2, (int[]) { 54, 54 }, 0);
        if (mu_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
        if (mu_button(ctx, "Button 4")) { write_log("Pressed button 4"); }
        if (mu_button(ctx, "Button 5")) { write_log("Pressed button 5"); }
        if (mu_button(ctx, "Button 6")) { write_log("Pressed button 6"); }
        mu_end_treenode(ctx);
      }
      if (mu_begin_treenode(ctx, "Test 3")) {
        static int checks[3] = { 1, 0, 1 };
        mu_checkbox(ctx, "Checkbox 1", &checks[0]);
        mu_checkbox(ctx, "Checkbox 2", &checks[1]);
        mu_checkbox(ctx, "Checkbox 3", &checks[2]);
        mu_end_treenode(ctx);
      }
      mu_layout_end_column(ctx);

      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 1, (int[]) { -1 }, 0);
      mu_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
        "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
        "ipsum, eu varius magna felis a nulla.");
      mu_layout_end_column(ctx);
    }

    /* background color sliders */
    if (mu_header_ex(ctx, "Background Color", MU_OPT_EXPANDED)) {
      mu_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);
      /* sliders */
      mu_layout_begin_column(ctx);
      mu_layout_row(ctx, 2, (int[]) { 46, -1 }, 0);
      mu_label(ctx, "Red:");   mu_slider(ctx, &bg[0], 0, 255);
      mu_label(ctx, "Green:"); mu_slider(ctx, &bg[1], 0, 255);
      mu_label(ctx, "Blue:");  mu_slider(ctx, &bg[2], 0, 255);
      mu_layout_end_column(ctx);
      /* color preview */
      mu_Rect r = mu_layout_next(ctx);
      mu_draw_rect(ctx, r, mu_color(bg[0], bg[1], bg[2], 255));
      char buf[32];
      sprintf(buf, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
      mu_draw_control_text(ctx, buf, r, MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
    }

    mu_end_window(ctx);
  }
}


static void log_window(mu_Context *ctx) {
  if (mu_begin_window(ctx, "Log Window", mu_rect(350, 40, 300, 200))) {
    /* output text panel */
    mu_layout_row(ctx, 1, (int[]) { -1 }, -25);
    mu_begin_panel(ctx, "Log Output");
    mu_Container *panel = mu_get_current_container(ctx);
    mu_layout_row(ctx, 1, (int[]) { -1 }, -1);
    if (logbuf_updated) {
      read_log();
    }
    mu_text(ctx, logbuf);
    mu_end_panel(ctx);
    if (logbuf_updated) {
        printf("panel->content_size.y = %i\n",panel->content_size.y);
      panel->scroll.y = panel->content_size.y;
      logbuf_updated = 0;
    }

    /* input textbox + submit button */
    static char buf[128];
    int submitted = 0;
    mu_layout_row(ctx, 2, (int[]) { -70, -1 }, 0);
    if (mu_textbox(ctx, buf, sizeof(buf)) & MU_RES_SUBMIT) {
      mu_set_focus(ctx, ctx->last_id);
      submitted = 1;
    }
    if (mu_button(ctx, "Submit")) { submitted = 1; }
    if (submitted) {
      write_log(buf);
      buf[0] = '\0';
    }

    mu_end_window(ctx);
  }
}

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low, int high) {
  static float tmp;
  mu_push_id(ctx, &value, sizeof(value));
  tmp = *value;
  int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
  *value = tmp;
  mu_pop_id(ctx);
  return res;
}


static void style_window(mu_Context *ctx, ren_rect_t rect) {
  static struct { const char *label; int idx; } colors[] = {
    { "text:",         MU_COLOR_TEXT        },
    { "border:",       MU_COLOR_BORDER      },
    { "windowbg:",     MU_COLOR_WINDOWBG    },
    { "titlebg:",      MU_COLOR_TITLEBG     },
    { "titletext:",    MU_COLOR_TITLETEXT   },
    { "panelbg:",      MU_COLOR_PANELBG     },
    { "button:",       MU_COLOR_BUTTON      },
    { "buttonhover:",  MU_COLOR_BUTTONHOVER },
    { "buttonfocus:",  MU_COLOR_BUTTONFOCUS },
    { "base:",         MU_COLOR_BASE        },
    { "basehover:",    MU_COLOR_BASEHOVER   },
    { "basefocus:",    MU_COLOR_BASEFOCUS   },
    { "scrollbase:",   MU_COLOR_SCROLLBASE  },
    { "scrollthumb:",  MU_COLOR_SCROLLTHUMB },
    { NULL }
  };

  if (mu_begin_window(ctx, "Style Editor",rect))
    // mu_rect(0, 0, 300, 1400)))
    // mu_rect(350, 250, 300, 240)))
  {
    int sw = mu_get_current_container(ctx)->body.w * 0.14;
    mu_layout_row(ctx, 6, (int[]) { 80, sw, sw, sw, sw, -1 }, 0);
    for (int i = 0; colors[i].label; i++) {
      mu_label(ctx, colors[i].label);
      uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
      uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
      mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
    }
    mu_end_window(ctx);
  }
}


//  nvgTransformPoint(float* dx, float* dy, const float* tf, float sx, float sy)
// 	    *dx = sx*tf[0] + sy*tf[2] + tf[4];
// 	    *dy = sx*tf[1] + sy*tf[3] + tf[5];
//  Identity
// 	    t[0] = 1.0f; t[1] = 0.0f;
// 	    t[2] = 0.0f; t[3] = 1.0f;
// 	    t[4] = 0.0f; t[5] = 0.0f;
//



typedef struct data_t {
	int fontNormal, fontBold, fontIcons, fontEmoji;
	int images[12];
} data_t;

int demo_img;
float demo_img_scale = 1.0;

int ren_load_img(const char* filename, int img_flags) {
    int handle = vg_img_create(ctx.vg, filename, img_flags);
    return handle;
}

void ren_free_img(int handle) {
    vg_img_free(ctx.vg, handle);
}

void ren_draw_img(ren_rect_t rect, int handle) {
    float anchor_x = (float)ctx.mouse_x;
    float anchor_y = (float)ctx.mouse_y;

    vg_t* vg = ctx.vg;
    float ix,iy,iw,ih,rect_ratio,img_ratio,alpha;
    alpha = 1.0f;
    float rect_x = (float)rect.x;
    float rect_y = (float)rect.y;
    float rect_w = (float)rect.w;
    float rect_h = (float)rect.h;
    float rect_r = rect_x + rect_w;
    float rect_b = rect_y + rect_h;
    anchor_x = (anchor_x <= rect_x) ? rect_x
             : (anchor_x >= rect_r) ? rect_r
             :  anchor_x;
    anchor_y = (anchor_y <= rect_y) ? rect_y
             : (anchor_y >= rect_b) ? rect_b
             :  anchor_y;
    vg_paint_t img_paint;
    int imgw, imgh;
    vg_img_size(vg, handle, &imgw, &imgh);
    img_ratio = (float)imgw/imgh;
    rect_ratio = rect_w/rect_h;

    demo_img_scale += (float)ctx.wheel_dy/100.0;
    if (img_ratio < rect_ratio) {
        iw = rect_w * demo_img_scale;
        ih = iw * img_ratio;
    } else {
        ih = rect_h * demo_img_scale;
        iw = ih * img_ratio;
    }
    ix = -(iw - rect_w) * (anchor_x - rect_x) / rect_w;
    iy = -(ih - rect_h) * (anchor_y - rect_y) / rect_h;

    // printf("anchor = %f, %f\n",anchor_x,anchor_y);



        // vg_tf_scale(vg, iw/rect_w, ih/rect_h);
        vg_tf_translate(vg, rect_x, rect_y);
        // img_paint = vg_img_pattern(vg, rect_x, rect_y, rect_w, rect_h, 0.0f, //0.0f/180.0f*NVG_PI,
        vg_clip(vg, 0, 0, rect_w, rect_h);
        img_paint = vg_img_pattern(vg, ix, iy, iw, ih, 0.0f, //0.0f/180.0f*NVG_PI,
            handle, alpha);
        vg_path(vg);
        vg_rect(vg, 0, 0, rect_w, rect_h); // nvgRoundedRect(vg, tx,ty, thumb,thumb, 5);
        vg_fill_paint(vg, img_paint);
        vg_fill(vg);

        // vg_clip(vg, rect_x, rect_y, rect_w, rect_h);
        // img_paint = vg_img_pattern(vg, rect_x + ix, rect_y + iy, iw, ih, 0.0f, //0.0f/180.0f*NVG_PI,
        //     handle, alpha);
        // vg_path(vg);
        // vg_rect(vg, rect_x, rect_y, rect_w, rect_h); // nvgRoundedRect(vg, tx,ty, thumb,thumb, 5);
        // vg_fill_paint(vg, img_paint);
        // vg_fill(vg);

    // vg_pop(vg);
    vg_tf_translate(vg, ix, iy);
    vg_tf_scale(vg, demo_img_scale, demo_img_scale);
}

vg_pathdata_t sym[8];

// draw_media_workspace()
// {
// }

void draw_path_data(void)
{
    vg_t* vg = ctx.vg;
    // vg_push(vg);

    // vg_tf_translate(vg, 800, 300);
    vg_stroke_w(vg,1.0);
    vg_stroke_color(vg, vg_rgba(0,0,255,255));
    vg_fill_color(vg, vg_rgba(255,0,0,255));

        vg_path(vg);
        vg_pathdata(vg, sym[0]);
        vg_stroke(vg);

        vg_path(vg);
        vg_moveto(vg,29,54);
        vg_lineto(vg,29,300);
        vg_lineto(vg,150,300);
        vg_stroke(vg);

        vg_path(vg);
        vg_pathdata(vg, sym[1]);
        vg_stroke(vg);

        vg_path(vg);
        vg_pathdata(vg, sym[2]);
        vg_stroke(vg);

        vg_path(vg);
        vg_pathdata(vg, sym[3]);
        vg_stroke(vg);

        // vg_tf_scale(vg, 5., 5.);
        vg_path(vg);
        vg_pathdata(vg, sym[4]);
        // vg_stroke(vg);
        vg_fill(vg);
    // vg_pop(vg);
}

static void render(void) {
    vg_t* vg = ctx.vg;
    vg_push(vg);
        ren_draw_img((ren_rect_t){.x=300,.y=80,.w=2000,.h=1120}, demo_img);
        draw_path_data();
    vg_pop(vg);
    mu_Context* mu = &ctx.mu;
    mu_begin(mu);
    // style_window(ctx, (ren_rect_t){.x=300,.y=1200,.w=2560,.h=200});
    // log_window(ctx);
    test_window(mu, (ren_rect_t){.x=0,.y=0,.w=300,.h=1400});
    mu_end(mu);
    ren_DRAW();
}

static void setup(ren_ctx_t* ctx) {
    data_init();

    win_max_size(ctx->win);
    ren_install_font_defaults();
    demo_img = ren_load_img(DEMO_IMG,
        // #ifndef GFX_WASM ?
        NVG_IMAGE_GENERATE_MIPMAPS
    );

    vg_pathdata_parse("M 25,50 l 0,250 L  150,300", &sym[0]);

    // testing bezier examples from https://css-tricks.com/svg-path-syntax-illustrated-guide/
    vg_pathdata_parse("M 25,50 C 25,100  150,100  150,50", &sym[1]);
    vg_pathdata_parse("M 25,100 C 25,150 75,150 75,100 S 100,25 150,75", &sym[2]);
    vg_pathdata_parse("M 25,75 Q 50,150 75,100 T 150,150", &sym[3]);

    // testing icon
    vg_pathdata_parse(
        // "M19.43 12.98, c.04-.32.07-.64.07-.98, s-.03-.66-.07-.98, l2.11-1.65, c.19-.15.24-.42.12-.64, l-2-3.46, c-.12-.22-.39-.3-.61-.22l-2.49 1c-.52-.4-1.08-.73-1.69-.98l-.38-2.65C14.46 2.18 14.25 2 14 2h-4c-.25 0-.46.18-.49.42l-.38 2.65c-.61.25-1.17.59-1.69.98l-2.49-1c-.23-.09-.49 0-.61.22l-2 3.46c-.13.22-.07.49.12.64l2.11 1.65c-.04.32-.07.65-.07.98s.03.66.07.98l-2.11 1.65c-.19.15-.24.42-.12.64l2 3.46c.12.22.39.3.61.22l2.49-1c.52.4 1.08.73 1.69.98l.38 2.65c.03.24.24.42.49.42h4c.25 0 .46-.18.49-.42l.38-2.65c.61-.25 1.17-.59 1.69-.98l2.49 1c.23.09.49 0 .61-.22l2-3.46c.12-.22.07-.49-.12-.64l-2.11-1.65zM12 15.5c-1.93 0-3.5-1.57-3.5-3.5s1.57-3.5 3.5-3.5 3.5 1.57 3.5 3.5-1.57 3.5-3.5 3.5z"
        "M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14zM7 9h5v1H7z"
    , &sym[4]);
}

static void teardown(void)
{
    ren_destroy();
}

static void loop()
{
    // ren_ctx_t* ctx
    if (ren_INPUT() != 0) {
    //  ren_delay(16);
    //   continue;
        ren_BEGIN();
        render();
        ren_END();
        ren_present();
    }
    REN_DELAY_FRAME()
}

int main(int argc, char **argv) {
    ren_ctx_t* ctx = ren_init_ctx();
    setup(ctx);
    REN_LOOP(loop)
    teardown();
    return 0;
}


