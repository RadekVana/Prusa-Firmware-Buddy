#include "dbg.h"
#include "gui.hpp"
#include "config.h"
#include "window_header.hpp"
#include "status_footer.h"
#include "marlin_client.h"
#include "filament.h"
#include "marlin_server.h"
#include "guitypes.h"      //font_meas_text
#include "stm32f4xx_hal.h" //HAL_GetTick
#include "screens.h"
#include "../lang/i18n.h"

#define BUTTON_TUNE       0
#define BUTTON_PAUSE      1
#define BUTTON_DISCONNECT 2

enum item_id_t {
    iid_tune,
    iid_pause,
    iid_disconnect,
    iid_count // - MAIN COUNT INDEX for asert check
};

const uint16_t serial_printing_icons[iid_count] = {
    IDR_PNG_menu_icon_settings,
    IDR_PNG_menu_icon_pause,
    IDR_PNG_menu_icon_disconnect
};

const char *serial_printing_labels[iid_count] = {
    "Tune",
    "Pause",
    "Disconnect"
};

struct screen_printing_serial_data_t : public window_frame_t {
    window_header_t header;
    status_footer_t footer;

    window_icon_t octo_icon;

    window_icon_t w_buttons[iid_count];
    window_text_t w_labels[iid_count];

    int last_tick;
};

void screen_printing_serial_init(screen_t *screen);
void screen_printing_serial_done(screen_t *screen);
void screen_printing_serial_draw(screen_t *screen);
int screen_printing_serial_event(screen_t *screen, window_t *window, uint8_t event, void *param);

#define pw ((screen_printing_serial_data_t *)screen->pdata)

screen_t screen_printing_serial = {
    0,
    0,
    screen_printing_serial_init,
    screen_printing_serial_done,
    screen_printing_serial_draw,
    screen_printing_serial_event,
    sizeof(screen_printing_serial_data_t), //data_size
    0,                                     //pdata
};
screen_t *const get_scr_printing_serial() { return &screen_printing_serial; }

static void set_icon_and_label(item_id_t id_to_set, window_icon_t *p_button, window_text_t *lbl) {
    if (p_button->GetIdRes() != serial_printing_icons[id_to_set])
        p_button->SetIdRes(serial_printing_icons[id_to_set]);
    //compare pointers to text, compare texts would take too long
    if (lbl->GetText() != serial_printing_labels[id_to_set])
        lbl->SetText(serial_printing_labels[id_to_set]);
}

void screen_printing_serial_init(screen_t *screen) {
    pw->last_tick = 0;
    int16_t root = window_create_ptr(WINDOW_CLS_FRAME, -1,
        rect_ui16(0, 0, 0, 0), pw);
    window_create_ptr(WINDOW_CLS_HEADER, root, gui_defaults.header_sz, &(pw->header));
    p_window_header_set_icon(&(pw->header), IDR_PNG_status_icon_printing);
    p_window_header_set_text(&(pw->header), "SERIAL PRT.");

    //octo icon
    point_ui16_t pt_ico = icon_meas(resource_ptr(IDR_PNG_serial_printing));
    window_create_ptr(
        WINDOW_CLS_ICON, root,
        rect_ui16((240 - pt_ico.x) / 2, gui_defaults.scr_body_sz.y, pt_ico.x, pt_ico.y),
        &(pw->octo_icon));
    pw->octo_icon.Enable();
    pw->octo_icon.SetIdRes(IDR_PNG_serial_printing);
    pw->octo_icon.f_enabled = 0;
    pw->octo_icon.f_disabled = 0;

    for (unsigned int col = 0; col < iid_count; col++) {
        window_create_ptr(
            WINDOW_CLS_ICON, root,
            rect_ui16(8 + (15 + 64) * col, 185, 64, 64),
            &(pw->w_buttons[col]));
        //pw->w_buttons[col].SetBackColor(COLOR_GRAY); //this did not work before, do we want it?
        pw->w_buttons[col].SetTag(col + 1);
        pw->w_buttons[col].Enable();

        window_create_ptr(
            WINDOW_CLS_TEXT, root,
            rect_ui16(80 * col, 196 + 48 + 8, 80, 22),
            &(pw->w_labels[col]));
        pw->w_labels[col].font = resource_font(IDR_FNT_SMALL);
        pw->w_labels[col].SetPadding(padding_ui8(0, 0, 0, 0));
        pw->w_labels[col].SetAlignment(ALIGN_CENTER);
    }

    // -- CONTROLS
    window_icon_t *sp_button;
    // -- tune button
    static_assert(BUTTON_TUNE < iid_count, "BUTTON_TUNE not in range of buttons array");
    sp_button = &pw->w_buttons[BUTTON_TUNE];
    set_icon_and_label(iid_tune, sp_button, &pw->w_labels[BUTTON_TUNE]);
    // -- pause
    static_assert(BUTTON_PAUSE < iid_count, "BUTTON_PAUSE not in range of buttons array");
    sp_button = &pw->w_buttons[BUTTON_PAUSE];
    set_icon_and_label(iid_pause, sp_button, &pw->w_labels[BUTTON_PAUSE]);
    // -- disconnect
    static_assert(BUTTON_DISCONNECT < iid_count, "BUTTON_DISCONNECT not in range of buttons array");
    sp_button = &pw->w_buttons[BUTTON_DISCONNECT];
    set_icon_and_label(iid_disconnect, sp_button, &pw->w_labels[BUTTON_DISCONNECT]);

    status_footer_init(&(pw->footer), root);
}

void screen_printing_serial_done(screen_t *screen) {
    marlin_gcode("G27 P2"); /// park nozzle and raise Z axis
    marlin_gcode("M86 S1"); /// enable safety timer
    window_destroy(pw->id);
}

void screen_printing_serial_draw(screen_t *screen) {
}

int screen_printing_serial_event(screen_t *screen, window_t *window, uint8_t event, void *param) {
    window_header_events(&(pw->header));

    if (status_footer_event(&(pw->footer), window, event, param)) {
        return 1;
    }
    if (event != WINDOW_EVENT_CLICK) {
        return 0;
    }

    int p = reinterpret_cast<int>(param) - 1;
    switch (p) {
    case BUTTON_TUNE:
        screen_open(get_scr_menu_tune()->id);
        return 1;
        break;
    case BUTTON_PAUSE:
        marlin_gcode("M118 A1 action:pause");
        return 1;
        break;
    case BUTTON_DISCONNECT:
        if (gui_msgbox(_("Really Disconnect?"), MSGBOX_BTN_YESNO) == MSGBOX_RES_YES) {
            marlin_gcode("M118 A1 action:disconnect");
            screen_close();
        }
        return 1;
        break;
    }

    return 0;
}
