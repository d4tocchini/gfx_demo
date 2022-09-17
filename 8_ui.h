
// typedef struct view_t view_t;
// // typedef struct win_t win_t;
// // https://github.com/yue/yue/blob/master/nativeui/view.h
// struct view_t {
//     uint32_t    flags;
//     uint32_t    win_id;
//     view_t**    children;
//     view_t*     parent;
// } ;

typedef struct UI_el {
    
    uint32_t    proto;
    uint32_t    flags;


} UI_ctx;

typedef struct UI_ctx {
    UI_el root;
    
    avl_tree_t ___ proto_map;    



} UI_ctx;

UI_ctx* 
ui_init(void) {

}

void
el_scrollable_init(UI_ctx* ui, uint32_t idx) {
    ui_set_el_eventflags(idx, UI_ON_SCROLL|UI_ON_MOUSE);    
}

void
el_scrollable_onevent(UI_ctx* ui, uint32_t idx) {
    
}

void
el_scrollable_measure(UI_ctx* ui, uint32_t idx) {

}

el_scrollable_layout() {

}

el_scrollable_display() {

}


void
site_init(void)
{
    // emscripten_set_resize_callback  https://emscripten.org/docs/api_reference/html5.h.html#ui
    
    UI_ctx* ui = ui_init();
    
    ui_register_el_proto(ui,"scrollable",(UI_el_proto){
        .init = &el_scrollable_init,
        .measure = &el_scrollable_measure,
        .layout = &el_scrollable_layout,
        .display = &el_scrollable_display
    });
    ui_register_el_proto(ui,"site_header",(UI_el_proto){
        .init = &site_header_init,
        .measure = &site_header_measure,        
        .display = &site_header_display
    });

    UI_el scrollable = ui_create_el(ui, "scrollable");
    UI_el header = ui_create_el(ui, "site_header");

    ui_append_el(ui, scrollable, header);

    ui_append_el(ui, UI_ROOT, scrollable);

    
}


void
site_header_draw() 
{

}