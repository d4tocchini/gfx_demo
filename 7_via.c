
// #define GFX_MAC
// #define GFX_DARWIN
// #define GFX_SDL
// #define GFX_GL33

#define GFX_SDL
#define GFX_WEBGL2

#include "gfx/gfx.h"
#define REN_MICROUI
#include "gfx/ren.h"

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<time.h>

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

/*
    https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/d
    " : 34
    \ \n \t : 32,10,9
    ,       : 44
    - . / 0-9 : 45,46,47,48-57
        A-a : 65-97
*/

#include <stdbool.h>

#define VG_D_NVALS_SHIFT        12
#define VG_D_NVALS_MASK         0b1111000000000000
#define VG_CLOSEPATH_NVALS      (1 << VG_D_NVALS_SHIFT)
#define VG_MOVETO_NVALS         (3 << VG_D_NVALS_SHIFT)
#define VG_LINETO_NVALS         (3 << VG_D_NVALS_SHIFT)
#define VG_BEZIERTO_NVALS       (7 << VG_D_NVALS_SHIFT)

#define VG_D_TYPE_SHIFT         16
#define _VG_D_MOVETO             ((NVG_MOVETO   << VG_D_TYPE_SHIFT) | VG_MOVETO_NVALS)
#define _VG_D_LINETO             ((NVG_LINETO   << VG_D_TYPE_SHIFT) | VG_LINETO_NVALS)
#define _VG_D_BEZIERTO           ((NVG_BEZIERTO << VG_D_TYPE_SHIFT) | VG_BEZIERTO_NVALS)

#define VG_D_CLOSE              ((NVG_CLOSE << VG_D_TYPE_SHIFT) | (1 << VG_D_NVALS_SHIFT))
#define VG_D_WINDING            ((NVG_WINDING << VG_D_TYPE_SHIFT) | (1 << VG_D_NVALS_SHIFT))

                                                      //||||____xxxx

// TODO: #define VG_ARCTO_ARGC

// #define VG_D_HORIZONTAL_SHIFT   4
// #define VG_D_VERTICAL_SHIFT     5
// #define VG_D_SMOOTH_SHIFT       6
// #define VG_D_RELATIVE_SHIFT     7
#define VG_D_HORIZONTAL         0b000000010000
#define VG_D_VERTICAL           0b000000100000
#define VG_D_RELATIVE           0b000001000000
#define VG_D_CUBIC              0b000010000000
#define VG_D_QUADRATIC          0b000100000000
#define VG_D_SMOOTH             0b001000000000

#define VG_D_ARGC_MASK          0b0000000000001111
#define VG_D_MOVETO             (_VG_D_MOVETO   | 2)
#define VG_D_LINETO             (_VG_D_LINETO   | 2)
#define VG_D_HLINETO            (_VG_D_LINETO   | 1 | VG_D_HORIZONTAL)
#define VG_D_VLINETO            (_VG_D_LINETO   | 1 | VG_D_VERTICAL)
#define VG_D_BEZIERTO           (_VG_D_BEZIERTO | 6 | VG_D_CUBIC)
#define VG_D_SBEZIERTO          (_VG_D_BEZIERTO | 4 | VG_D_CUBIC | VG_D_SMOOTH)
#define VG_D_QUADTO             (_VG_D_BEZIERTO | 4 | VG_D_QUADRATIC)
#define VG_D_SQUADTO            (_VG_D_BEZIERTO | 2 | VG_D_QUADRATIC | VG_D_SMOOTH)




#define _VG_D_LOG(...)
// #define _VG_D_LOG(...)           printf(__VA_ARGS__)
#define _VG_D_LOG_COMMAND(ch)    printf("%c\n", ch + 32*(!!(comflag & VG_D_RELATIVE)))
#define _VG_D_ERROR(...) { \
        fprintf(stderr, __VA_ARGS__); \
        return 1; \
    }
// int _vg_path_d_preparse(const char* commandstring, size_t len, int* out_commandlen, int* out_floatlen)
// {
//     int commandlen = 0;
//     int floatlen = 0;
//     int i = 0;
//     while (i < len) {
//         char ch = commandstring[i];
//         if (ch >= 'a') // lowercase == relative
//             ch -= 32; // force uppercase
//         switch (ch) {
//             case 'M':
//                 floatlen += VG_MOVETO_ARGC;
//                 break;
//             case 'L':
//             case 'H':
//             case 'V':
//                 floatlen += 1+VG_LINETO_ARGC;
//                 break;
//             case 'C':
//             case 'S':
//                 floatlen += 1+VG_BEZIERTO_ARGC;
//                 break;
//             case 'Z':
//                 floatlen += 1;
//                 break;
//             case 'Q':
//             case 'T':
//                 floatlen += 1+VG_QUADTO_ARGC;
//                 break;
//             case 'A': { // arcto (rx,ry, angle,arcflag,sweepflag, dx,dy)+
//                 _VG_D_ERROR("arcto command WIP");
//             }
//             default:
//                 continue;
//         }
//         ++commandlen;
//     }
//     *out_commandlen = commandlen;
//     *out_floatlen = floatlen;
// }

// _vg_path_d_comput_next(float* prev, float* next)
// {

// }


//  nvgTransformPoint(float* dx, float* dy, const float* tf, float sx, float sy)
// 	    *dx = sx*tf[0] + sy*tf[2] + tf[4];
// 	    *dy = sx*tf[1] + sy*tf[3] + tf[5];
//  Identity
// 	    t[0] = 1.0f; t[1] = 0.0f;
// 	    t[2] = 0.0f; t[3] = 1.0f;
// 	    t[4] = 0.0f; t[5] = 0.0f;
//

inline int _vg_grow_commands_if_needed(vg_t* ctx, int nvals) {
    if (ctx->ncommands+nvals > ctx->ccommands) {
      float* commands;
      int ccommands = ctx->ncommands+nvals + ctx->ccommands/2;
      commands = (float*)realloc(ctx->commands, sizeof(float)*ccommands);
      if (commands == NULL) return 1;
      ctx->commands = commands;
      ctx->ccommands = ccommands;
    }
    return 0;
}

#define _VG_TRANSFORM_PT_AT(vals, idx, tf) \
    vals[idx]   = vals[idx]*tf[0] + vals[idx+1]*tf[2] + tf[4]; \
    vals[idx+1] = vals[idx]*tf[1] + vals[idx+1]*tf[3] + tf[5];

#define VG_D_HEADER_LEN 2

typedef float* vg_path_data_t;

int vg_path_d(vg_t* ctx, vg_path_data_t buf)
{
    float* tf = _vg_get_transform(ctx);
    float* argv;
    int   argc = 0;
    int   argi = 0;

    int comflag = 0;
    float x = 0.0;
    float y = 0.0;
    float cx = 0.0;
    float cy = 0.0;

    int comflag0 = 0;
    float x0 = 0.0;
    float y0 = 0.0;
    float cx0 = 0.0;
    float cy0 = 0.0;
    int len = (int)buf[0] ;
    int commands_len = (int)buf[1];
    // if (len < 0)
    //   _VG_D_ERROR("vg_path_d invalid header[0] value: len\n")
    // _VG_D_LOG("vg_path_d len=%i commands_len=%i\n",len,commands_len);
    int i = VG_D_HEADER_LEN;
    int idx = 0;
    if (_vg_grow_commands_if_needed(ctx, len))
        _VG_D_ERROR("vg_path_d _vg_grow_commands_if_needed failed to grow\n")

    float* commands = ctx->commands + ctx->ncommands;
    ctx->ncommands += len - VG_D_HEADER_LEN;

    while (i < len) {
        comflag0 = comflag;
        x0 = x;
        y0 = y;
        cx0 = cx;
        cy0 = cy;

        argv = buf + i;
        comflag = (int)argv[0];
        argc = (comflag & VG_D_ARGC_MASK);

        int nvals = (comflag & VG_D_NVALS_MASK) >> VG_D_NVALS_SHIFT;
        float* vals = commands + idx;
        vals[0] = (float)(comflag >> VG_D_TYPE_SHIFT);
        _VG_D_LOG("  comflag=%i, i=%i, nvals=%i, argc=%i\n",comflag,i,nvals,argc);

        i += 1 + argc;
        idx += nvals;
        cx = 0.0;
        cy = 0.0;

        // CLOSE || WINDING
        if (nvals < 3)
            continue;

        bool not_horiz = 1 ^ (bool)(VG_D_HORIZONTAL & comflag);
        bool not_verti = 1 ^ (bool)(VG_D_VERTICAL & comflag);
        bool is_rel = (bool)(VG_D_RELATIVE & comflag);
        float x0_rel = x0 * is_rel;
        float y0_rel = y0 * is_rel;
        x = x0_rel + argv[argc - not_horiz] * not_verti;
        y = y0_rel + argv[argc] * not_horiz;

        vals[nvals-2] = x;
        vals[nvals-1] = y;

        _VG_D_LOG("\tx%f,y%f\n",x,y);

        // MOVETO || LINETO
        if (nvals == 3) {
            _VG_TRANSFORM_PT_AT(vals,1,tf)
            continue;
        }

        // BEZIERTO
        if (comflag & VG_D_CUBIC) {
            switch ((bool)(comflag & VG_D_SMOOTH)) {
                case false: {
                    vals[1] = x0_rel + argv[1];
                    vals[2] = y0_rel + argv[2];
                    vals[3] = x0_rel + argv[3];
                    vals[4] = y0_rel + argv[4];
                    _VG_D_LOG("\tcubic: c1%f,%f  c2%f,%f\n",argv[1],argv[2], argv[3],argv[4]);
                }   break;
                case true: {
                    bool prev_is_type = (bool)(comflag0 & VG_D_CUBIC);
                    // start control point is reflection of previous (if cubic)
                    vals[1] = x0 + (x0 - cx0) * prev_is_type;
                    vals[2] = y0 + (y0 - cy0) * prev_is_type;
                    // end control point
                    vals[3] = x0_rel + argv[1];
                    vals[4] = y0_rel + argv[2];
                }   break;
            }
            cx = vals[3];
            cy = vals[4];
            _VG_D_LOG("\tcubic: c1%f,%f  c2%f,%f\n",vals[1],vals[2], vals[3],vals[4]);

        }
        else if (comflag & VG_D_QUADRATIC) {
            switch ((bool)(comflag & VG_D_SMOOTH)) {
                case false: {
                    cx = x0_rel + argv[1];
                    cy = y0_rel + argv[2];
                }   break;
                case true: {
                    bool prev_is_type = (bool)(comflag0 & VG_D_QUADRATIC);
                    // control point is reflection of previous (if quadratic)
                    cx = x0 + (x0 - cx0) * prev_is_type;
                    cy = y0 + (y0 - cy0) * prev_is_type;
                }   break;
            }
            vals[1] = x0 + 2.0f/3.0f*(cx - x0);
            vals[2] = y0 + 2.0f/3.0f*(cy - y0);
            vals[3] = x + 2.0f/3.0f*(cx - x);
            vals[4] = y + 2.0f/3.0f*(cy - y);
        }
        _VG_TRANSFORM_PT_AT(vals,1, tf)
        _VG_TRANSFORM_PT_AT(vals,3, tf)
        _VG_TRANSFORM_PT_AT(vals,5, tf)
    }
    _VG_D_LOG("done looping...\n");
    ctx->commandx = x;
    ctx->commandy = y;
    // memcpy(&ctx->commands[ctx->ncommands], vals, nvals*sizeof(float));
    return 0;
}

#define _vg_path_d_parse(commandstring, out)  vg_path_d_parse_n(commandstring, strlen(commandstring), out)
#define  vg_path_d_parse(commandstring, out) _vg_path_d_parse((commandstring),(out))
int      vg_path_d_parse_n(const char* commandstring, int len, vg_path_data_t* out)
{
    float argv[len]; // max number
    int argi = VG_D_HEADER_LEN; // 0: float count, 1: command count
    int command_count = 0;

    #define _VG_D_PARSE_SUCCESS { \
        argv[0] = (float)argi; \
        argv[1] = (float)command_count; \
        _VG_D_LOG("\nfloat_count=%i, command_count=%i\n",argi,command_count); \
        *out = malloc(argi*sizeof(float)); \
        if (*out == NULL) _VG_D_ERROR("ERROR vg_path_d cannot allocate for out buffer\n"); \
        _VG_D_LOG("bufflen=%i\n",argi*sizeof(float)); \
        memcpy(*out, argv, argi*sizeof(float)); \
        _VG_D_LOG("\nfloat_count=%i, command_count=%i\n",(int)((*out)[0]),(int)((*out)[1])); \
        return 0; \
    }

    int vals_offset = 0;
    int comflag = 0;
    int numc = 0;
    // int argc = 0;
    int i = 0;
    while (i < len) {
        char ch = commandstring[i];
        while (ch <=',') { // skip whitespace
            if (i == len)
                _VG_D_PARSE_SUCCESS
            ch = commandstring[++i];
        }
        if (ch <= '9') { // extract floats
            if (numc == (comflag & VG_D_ARGC_MASK)) { // implicit command repeat
                ++command_count;
                argv[argi++] = (float)comflag;
                numc = 0;
                _VG_D_LOG("\n");
            }
            if (comflag == 0)
                _VG_D_ERROR("ERROR vg_path_d cannot start command with number\n");
            const char* end;
            float val = strtof(commandstring + i, &end);
            i += end - (commandstring + i);

            argv[argi++] = val;
            numc++;

            _VG_D_LOG("\t%f\n", argv[argi-1]);
            continue;
        }
        if (numc != (comflag & VG_D_ARGC_MASK))
            _VG_D_ERROR("ERROR vg_path_d param count is %i, expects %i. ch='%c'\n",numc,(comflag & VG_D_ARGC_MASK), ch);
        numc = 0;
        comflag = 0;
        if (ch >= 'a') { // lowercase == relative
            comflag |= VG_D_RELATIVE;
            ch -= 32; // force uppercase
        }
        ++command_count;
        _VG_D_LOG_COMMAND(ch);
        switch (ch) {
            case 'M': { // moveto (dx,dy)+
                comflag |= VG_D_MOVETO;
            }   break;
            case 'L': { // lineto (dx,dy)+
                comflag |= VG_D_LINETO;
            }   break;
            case 'H': { // hlineto (dx)+
                comflag |= VG_D_HLINETO;
            }   break;
            case 'V': { // vlineto (dy)+
                comflag |= VG_D_VLINETO;
            }   break;
            case 'C': { // bezierto (dx1,dy1, dx2,dy2, dx,dy)+
                comflag |= VG_D_BEZIERTO;
            }   break;
            case 'S': { // smooth bezierto (dx2,dy2, dx,dy)+
                comflag |= VG_D_SBEZIERTO;
            }   break;
            case 'Z': { // closepath ()
                comflag |= VG_D_CLOSE;
            }   break;
            case 'Q': { // quadto (dx1,dy1, dx,dy)+
                comflag |= VG_D_QUADTO;
            }   break;
            case 'T': { // smooth quadto (dx,dy)+
                comflag |= VG_D_SQUADTO;
            }   break;
            // case 'A': { // arcto (rx,ry, angle,arcflag,sweepflag, dx,dy)+
            //     // comflag |= 7;
            //     _VG_D_ERROR("arcto command WIP");
            // }
            default: _VG_D_ERROR("ERROR vg_path_d unrecognized command : %c\n",ch);
        }
        i++;
        _VG_D_LOG("  comflag=%i, argi=%i\n", comflag, argi);
        argv[argi++] = (float)comflag;
    }
    _VG_D_PARSE_SUCCESS
}

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
    vg_t* vg = ctx.vg;
    float ix,iy,iw,ih,rect_ratio,img_ratio,alpha;
    alpha = 1.0f;
    float rect_x = (float)rect.x;
    float rect_y = (float)rect.y;
    float rect_w = (float)rect.w;
    float rect_h = (float)rect.h;
    float rect_r = rect_x + rect_w;
    float rect_b = rect_y + rect_h;
    float anchor_x = (float)ctx.mouse_x;
    float anchor_y = (float)ctx.mouse_y;
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

    vg_push(vg);
        vg_clip(vg, rect_x, rect_y, rect_w, rect_h);
        // vg_tf_scale(vg, iw/rect_w, ih/rect_h);
        // vg_tf_translate(vg,
        //     ix, ih);
        // img_paint = vg_img_pattern(vg, rect_x, rect_y, rect_w, rect_h, 0.0f, //0.0f/180.0f*NVG_PI,
        img_paint = vg_img_pattern(vg, rect_x + ix, rect_y + iy, iw, ih, 0.0f, //0.0f/180.0f*NVG_PI,
            handle, alpha);
        vg_path(vg);
        vg_rect(vg, rect_x, rect_y, rect_w, rect_h); // nvgRoundedRect(vg, tx,ty, thumb,thumb, 5);
        vg_fill_paint(vg, img_paint);
        vg_fill(vg);

    vg_pop(vg);
}

vg_path_data_t sym[8];

// draw_media_workspace()
// {

// }

void draw_path_data(void)
{
    vg_t* vg = ctx.vg;
    vg_tf_scale(vg, demo_img_scale, demo_img_scale);
    vg_push(vg);
    // vg_tf_translate(vg, 800, 300);
    vg_stroke_w(vg,1.0);
    vg_stroke_color(vg, vg_rgba(0,0,255,255));
    vg_fill_color(vg, vg_rgba(255,0,0,255));

        vg_path(vg);
        vg_path_d(vg, sym[0]);
        vg_stroke(vg);

        vg_path(vg);
        vg_moveto(vg,29,54);
        vg_lineto(vg,29,300);
        vg_lineto(vg,150,300);
        vg_stroke(vg);

        vg_path(vg);
        vg_path_d(vg, sym[1]);
        vg_stroke(vg);

        vg_path(vg);
        vg_path_d(vg, sym[2]);
        vg_stroke(vg);

        vg_path(vg);
        vg_path_d(vg, sym[3]);
        vg_stroke(vg);

        // vg_tf_scale(vg, 5., 5.);
        vg_path(vg);
        vg_path_d(vg, sym[4]);
        // vg_stroke(vg);
        vg_fill(vg);
    vg_pop(vg);
}

static void render(void) {
    ren_draw_img((ren_rect_t){.x=300,.y=80,.w=2000,.h=1120}, demo_img);

    mu_Context* mu = &ctx.mu;
    mu_begin(mu);
    // style_window(ctx, (ren_rect_t){.x=300,.y=1200,.w=2560,.h=200});
    // log_window(ctx);
    test_window(mu, (ren_rect_t){.x=0,.y=0,.w=300,.h=1400});
    mu_end(mu);
    ren_DRAW();
    draw_path_data();
}

static void setup(ren_ctx_t* ctx) {
    data_init();

    SDL_MaximizeWindow(ctx->win);
    ren_install_font_defaults();
    demo_img = ren_load_img("./data/img/d5.png",
        // #ifndef GFX_WASM ?
        NVG_IMAGE_GENERATE_MIPMAPS
    );

    vg_path_d_parse("M 25,50 l 0,250 L  150,300", &sym[0]);

    // testing bezier examples from https://css-tricks.com/svg-path-syntax-illustrated-guide/
    vg_path_d_parse("M 25,50 C 25,100  150,100  150,50", &sym[1]);
    vg_path_d_parse("M 25,100 C 25,150 75,150 75,100 S 100,25 150,75", &sym[2]);
    vg_path_d_parse("M 25,75 Q 50,150 75,100 T 150,150", &sym[3]);

    // testing icon
    vg_path_d_parse(
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


