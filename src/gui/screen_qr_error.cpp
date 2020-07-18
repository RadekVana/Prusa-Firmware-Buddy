#include "screen_qr_error.hpp"
#include "config.h"
#include <stdlib.h>

#include "display.h"
#include "errors.h"

screen_qr_error_data_t::screen_qr_error_data_t()
    : window_frame_t(&errText)
    , errText(this, rect_ui16(8, 0, 224, 25))
    , errDescription(this, rect_ui16(8, 30, 224, 95))
    , info(this, rect_ui16(8, 275, 224, 20))
    , qr(this, rect_ui16(59, 140, 224, 95))
    , first_run_flag(true) {
    errText.SetBackColor(COLOR_RED_ALERT);
    errText.font = resource_font(IDR_FNT_BIG);
    errText.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)get_actual_error()->err_title));

    errDescription.SetBackColor(COLOR_RED_ALERT);
    errDescription.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)get_actual_error()->err_text));

    info.SetBackColor(COLOR_RED_ALERT);
    info.SetAlignment(ALIGN_CENTER);
    static const char hlp[] = "help.prusa3d.com";
    info.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)hlp));

    qr.px_per_module = 2;
    create_path_info_4error(qr_text.data(), qr_text.size(), 1);
    qr.text = qr_text.data();
}

void screen_qr_error_data_t::unconditionalDraw() {
    window_frame_t::unconditionalDraw();
    display::FillRect(rect_ui16(8, 25, 224, 2), COLOR_WHITE);
}

int screen_qr_error_data_t::event(window_t *sender, uint8_t event, void *param) {
    if ((event == WINDOW_EVENT_CLICK) || (event == WINDOW_EVENT_BTN_DN)) {
        screen_close();
        return (1);
    }
    if (!first_run_flag)
        return (0);
    first_run_flag = false;
    unconditionalDraw(); // todo why?
    return (0);
}
