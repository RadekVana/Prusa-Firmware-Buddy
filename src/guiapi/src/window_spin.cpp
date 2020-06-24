// window_spin.c
#include "window_spin.hpp"
#include "gui.hpp"

extern osThreadId displayTaskHandle;

extern void window_numb_draw(window_numb_t *window);

void window_spin_inc(window_spin_t *window, int dif);
void window_spin_dec(window_spin_t *window, int dif);

void window_spin_init(window_spin_t *window) {
    window_class_numb.cls.init(window);
    window->min = 0.0;
    window->max = 100.0F;
    window->step = 1.0F;
    window->count = 101;
    window->index = 0;
    window->flg |= WINDOW_FLG_ENABLED;
}

void window_spin_event(window_spin_t *window, uint8_t event, void *param) {
    switch (event) {
    case WINDOW_EVENT_BTN_DN:
        if ((window->flg & WINDOW_FLG_ENABLED) && window->f_tag)
            screen_dispatch_event((window_t *)window, WINDOW_EVENT_CHANGE, (void *)(int)window->f_tag);
        if (window_ptr(window->id_parent))
            window_ptr(window->id_parent)->SetCapture();
        break;
    case WINDOW_EVENT_ENC_DN:
        window_spin_dec(window, (int)param);
        break;
    case WINDOW_EVENT_ENC_UP:
        window_spin_inc(window, (int)param);
        break;
    case WINDOW_EVENT_CAPT_0:
    case WINDOW_EVENT_CAPT_1:
        window->Invalidate();
        break;
    }
}

void window_spin_inc(window_spin_t *window, int dif) {
    window->index += dif;
    if (window->index >= window->count)
        window->index = window->count - 1;
    window->value = window->min + window->index * window->step;
    _window_invalidate((window_t *)window);
}

void window_spin_dec(window_spin_t *window, int dif) {
    window->index -= dif;
    if (window->index < 0)
        window->index = 0;
    window->value = window->min + window->index * window->step;
    _window_invalidate((window_t *)window);
}

const window_class_spin_t window_class_spin = {
    {
        {
            WINDOW_CLS_SPIN,
            sizeof(window_spin_t),
            (window_init_t *)window_spin_init,
            0,
            (window_draw_t *)window_numb_draw,
            (window_event_t *)window_spin_event,
        },
    }
};

void window_spin_t::setValue(float val) {
    if (val < min)
        val = min;
    if (val > max)
        val = max;
    value = value;
    index = int((value - min) / step);
}
