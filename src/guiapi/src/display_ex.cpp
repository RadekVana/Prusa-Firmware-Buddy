// display_ex.cpp
#include "display_ex.hpp"
#include <functional>
#include <cmath>
#include "guiconfig.h" //USE_ST7789

#ifdef USE_ST7789
    #include "st7789v.h"
/*****************************************************************************/
//st7789v specific variables objects and function aliases
static constexpr Rect16 display_clip = { 0, 0, ST7789V_COLS, ST7789V_ROWS };

inline uint16_t color_to_native(uint32_t clr) {
    return color_to_565(clr);
}

extern "C" {

extern uint8_t st7789v_buff[ST7789V_COLS * 2 * ST7789V_BUFF_ROWS]; //16 lines buffer

extern void st7789v_draw_char_from_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

} //extern "C"

//TDispBuffer configuration
static constexpr size_t BuffLEN = 16;              // size of buffer (number of fields, not bytes)
using BuffDATA_TYPE = uint16_t;                    // type of buffer internally used in TDispBuffer
using BuffPTR_TYPE = uint16_t;                     // type of buffer internally used pointer (does not need to match BuffDATA_TYPE)
static constexpr size_t BuffNATIVE_PIXEL_SIZE = 1; // bytes per pixel (can be same or smaller than size of BuffDATA_TYPE)

static constexpr uint8_t *getBuff() { return st7789v_buff; }

static inline void drawCharFromBuff(point_ui16_t pt, uint16_t w, uint16_t h) { st7789v_draw_char_from_buffer(pt.x, pt.y, w, h); }

static inline void draw_from_buffer(Rect16 rect) {
    st7789v_draw_char_from_buffer(rect.TopLeft().x, rect.TopLeft().y, rect.Width(), rect.Height());
}

static inline void draw_to_buffer(Rect16 rect, uint16_t artefact_width, color_t color) {
    uint8_t *buff = st7789v_buff + (rect.Top() * artefact_width + rect.Left()) * 2;
    uint32_t clr = color_to_565(color);
    for (int i = 0; i < rect.Height(); i++) {
        int offset = i * artefact_width * 2;
        for (int j = 0; j < rect.Width(); j++) {
            buff[offset + (2 * j) + 0] = (uint8_t)clr;
            buff[offset + (2 * j) + 1] = (uint8_t)(clr << 8);
        }
    }
    return;
}

static inline void clear_buffer_line(int i, color_t back) {
    draw_to_buffer(Rect16(0, i, ST7789V_COLS, 1), ST7789V_COLS, back);
}

void display_ex_clear(const color_t clr) {
    st7789v_clear(color_to_565(clr));
}

static inline void draw_png_ex_C(uint16_t point_x, uint16_t point_y, FILE *pf, uint32_t clr0, uint8_t rop) {
    st7789v_draw_png_ex(point_x, point_y, pf, clr0, rop);
}

static inline uint32_t get_pixel_C(uint16_t point_x, uint16_t point_y) {
    return color_from_565(st7789v_get_pixel_colorFormat565(point_x, point_y));
}

static inline uint8_t *get_block_C(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y) {
    return st7789v_get_block(start_x, start_y, end_x, end_y);
}

static inline uint16_t get_pixel_directColor_C(uint16_t point_x, uint16_t point_y) {
    return st7789v_get_pixel_colorFormat565(point_x, point_y);
}

static inline void set_pixel_colorFormatNative(uint16_t point_x, uint16_t point_y, uint32_t nativeclr) {
    st7789v_set_pixel(point_x, point_y, nativeclr);
}

static inline void fill_rect_colorFormatNative(uint16_t rect_x, uint16_t rect_y, uint16_t rect_w, uint16_t rect_h, uint32_t nativeclr) {
    st7789v_fill_rect_colorFormat565(rect_x, rect_y, rect_w, rect_h, nativeclr);
}
//end st7789v specific variables objects and function aliases
/*****************************************************************************/
#endif //USE_ST7789

#ifdef USE_MOCK_DISPLAY
    #include "st7789v.h"
/*****************************************************************************/
//mock_display specific variables objects and function aliases
static constexpr Rect16 display_clip = { 0, 0, MockDisplay::Cols(), MockDisplay::Rows() };

inline uint32_t color_to_native(uint32_t clr) {
    return clr;
}

//TDispBuffer configuration
static constexpr size_t BuffLEN = 16;              // size of buffer (number of fields, not bytes)
using BuffDATA_TYPE = uint32_t;                    // type of buffer internally used in TDispBuffer
using BuffPTR_TYPE = uint32_t;                     // type of buffer internally used pointer (does not need to match BuffDATA_TYPE)
static constexpr size_t BuffNATIVE_PIXEL_SIZE = 1; // bytes per pixel (can be same or smaller than size of BuffDATA_TYPE)

using getBuff = MockDisplaygetBuff;

static inline void drawCharFromBuff(point_ui16_t pt, uint16_t w, uint16_t h) { st7789v_draw_char_from_buffer(pt.x, pt.y, w, h); }

static inline void draw_from_buffer(Rect16 rect) {
    st7789v_draw_char_from_buffer(rect.TopLeft().x, rect.TopLeft().y, rect.Width(), rect.Height());
}

static inline void draw_to_buffer(Rect16 rect, uint16_t artefact_width, color_t color) {
    uint8_t *buff = st7789v_buff + (rect.Top() * artefact_width + rect.Left()) * 2;
    uint32_t clr = color_to_565(color);
    for (int i = 0; i < rect.Height(); i++) {
        int offset = i * artefact_width * 2;
        for (int j = 0; j < rect.Width(); j++) {
            buff[offset + (2 * j) + 0] = (uint8_t)clr;
            buff[offset + (2 * j) + 1] = (uint8_t)(clr << 8);
        }
    }
    return;
}

void display_ex_clear(const color_t clr) {
    MockDisplay::clear(clr);
}

static inline void clear_buffer_line(int i, color_t back) {
    draw_to_buffer(Rect16(0, i, ST7789V_COLS, 1), ST7789V_COLS, back);
}

static inline void draw_png_ex_C(uint16_t point_x, uint16_t point_y, FILE *pf, uint32_t clr0, uint8_t rop) {
    st7789v_draw_png_ex(point_x, point_y, pf, clr0, rop);
}

static inline uint32_t get_pixel_C(uint16_t point_x, uint16_t point_y) {
    return color_from_565(st7789v_get_pixel_colorFormat565(point_x, point_y));
}

static inline uint8_t *get_block_C(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y) {
    return st7789v_get_block(start_x, start_y, end_x, end_y);
}

static inline uint16_t get_pixel_directColor_C(uint16_t point_x, uint16_t point_y) {
    return st7789v_get_pixel_colorFormat565(point_x, point_y);
}

static inline void set_pixel_colorFormatNative(uint16_t point_x, uint16_t point_y, uint32_t nativeclr) {
    st7789v_set_pixel(point_x, point_y, nativeclr);
}

static inline void fill_rect_colorFormatNative(uint16_t rect_x, uint16_t rect_y, uint16_t rect_w, uint16_t rect_h, uint32_t nativeclr) {
    st7789v_fill_rect_colorFormat565(rect_x, rect_y, rect_w, rect_h, nativeclr);
}

//end mock_display specific variables objects and function aliases
/*****************************************************************************/
#endif //USE_MOCK_DISPLAY

//do not use directly, use DispBuffer instead
template <size_t LEN, class DATA_TYPE, class PTR_TYPE, uint32_t NATIVE_PIXEL_SIZE>
class TDispBuffer {
    PTR_TYPE *p;
    DATA_TYPE clr_native[LEN];
    const color_t clr_bg;
    const color_t clr_fg;

public:
    TDispBuffer(uint8_t pms, color_t clr_bg, color_t clr_fg)
        : p((PTR_TYPE *)getBuff())
        , clr_bg(clr_bg)
        , clr_fg(clr_fg) {
        for (size_t i = 0; i <= pms; i++)
            clr_native[i] = color_to_native(color_alpha(clr_bg, clr_fg, 255 * i / pms));
    }
    void inline Insert(size_t pos) {
        *(DATA_TYPE *)p = clr_native[pos];
        p += NATIVE_PIXEL_SIZE;
    }
    void inline Draw(point_ui16_t pt, uint16_t w, uint16_t h) { drawCharFromBuff(pt, w, h); }
};
using DispBuffer = TDispBuffer<BuffLEN, BuffDATA_TYPE, BuffPTR_TYPE, BuffNATIVE_PIXEL_SIZE>;

static bool display_ex_draw_char(point_ui16_t pt, char chr, const font_t *pf, color_t clr_bg, color_t clr_fg) {
    const uint16_t w = pf->w; //char width
    const uint16_t h = pf->h; //char height
    // character out of font range, display solid rectangle instead
    if ((chr < pf->asc_min) || (chr > pf->asc_max)) {
        display_ex_fill_rect(Rect16(pt.x, pt.y, w, h), clr_bg);
        return false;
    }
    // here we only have an ASCII character, its location in font can be computed easily
    uint8_t charX = (chr - pf->asc_min) % 16;
    uint8_t charY = (chr - pf->asc_min) / 16;
    return display_ex_draw_charUnicode(pt, charX, charY, pf, clr_bg, clr_fg);
}

/// Draws a single character according to selected font
/// \param charX x-index of character in font bitmap
/// \param charY y-index of character in font bitmap
/// \param clr_bg background color
/// \param clr_fg font/foreground color
/// If font is not available for the character, solid rectangle will be drawn in background color
/// \returns true if character is available in the font and was drawn
bool display_ex_draw_charUnicode(point_ui16_t pt, uint8_t charX, uint8_t charY, const font_t *pf, color_t clr_bg, color_t clr_fg) {
    const uint16_t w = pf->w; //char width
    const uint16_t h = pf->h; //char height

    int i;
    int j;
    uint8_t *pch;    //character data pointer
    uint8_t crd = 0; //current row byte data
    uint8_t rb;      //row byte
    uint8_t *pc;

    const uint8_t bpr = pf->bpr;        //bytes per row
    const uint16_t bpc = bpr * h;       //bytes per char
    const uint8_t bpp = 8 * bpr / w;    //bits per pixel
    const uint8_t ppb = 8 / bpp;        //pixels per byte
    const uint8_t pms = (1 << bpp) - 1; //pixel mask

    DispBuffer buff(pms, clr_bg, clr_fg);

    uint32_t chr = charY * 16 + charX; // compute character index in font

    pch = (uint8_t *)(pf->pcs) + ((chr /*- pf->asc_min*/) * bpc);

    for (j = 0; j < h; j++) {
        pc = pch + j * bpr;
        for (i = 0; i < w; i++) {
            if ((i % ppb) == 0) {
                if (pf->flg & (uint32_t)FONT_FLG_SWAP) {
                    rb = (i / ppb) ^ 1;
                    crd = pch[rb + j * bpr];
                } else
                    crd = *(pc++);
            }
            if (pf->flg & (uint32_t)FONT_FLG_LSBF) {
                buff.Insert(crd & pms);
                crd >>= bpp;
            } else {
                buff.Insert(crd >> (8 - bpp));
                crd <<= bpp;
            }
        }
    }
    buff.Draw(pt, w, h);

    return true;
}

/// Draws a text into the specified rectangle @rc
/// If a character does not fit into the rectangle the drawing is stopped
/// \param clr_bg background color
/// \param clr_fg font/foreground color
/// \returns true if whole text was written
bool display_ex_draw_text(Rect16 rc, const char *str, const font_t *pf, color_t clr_bg, color_t clr_fg) {
    int x = rc.Left();
    int y = rc.Top();

    const uint16_t rc_end_x = rc.Left() + rc.Width();
    const uint16_t rc_end_y = rc.Top() + rc.Height();
    const uint16_t w = pf->w; //char width
    const uint16_t h = pf->h; //char height

    // prepare for stream processing
    char c = 0;
    while ((c = *str++) != 0) {
        if (c == '\n') {
            y += h;
            x = rc.Left();
            if (y + h > rc_end_y)
                return false;
            continue;
        }

        display_ex_draw_char(point_ui16(x, y), c, pf, clr_bg, clr_fg);
        x += w;
        // FIXME Shouldn't it try to break the line first?
        if (x + w > rc_end_x)
            return false;
    }
    return true;
}

/// Draws a rectangle boundary of defined color
void display_ex_draw_rect(Rect16 rc, color_t clr) {
    if (rc.IsEmpty())
        return;

    point_i16_t pt0 = rc.TopLeft();
    point_i16_t pt1 = { int16_t(rc.Left() + rc.Width() - 1), rc.Top() };
    point_i16_t pt2 = { rc.Left(), int16_t(rc.Top() + rc.Height() - 1) };

    display_ex_fill_rect(Rect16(pt0, rc.Width(), 1), clr);  // top
    display_ex_fill_rect(Rect16(pt0, 1, rc.Height()), clr); // left
    display_ex_fill_rect(Rect16(pt1, 1, rc.Height()), clr); // right
    display_ex_fill_rect(Rect16(pt2, rc.Width(), 1), clr);  // bottom
}

void display_ex_fill_rect(Rect16 rc, color_t clr) {
    rc = rc.Intersection(display_clip);
    if (rc.IsEmpty())
        return;
    const uint32_t native_color = color_to_native(clr);
    fill_rect_colorFormatNative(rc.Left(), rc.Top(), rc.Width(), rc.Height(), native_color);
}

/// Draws simple line (no antialiasing)
/// Both end points are drawn
void display_ex_draw_line(point_ui16_t pt0, point_ui16_t pt1, color_t clr) {
    const uint32_t native_color = color_to_native(clr);
    //todo check rectangle
    int n;
    const int dx = pt1.x - pt0.x; // X-axis dimension
    const int dy = pt1.y - pt0.y; // Y-axis dimension
    int cx = std::abs(dx);
    int cy = std::abs(dy);
    const int adx = cx; // absolute difference in x ( = width - 1)
    const int ady = cy; // absolute difference in y ( = height - 1)

    if ((adx == 0) || (ady == 0)) { // orthogonal line
        fill_rect_colorFormatNative(std::min(pt0.x, pt1.x), std::min(pt0.y, pt1.y), adx + 1, ady + 1, native_color);
        return;
    }

    const int sx = std::signbit(dx) ? -1 : 1; // X-axis direction, positive when left-to-right
    const int sy = std::signbit(dy) ? -1 : 1; // Y-axis direction, positive when bottom-to-top

    if (adx > ady) { // likely vertical line
        for (n = adx; n > 0; --n) {
            set_pixel_colorFormatNative(pt0.x, pt0.y, native_color);
            if ((cx -= cy) <= 0) {
                pt0.y += sy;
                cx += adx;
            }
            pt0.x += sx;
        }
        return;
    }

    if (adx < ady) { // likely horizontal line
        for (n = ady; n > 0; --n) {
            set_pixel_colorFormatNative(pt0.x, pt0.y, native_color);
            if ((cy -= cx) <= 0) {
                pt0.x += sx;
                cy += ady;
            }
            pt0.y += sy;
        }
        return;
    }

    //adx == ady => diagonal line
    for (n = adx; n > 0; --n) {
        set_pixel_colorFormatNative(pt0.x, pt0.y, native_color);
        pt0.x += sx;
        pt0.y += sy;
    }
}

color_t display_ex_get_pixel(point_ui16_t pt) {
    if (!display_clip.Contain(pt))
        return 0;
    return get_pixel_C(pt.x, pt.y);
}

uint8_t *display_ex_get_block(point_ui16_t start, point_ui16_t end) {
    if (!display_clip.Contain(start) || !display_clip.Contain(end))
        return NULL;
    return get_block_C(start.x, start.y, end.x, end.y);
}

/// Turns the specified pixel to the specified color
void display_ex_set_pixel(point_ui16_t pt, color_t clr) {
    if (!display_clip.Contain(pt))
        return;
    const uint32_t native_color = color_to_native(clr);
    set_pixel_colorFormatNative(pt.x, pt.y, native_color);
}

void display_ex_set_pixel_displayNativeColor(point_ui16_t pt, uint16_t noClr) {
    if (!display_clip.Contain(pt))
        return;
    set_pixel_colorFormatNative(pt.x, pt.y, noClr);
}

uint16_t display_ex_get_pixel_displayNativeColor(point_ui16_t pt) {
    if (!display_clip.Contain(pt))
        return 0;
    return get_pixel_directColor_C(pt.x, pt.y);
}

void display_ex_draw_icon(point_ui16_t pt, uint16_t id_res, color_t clr0, uint8_t rop) {
    FILE *pf = resource_fopen(id_res, "rb");
    draw_png_ex_C(pt.x, pt.y, pf, clr0, rop);
    fclose(pf);
}

void display_ex_draw_png(point_ui16_t pt, FILE *pf) {
    draw_png_ex_C(pt.x, pt.y, pf, 0, 0);
}
