#pragma once
#include "gui.hpp"
#include "window_file_list.hpp"
#include "window_header.hpp"

class screen_filebrowser_data_t : public window_frame_t {
    window_header_t header;
    window_file_list_t w_filelist;

public:
    screen_filebrowser_data_t();
};