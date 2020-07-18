// window_text.c
#include "window_text.hpp"
#include "gui.hpp"

void window_text_t::SetText(string_view_utf8 txt) {
    text = txt;
    Invalidate();
}

void window_text_t::SetTextColor(color_t clr) {
    color_text = clr;
    Invalidate();
}

void window_text_t::SetPadding(padding_ui8_t padd) {
    padding = padd;
    Invalidate();
}

void window_text_t::SetAlignment(uint8_t alignm) {
    alignment = alignm;
    Invalidate();
}

window_text_t::window_text_t(window_t *parent, rect_ui16_t rect, string_view_utf8 txt)
    : window_t(parent, rect)
    , color_text(gui_defaults.color_text)
    , font(gui_defaults.font)
    , text(txt)
    , padding(gui_defaults.padding)
    , alignment(gui_defaults.alignment) {
}

void window_text_t::unconditionalDraw() {
    render_text_align(rect, text, font,
        (IsFocused()) ? color_text : color_back,
        (IsFocused()) ? color_back : color_text,
        padding, alignment);
}
