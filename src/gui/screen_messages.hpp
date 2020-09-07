//screen_messages.hpp
#pragma once
#include "gui.hpp"
#include "window_header.hpp"
#include "status_footer.h"
#include "window_term.hpp"

struct screen_messages_data_t : public window_frame_t {
    window_header_t header;
    status_footer_t footer;
    window_term_t term;
    term_t terminal;
    uint8_t term_buff[TERM_BUFF_SIZE(20, 16)]; //chars and attrs (640 bytes) + change bitmask (40 bytes)
    //todo calculate from screen size and font size

public:
    screen_messages_data_t();

private:
    virtual void windowEvent(window_t *sender, uint8_t event, void *param) override;
};
