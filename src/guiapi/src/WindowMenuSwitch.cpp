/**
 * @file WindowMenuSwitch.cpp
 * @author Radek Vana
 * @brief
 * @version 0.1
 * @date 2020-11-09
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "WindowMenuItems.hpp"
#include "resource.h"

/*****************************************************************************/
//IWiSwitch
IWiSwitch::IWiSwitch(int32_t index, string_view_utf8 label, uint16_t id_icon, is_enabled_t enabled, is_hidden_t hidden, Items_t items_)
    : AddSuper<WI_LABEL_t>(label, calculateExtensionWidth(items_), id_icon, enabled, hidden)
    , index(index)
    , items(items_) {
    has_brackets = GuiDefaults::MenuSwitchHasBrackets;
}

invalidate_t IWiSwitch::Change(int /*dif*/) {
    if ((++index) >= items.size) {
        index = 0;
    }
    return invalidate_t::yes;
}

void IWiSwitch::click(IWindowMenu & /*window_menu*/) {
    size_t old_index = index;
    Change(0);
    OnChange(old_index);
}

bool IWiSwitch::SetIndex(size_t idx) {
    if (idx >= items.size)
        return false;
    else {
        index = idx;
        return true;
    }
}

Rect16 IWiSwitch::getSwitchRect(Rect16 extension_rect) const {
    if (!has_brackets)
        return extension_rect;

    extension_rect += Rect16::Left_t(BracketFont->w);
    extension_rect -= Rect16::Width_t(BracketFont->w * 2);
    return extension_rect;
}

Rect16 IWiSwitch::getLeftBracketRect(Rect16 extension_rect) const {
    extension_rect = Rect16::Width_t(BracketFont->w);
    return extension_rect;
}

Rect16 IWiSwitch::getRightBracketRect(Rect16 extension_rect) const {
    extension_rect += Rect16::Left_t(extension_rect.Width() - BracketFont->w);
    extension_rect = Rect16::Width_t(BracketFont->w);
    return extension_rect;
}

void IWiSwitch::printExtension(Rect16 extension_rect, color_t color_text, color_t color_back, uint8_t swap) const {
    //draw switch
    render_text_align(getSwitchRect(extension_rect), items.texts[index], GuiDefaults::FontMenuItems, color_back,
        (IsFocused() && IsEnabled()) ? GuiDefaults::ColorSelected : color_text,
        GuiDefaults::MenuSwitchPadding, GuiDefaults::MenuAlignment);

    //draw brackets
    if (has_brackets) {
        render_text_align(getLeftBracketRect(extension_rect), _("["), BracketFont,
            color_back, COLOR_SILVER, GuiDefaults::MenuSwitchPadding, GuiDefaults::MenuAlignment);

        //draw bracket end  TODO: Change font
        render_text_align(getRightBracketRect(extension_rect), _("]"), BracketFont,
            color_back, COLOR_SILVER, GuiDefaults::MenuSwitchPadding, GuiDefaults::MenuAlignment);
    }
}

WI_SWITCH_t::WI_SWITCH_t(int32_t index, string_view_utf8 label, uint16_t id_icon, is_enabled_t enabled, is_hidden_t hidden, Items_t items_)
    : AddSuper<IWiSwitch>(index, label, id_icon, enabled, hidden, items_) {}
