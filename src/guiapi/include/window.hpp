//window.hpp
#pragma once

#include <inttypes.h>
#include "guitypes.h"
#include <array>
#include "../../lang/string_view_utf8.hpp"

//window class identifiers
#define WINDOW_CLS_FRAME 0 // FRAME - basic container class
#define WINDOW_CLS_TEXT  1 // TEXT - aligned singlecolor text
#define WINDOW_CLS_NUMB  2 // NUMB - aligned singlecolor formated number
#define WINDOW_CLS_ICON  3 // ICON - small image with left-top offset
#define WINDOW_CLS_LIST  4 // LIST - vertical or horizontal list (text-icon pairs)

//window events
#define WINDOW_EVENT_BTN_DN   0x01 //button down
#define WINDOW_EVENT_BTN_UP   0x02 //button up
#define WINDOW_EVENT_ENC_DN   0x03 //encoder minus
#define WINDOW_EVENT_ENC_UP   0x04 //encoder plus
#define WINDOW_EVENT_FOCUS0   0x05 //focus lost
#define WINDOW_EVENT_FOCUS1   0x06 //focus set
#define WINDOW_EVENT_CAPT_0   0x07 //capture lost
#define WINDOW_EVENT_CAPT_1   0x08 //capture set
#define WINDOW_EVENT_CLICK    0x09 //clicked (tag > 0)
#define WINDOW_EVENT_CHANGE   0x0a //value/index changed (tag > 0)
#define WINDOW_EVENT_CHANGING 0x0b //value/index changing (tag > 0)
#define WINDOW_EVENT_LOOP     0x0c //gui loop (every 50ms)
#define WINDOW_EVENT_TIMER    0x0d //gui timer
#define WINDOW_EVENT_MESSAGE  0x0e //onStatusChange() message notification

struct window_list_t;
typedef void(window_list_item_t)(window_list_t *pwindow_list,
    uint16_t index, const char **pptext, uint16_t *pid_icon);

class window_t {
    window_t *parent;
    window_t *next;
    uint8_t f_tag;

protected:
    union {
        uint8_t flg;
        struct {
            bool f_visible : 1;         // is visible
            bool f_enabled : 1;         // is enabled (can be focused)
            bool f_invalid : 1;         // content is invalid
            bool f_checked : 1;         // is checked/selected
            bool f_timer : 1;           // window has timers
            bool f_parent_defined0 : 1; // this flag can be defined in parent
            bool f_parent_defined1 : 1; // this flag can be defined in parent
            bool f_parent_defined2 : 1; // this flag can be defined in parent
        };
    };

public:
    int16_t id; // (2 bytes) window identifier (2bytes)

    rect_ui16_t rect; // (8 bytes) display rectangle
    color_t color_back;

public:
    void SetNext(window_t *nxt);
    void SetParent(window_t *par);
    window_t *GetNext() const;
    window_t *GetNextEnabled() const;
    window_t *GetParent() const;
    void Draw();
    void DispatchEvent(window_t *sender, uint8_t ev, void *param); //try to handle, frame resends childern
    void Event(window_t *sender, uint8_t event, void *param);      //try to handle, send to parrent if not handled
    void ScreenEvent(window_t *sender, uint8_t event, void *param);
    bool IsVisible() const;
    bool IsEnabled() const;
    bool IsInvalid() const;
    bool IsFocused() const;
    bool IsCapture() const;
    bool HasTimer() const;
    void Validate();
    void Invalidate();
    void SetTag(uint8_t tag);
    uint8_t GetTag() const;

    void SetHasTimer();
    void ClrHasTimer();
    void SetFocus();
    void SetCapture();
    void Enable();
    void Disable();
    void Show();
    void Hide();
    void SetBackColor(color_t clr);
    color_t GetBackColor() const;

    window_t(window_t *parent, rect_ui16_t rect); //todo remove nullptr default values
    virtual ~window_t();

    virtual void push_back(window_t *win);

protected:
    virtual void unconditionalDraw();
    virtual void draw();
    virtual int event(window_t *sender, uint8_t event, void *param) { return 0; }
    virtual void dispatchEvent(window_t *sender, uint8_t event, void *param);

private:
    static window_t *focused_ptr; // has focus
    static window_t *capture_ptr; // capture jog events

public:
    static window_t *GetFocusedWindow();
    static window_t *GetCapturedWindow();
};

extern window_t *window_popup_ptr; //current popup window

extern window_t *window_ptr(int16_t id);

extern int16_t window_create_ptr(int16_t cls_id, int16_t id_parent, rect_ui16_t rect, void *ptr);

extern void window_destroy(int16_t id);

extern void gui_invalidate(void);
