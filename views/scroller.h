
#define SCROLL_THUMB_SIZE   12
#define SCROLL_THUMB_RADIUS (SCROLL_THUMB_SIZE>>1)

typedef struct 
Scroller_State 
{        
    bool    has_scrollbar_x;    bool    has_scrollbar_y;
    // int?
    float   scroll_x;       float   scroll_y;
    float   scroll_dx;      float   scroll_dy;
    float   scroll_w;       float   scroll_h;    
    Vec4f   thumb_rect;    
    Vec4f   track_rect;
} 
Scroller_State;

typedef struct 
Scroller_Props 
{    
    float       scroll_damp;
    SDL_Color   thumb_color;
} 
Scroller_Props;

static Scroller_Props Scroller_default_props = {
    .scroll_damp = 0.09f,
    .thumb_color = {255, 0, 0, 128}
};


#define SCROLLER_PRINT_STATE(state) \
    printf("scroller->state\n\thas_scrollbar_y:%i scroll_h:%f \n",state->has_scrollbar_y,state->scroll_h);



void 
scroller_init(int idx) 
{   ___    
    // TODO
    if (!doc.root_scroll_view)
        doc.root_scroll_view = idx;
    //
    View self = view_read(idx);
    if (self.props == NULL)
        self.props = &Scroller_default_props;       
    assert(self.state == NULL);
    Scroller_State* state = (Scroller_State*)calloc(1,sizeof(Scroller_State));    
    self.state = state;
    view_write(idx, self);
}



void 
scroller_measure(int idx) 
{   ___
    View* self = view_ref(idx);
    self->w = doc.vp_w;
    self->h = doc.vp_h;    
}

void 
scroller_layout(int idx) 
{   ___
    View* self = view_ref(idx);        
    Scroller_State* state = (Scroller_State*)self->state;
    Scroller_Props props = *(Scroller_Props*)self->props;
    const float _w = self->w;
    const float _h = self->h;

    float s_y = state->scroll_y;      
    
    // apply scroll delta
    if (state->scroll_dy != 0.0f) {
        state->scroll_dy -= (state->scroll_dy * props.scroll_damp);
        if (fabsf(state->scroll_dy) < 0.000001f)
            state->scroll_dy = 0.0f;
        s_y += state->scroll_dy;
        // state->scroll_dy = 0;
    }
    
    float s_h = 0.0f;    

    float s_x = state->scroll_x;
    float s_w = 0.0f;
    
    if (true) { // vertical        
        s_w = self->w;
        
                
        // TODO: move to layout routines... eventually constraint driven
        VIEW_FOR_EACH_SUB(*self) {
            View* subview = view_ref(sub);
            subview->rect.y = s_h + s_y;
            s_h += subview->rect.h;
        }        
        
        state->thumb_rect.w = SCROLL_THUMB_SIZE;
        state->thumb_rect.x = s_w - state->thumb_rect.w;
        
        float s_ymin = self->h + -s_h;        
        if (s_ymin >= 0.0f) {                     
            // no scrollbar               
            state->has_scrollbar_y = false;
            state->thumb_rect.y = 0.0f; 
            state->thumb_rect.h = 0.0f;
        }
        else {                      
            state->has_scrollbar_y = true;                                  
            // TODO: handle resize jitter            
            s_y = min(max(s_y, s_ymin), 0.0f);
            state->scroll_y = s_y;                                    
            state->thumb_rect.h = _h * _h / s_h;
            state->thumb_rect.y = MAP_RANGE(state->scroll_y, 0.0f, s_ymin, 
                0.0f, _h - state->thumb_rect.h);                         
        }        

        state->scroll_h = s_h;
    }

    

    // SCROLLER_PRINT_STATE(state);
}


void 
scroller_drawlayer(int idx, int layer)
{   ___    
    if (layer != LAYER_TOP) 
        return;
    View self = view_read(idx);
    Scroller_State* state = (Scroller_State*)self.state;
    Scroller_Props props = *(Scroller_Props*)self.state;
    
    if (!state->has_scrollbar_y)
        return;
    
    SDL_Color thumb_color = {90, 90, 90, 128} ;//props.thumb_color;
    
    float s_dy = fabsf(state->scroll_dy);
    if (s_dy <= 1.0f)
        thumb_color.a = MAP_RANGE(s_dy,1.0f,0.0f, (float)thumb_color.a, 0.0f);
    if (thumb_color.a > 0.0f)
        GPU_RectangleRoundFilled2(doc.target, state->thumb_rect.gpu_rect, SCROLL_THUMB_SIZE, thumb_color);    
}

Proto Scroller = {
    .init = &scroller_init,
    .measure = &scroller_measure,
    .layout = &scroller_layout,
    .drawlayer = &scroller_drawlayer
}; 


// CONSTRUCTOR(__scroller__) {
//     Scroller = (Proto){
//         .init = &scroller_init,
//         .measure = &scroller_measure,
//         .drawlayer = &drawlayer,
//     };    
// }
