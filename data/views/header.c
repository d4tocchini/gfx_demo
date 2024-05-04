
typedef struct Header_State {    
    Vec4f       bg_rect;
    VG_Image    bg_img;
    Vec4f       title_rect;
    VG_Image    title_img;    
    Vec4f       logo_rect;
    GPU_Image*   logo_img;

    // Vec4f       noise_rect;
    // GPU_Image*   noise_img;
    // VG_Image* txt_layer;
    // VG_Image* fg_layer;
} Header_State;

typedef struct Header_Props {    
    char* title;
    char* logo_path;
    int   logo_w;
    int   logo_h;

    // char* noise_path;
} Header_Props;

static Header_Props Header_default_props = {
    .title = "LET'S TURN LEADERSHIT INTO LEADERSHIP",
    .logo_path = "data/img/logo.png",    
    .logo_w = 360,
    .logo_h = 270        
};

void
_header_draw_bg(
    vg_t* _vg, 
    Vec4f rect
){  ___
    
    const float _x = 0; const float _y = 0;
    const float _w = rect.w; const float _h = rect.h;
    static float square_r = 5.0f;
    
    // vg_reset(_vg);
    // vg_push(_vg);
    
    vg_path(_vg);
    vg_rect(_vg, _x, _y, _w, _h);
    // vg_rrect(_vg, _x, _y, _w, _h, ++square_r);
    // vg_fill_paint(_vg, 
    //     vg_gradient(_vg, _x, _y, _x+_w, _y+_h, 
    //         vg_rgba(255, 255, 255, 255), 
    //         vg_rgba(255, 255, 255, 155))
    // );
    vg_fill_color(_vg, vg_rgba(20,20,20, 255));
    vg_fill(_vg);

    // vg_pop(_vg);    
    // vg_push(_vg);
    vg_reset(_vg);
    
    {
        float x = doc.vp_w/6;
        float y = doc.vp_h*5/18;
        float w = doc.vp_w*4/6;
        vg_font_name(_vg, "display");
        vg_font_size(_vg, 80.0f);
        vg_text_lineh(_vg, 1.1f);
        vg_text_tracking(_vg, 20.0f);
        vg_text_align(_vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP);
        // nvgFillColor(_vg, nvgRGBA(0,0,0,220));
        vg_fill_paint(_vg, 
            vg_gradient(_vg, x, y, x+w, y+80.0f, 
                vg_rgba(163, 108, 36, 255),
                vg_rgba(255, 0, 0, 255)
            )
        );
        vg_textbox(_vg, x,y, w, "WE TURN LEADERSHIT INTO LEADERSHIP", NULL);    
        
        // vg_pop(_vg);
        vg_reset(_vg);
    }
}

void
_header_draw_title(
    vg_t* _vg, 
    Vec4f rect
){  ___
    // const float _x = 0; const float _y = 0;
    // const float _w = rect.w; const float _h = rect.h;
    // static float square_r = 5.0f;
    // vg_font_name("display");
    // vg_font_size(vg, 40.0f);
    // vg_text_lineh(vg, 60.0f);
    // vg_text_align(vg, NVG_ALIGN_CENTER|NVG_ALIGN_TOP)
    // vg_text(vg,"LET'S TURN LEADERSHIT INTO LEADERSHIP");
    // vg_textbox(vg, doc.vp_w/6, doc.vp_h/6, doc.vp_w*4/6, "LET'S TURN LEADERSHIT INTO LEADERSHIP", NULL);
    // nvgBeginPath(_vg);
    // nvgRoundedRect(_vg, _x, _y, _w, _h, ++square_r);
    // nvgFillPaint(_vg, 
    //     nvgLinearGradient(_vg, _x, _y, _x+_w, _y+_h, 
    //         nvgRGBA(255, 255, 255, 255), 
    //         nvgRGBA(255, 255, 255, 155))
    // );
    // nvgFill(_vg);
}

void header_init(int idx) 
{   ___
    View self = view_read(idx);

    if (self.props == NULL)
        self.props = &Header_default_props;       

    assert(self.state == NULL);
    Header_State* state = (Header_State*)calloc(1,sizeof(Header_State));    
    self.state = state;
    state->bg_img.draw = &_header_draw_bg;
    view_write(idx, self);
}

void header_measure(int idx) 
{   ___
    View* self = view_ref(idx);    
    self->rect.w = doc.vp_w;
    self->rect.h = doc.vp_h;    
}

void header_layout(int idx) 
{   ___
    View self = view_read(idx);            
    Header_State* state = (Header_State*)self.state;        
    Header_Props props = *(Header_Props*)self.props;    
    // bg 
    state->bg_rect = self.rect;  
    // logo
    {        
        float w = self.rect.w / 24.0f;
        float h = w * props.logo_h / props.logo_w;
        float x = (self.rect.w / 2.0) - (w / 2.0);
        float y = self.rect.h / 9.0f;
        y+=self.rect.y;
        x+=self.rect.x;
        state->logo_rect = (Vec4f){.x=x,.y=y,.w=w,.h=h};        
    }    
}

void header_drawlayer(int idx, int layer)
{   ___    
    View self = view_read(idx);

    if ((self.rect.y > doc.vp_h) || (self.rect.y + self.rect.h < 0))
        return;

    Header_State* state = (Header_State*)self.state;
    // const float _w = self.rect.w;
    // const float _h = self.rect.h;
    // printf("\tview %i",idx);
    // printf("\tstate.bg_rect {.x=%f, .y=%f .w=%f, .h=%f}\n",state->bg_rect.x, state->bg_rect.y, state->bg_rect.w,state->bg_rect.h);
    if (layer == LAYER_OFFSCREEN) {        
        // logo
        Header_Props* props = (Header_Props*)self.props;    
        if (NULL == state->logo_img) {    
            state->logo_img = GPU_LoadImage(props->logo_path);
        }
        // if (NULL == state->noise_img) {            
        //     state->noise_img = GPU_LoadImage(props->noise_path);
        //     GPU_SetBlending(state->noise_img,true);
        //     GPU_SetBlendMode(state->noise_img, GPU_BLEND_SUBTRACT);
        //     GPU_SetWrapMode(state->noise_img, GPU_WRAP_REPEAT, GPU_WRAP_REPEAT);
        // }
        // bg
        VG_Image bg_img = state->bg_img;
        if (NULL == state->bg_img.fb) {
            const Vec4f rect = state->bg_rect;
            const float _w = rect.w;
            const float _h = rect.h;
            const float px_ratio = doc.px_ratio;
            vg_t* vg = doc.vg;
            // init fb
            vg_fb_t* fb = vg_fb_create(vg, _w, _h, 0); //NVG_IMAGE_NODELETE
            if (fb == NULL) {printf("Could not create FBO.\n"); exit(1);}
            bg_img.fb = fb;
            // begin_draw
            vg_fb_bind(fb); glViewport(0, 0, _w, _h); glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);            
            GPU_FlushBlitBuffer(); // ??? IMPORTANT: run GPU_FlushBlitBuffer before nvgBeginFrame
            vg_frame_begin(vg, _w, _h, px_ratio);
            // draw
            bg_img.draw(vg, rect);
            // end
            vg_frame_end(vg); vg_fb_bind(NULL); GPU_ResetRendererState();
            bg_img.image = GPU_CreateImageUsingTexture3(fb->texture, false, _w, _h, GL_RGBA); // should take_ownership be true?            
            assert(bg_img.image != NULL);       
            // write state
            state->bg_img = bg_img;
        }
        // else {
        //     const Vec4f rect = state->bg_rect;
        //     const float _w = rect.w;
        //     const float _h = rect.h;
        //     const float px_ratio = doc.px_ratio;
        //     vg_t* vg = doc.vg;
        //     // begin_draw
        //     vg_fb_bind(bg_img.fb); glViewport(0, 0, _w, _h); glClearColor(0, 0, 0, 0); glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);            
        //     GPU_FlushBlitBuffer(); // ??? IMPORTANT: run GPU_FlushBlitBuffer before nvgBeginFrame
        //     vg_frame_begin(vg, _w, _h, px_ratio);
        //     // draw
        //     bg_img.draw(vg, rect);
        //     // end
        //     vg_frame_end(vg); vg_fb_bind(NULL); GPU_ResetRendererState();        
        // }
        return;
    }

    // GPU_SetDepthTest(doc.target, 1);
    // GPU_MatrixMode(doc.target, GPU_MODEL);
    // GPU_LoadIdentity();
    // GPU_PushMatrix();
    // GPU_Translate(40, 100.0f * (idx&1), 100.0f * (idx&1));
    // GPU_PushMatrix();
    // GPU_Translate(0, 40, 0.0f);
    // GPU_PopMatrix();

    switch (layer) 
    {    
        case 0: 
        {
            VG_Image bg_img = state->bg_img;
            assert(bg_img.fb != NULL && state->bg_rect.w > 0.0f);
            static float q = 0.0f;
            q++;
            GPU_BlitRectX(bg_img.image, NULL, doc.target, &state->bg_rect, 0.0f, 0.0f, 0.0f, GPU_FLIP_VERTICAL); // IMPORTANT: GPU_BlitRectX is required to use GPU_FLIP_VERTICAL which is required for NVGLUframebuffer data (why???)
        // }   break;
        // case 1: {        
            // GPU_SetRGBA(state->logo_img, 0,0,0,255);
            // GPU_GenerateMipmaps(state->logo_img);
            // GPU_SetBlending(state->logo_img,true);
            // GPU_SetBlendMode(state->logo_img, GPU_BLEND_PREMULTIPLIED_ALPHA);
            GPU_BlitRectX(state->logo_img, NULL, doc.target, &state->logo_rect, 0.0f, 0.0f, 0.0f, 0); // IMPORTANT: GPU_BlitRectX is required to use GPU_FLIP_VERTICAL which is required for NVGLUframebuffer data (why???)            
        }   break;
        case LAYER_TOP:
        {            
            // GPU_BlitRectX(state->noise_img, &state->bg_rect, doc.target, NULL, 0.0f, 0.0f, 0.0f, 0);
        }
    }    
    
    // GPU_PopMatrix();
    // GPU_SetDepthTest(doc.target, 0);
}

Proto Header = {
    .init = &header_init,
    .measure = &header_measure,
    .layout = &header_layout,
    .drawlayer = &header_drawlayer
}; 




// CONSTRUCTOR(__header__) {
//     Header = (Proto){
//         .init = &header_init,
//         .measure = &header_measure,
//         .drawlayer = &drawlayer,
//     };    
// }
