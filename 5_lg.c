#ifndef GFX_ENV
    #define GFX_ENV
    #define GFX_MAC
    #define GFX_GL3
#endif
#include "gfx/gfx.h"
#define REN_MICROUI
#include "gfx/ren.h"


#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<time.h>
// #define	O_RDWR		0x0002
// #define	O_CREAT		0x0200
// #include "lemongraph.h"


// #define LG_FORCE_32BIT
#define LG_IMPLEMENTATION
#define LG_NO_AVL
#define LG_NO_AFSYNC
#include "lg/lib/api.h"


// const char * const  path = "./.tmp/graph";
// const int           mode = 0760;
// const int           os_flags = O_RDWR | O_CREAT;
//       int           db_flags = 0;

//       graph_t g;

LG_graph g;

int g_init() {
    // g = graph_open(path,
    //     os_flags, mode,
    //     db_flags//|DB_NOSYNC|DB_NORDAHEAD
    // );
    // if (g == NULL) {
    //     fprintf(stderr, graph_strerror(errno));
    //     return 1;
    // }
    lg_init("data/.lg/");
    lg_open(&g, "graph", LG_NO_TLS|LG_NO_LOCK|LG_NO_FSYNCMETA|LG_NO_FLUSH);    
    // 
    return 0;
}

static  char logbuf[64000];
static   int logbuf_updated = 1;
static float bg[3] = { 90, 95, 100 };

static logID_t msg_logid = 0;

#define NEEDS_UPDATE (logbuf_updated)

static void read_log() {
    logbuf[0] = '\0';  
    LG_READ(&g,
      msg_logid = lg_log_next(txn);            
      LG_iter it;
      LG_str n_type = str_ro_("msg");
      LG_id n_id;
      LG_Node node;
      LG_Blob blob;
      int i = 0; 
      nodes_type_b4_(&it, n_type, msg_logid);
      while (n_id = lg_iter_next(&it)) {  
          printf("msg_logid=%i, n_id:%i\n",msg_logid,n_id);
          ++i;  
          node_read_(n_id, &node);
          blob_read_(node.val, &blob);          
          strcat(logbuf, blob.string);
          strcat(logbuf, "\n");
      }
      lg_iter_close(&it);
      printf("msg_logid=%i, added:%i\n",msg_logid,i);
    )
    

#if LG_LEGACY_LEMONGRAPH
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
//   if (logbuf[0]) { strcat(logbuf, "\n"); }
//   strcat(logbuf, text);
  logbuf_updated = 1;
  LG_WRITE(&g,
    node_(str_("msg"), str_(text));
    msg_logid = lg_log_next(txn);
  )
#if LG_LEGACY_LEMONGRAPH
    graph_txn_t txn = graph_txn_begin(g, NULL, 0);
    if (txn == NULL) {
        return ;
    }

    node_t n1 = graph_node_resolve(txn, "msg",3, text,strlen(text));

    free(n1);

    msg_logid = graph_log_nextID(txn);
    graph_txn_commit(txn);
#endif
}

static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window(ctx, "Demo Window", mu_rect(40, 40, 300, 450))) {
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


static void style_window(mu_Context *ctx) {
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

  if (mu_begin_window(ctx, "Style Editor", mu_rect(350, 250, 300, 240))) {
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

static void render(mu_Context *ctx) {
  mu_begin(ctx);
  style_window(ctx);
  log_window(ctx);
  test_window(ctx);
  mu_end(ctx);
}


ren_ctx_t* ren_ctx;

static void loop()
{

    // if ((ren_INPUT()|NEEDS_UPDATE) == 0) {
    //   ren_delay(16);
    //   continue;
    // }
    // render(ctx);
    // ren_BEGIN();
    // ren_DRAW();
    // ren_END();
    // ren_present();

    if (ren_INPUT() != 0) {
      render(ren_ctx);
      ren_BEGIN();
      ren_DRAW();
      ren_END();
      ren_present();
    }
    REN_DELAY_FRAME()
}

int main(int argc, char **argv) {
  
  // https://github.com/microsoft/mimalloc/issues/308
  mi_reserve_os_memory( 1UL << 30 /*1GiB*/,  false /*commit*/, true /*allow large*/ );

  g_init();
  // read_log();
  ren_ctx = ren_init_ctx();
  SDL_MaximizeWindow(ren_ctx->win);
  ren_install_font_defaults();
  
  // for wasm
  REN_LOOP(loop)
  
  ren_destroy();
  return 0;
}


