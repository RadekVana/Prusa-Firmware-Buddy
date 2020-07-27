// screen_test_msgbox.cpp

#include "screen_test_msgbox.hpp"
#include "config.h"
#include "../lang/i18n.h"
#include "ScreenHandler.hpp"
#include "window_msgbox.hpp"

static const char *test_text = N_("Welcome to the Original Prusa MINI setup wizard. Would you like to continue?");
static const string_view_utf8 test_text_view = string_view_utf8::MakeCPUFLASH((const uint8_t *)(test_text));
static const char *test_header = N_("Header");
static const string_view_utf8 test_header_view = string_view_utf8::MakeCPUFLASH((const uint8_t *)(test_header));

screen_test_msgbox_data_t::screen_test_msgbox_data_t()
    : window_frame_t()
    , tst(this, rect_ui16(10, 32, 220, 22))
    , back(this, rect_ui16(10, 54, 220, 22), is_closed_on_click_t::yes)
    , tst_ok(this, rect_ui16(10, 76, 220, 22), []() { MsgBox(Responses_Ok, test_text_view); })
    , tst_okcancel(this, rect_ui16(10, 98, 220, 22), []() { MsgBox(Responses_OkCancel, test_text_view); })
    , tst_ico_error(this, rect_ui16(10, 120, 220, 22), []() { MsgBoxError(Responses_AbortRetryIgnore, test_text_view); })
    , tst_ico_question(this, rect_ui16(10, 142, 220, 22), []() { MsgBoxQuestion(Responses_YesNoCancel, test_text_view); })
    , tst_ico_warning(this, rect_ui16(10, 164, 220, 22), []() { MsgBoxWarning(Responses_YesNo, test_text_view); })
    , tst_ico_info(this, rect_ui16(10, 186, 220, 22), []() { MsgBoxQuestion(Responses_RetryCancel, test_text_view); })
    , tst_icon(this, rect_ui16(10, 208, 220, 22), []() { MsgBoxQuestion(Responses_Ok, test_text_view); }) {
    static const char tm[] = "TEST MSGBOX";
    tst.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)tm));

    static const char bck[] = "back";
    back.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)bck));

    static const char ok[] = "OK";
    tst_ok.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)ok));

    static const char oc[] = "OK-CANCEL";
    tst_okcancel.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)oc));

    static const char er[] = "ERROR";
    tst_ico_error.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)er));

    static const char qu[] = "QUESTION";
    tst_ico_question.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)qu));

    static const char wa[] = "WARNING";
    tst_ico_warning.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)wa));

    static const char in[] = "INFO";
    tst_ico_info.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)in));

    static const char ic[] = "ICON";
    tst_icon.SetText(string_view_utf8::MakeCPUFLASH((const uint8_t *)ic));
}
