/**
 * @file mock_windows.hpp
 * @author Radek Vana
 * @brief Mock windows and screens to check window registration and capture
 * @date 2020-11-30
 */

#pragma once
#include "catch2/catch.hpp"
#include "screen.hpp"
#include "window_dlg_popup.hpp"
#include "IDialog.hpp"
#include "window_dlg_strong_warning.hpp"

struct MockMsgBox : public AddSuperWindow<IDialog> {
    MockMsgBox(Rect16 rc)
        : AddSuperWindow<IDialog>(rc) {}
};

class MockStrongDialog : public AddSuperWindow<window_dlg_strong_warning_t> {
public:
    void Show(string_view_utf8 txt) { show(txt); }

    static MockStrongDialog &ShowHotendFan() {
        static MockStrongDialog dlg;
        dlg.Show(_(HotendFanErrorMsg));
        return dlg;
    }

    static MockStrongDialog &ShowPrintFan() {
        static MockStrongDialog dlg;
        dlg.Show(_(PrintFanErrorMsg));
        return dlg;
    }

    static MockStrongDialog &ShowHeaterTimeout() {
        static MockStrongDialog dlg;
        dlg.Show(_(HeaterTimeoutMsg));
        return dlg;
    }

    static MockStrongDialog &ShowUSBFlashDisk() {
        static MockStrongDialog dlg;
        dlg.Show(_(USBFlashDiskError));
        return dlg;
    }
};

struct MockScreen : public AddSuperWindow<screen_t> {
    window_t w_first; // just so w0 is not first
    window_t w0;
    window_t w1;
    window_t w2;
    window_t w3;
    window_t w_last; // just so w3 is not last

    MockScreen()
        : w_first(this, GuiDefaults::RectHeader) // header is not hidden behind dialog
        , w0(this, Rect16(20, 20, 10, 10) + GuiDefaults::RectScreenBody.Top())
        , w1(this, Rect16(20, 40, 10, 10) + GuiDefaults::RectScreenBody.Top())
        , w2(this, Rect16(40, 20, 10, 10) + GuiDefaults::RectScreenBody.Top())
        , w3(this, Rect16(40, 40, 10, 10) + GuiDefaults::RectScreenBody.Top())
        , w_last(this, GuiDefaults::RectHeader) {} // header is not hidden behind dialog

    void ParrentCheck() const;
    void LinkedListCheck(size_t popup_cnt = 0, size_t dialog_cnt = 0, size_t strong_dialog_cnt = 0) const;
    void BasicCheck(size_t popup_cnt = 0, size_t dialog_cnt = 0, size_t strong_dialog_cnt = 0) const;

    template <class... E>
    void CheckOrderAndVisibility(E *... e);

private:
    void checkPtrRange(window_t *&iter, size_t cnt, window_t *first, window_t *last) const;

    template <class T>
    static void checkHidden(const T &arr, window_t &win);
};

template <class T>
void MockScreen::checkHidden(const T &extra_windows, window_t &win) {
    bool hidden = false;

    for (size_t i = 0; i < extra_windows.size(); ++i) {
        if (win.rect.HasIntersection(extra_windows[i]->rect))
            hidden = true;
    }

    //check IsHiddenBehindDialog()
    REQUIRE(win.IsHiddenBehindDialog() == hidden);
}

template <class... E>
void MockScreen::CheckOrderAndVisibility(E *... e) {
    constexpr size_t sz = sizeof...(e);
    std::array<window_t *, sz> extra_windows = { e... };

    size_t popup_cnt = 0;
    size_t dialog_cnt = 0;
    size_t strong_dialog_cnt = 0;

    for (size_t i = 0; i < sz; ++i) {
        switch (extra_windows[i]->GetType()) {
        case win_type_t::dialog:
            ++dialog_cnt;
            break;
        case win_type_t::strong_dialog:
            ++strong_dialog_cnt;
            break;
        case win_type_t::popup:
            ++popup_cnt;
            break;
        default:
            break;
        }
    }

    //check parrent
    ParrentCheck();

    //check linked list
    LinkedListCheck(popup_cnt, dialog_cnt, strong_dialog_cnt);

    //hidden check
    REQUIRE_FALSE(getFirstNormal() == nullptr);
    REQUIRE_FALSE(getLastNormal() == nullptr);
    for (window_t *pWin = getFirstNormal(); pWin != getLastNormal()->GetNext(); pWin = pWin->GetNext()) {
        REQUIRE_FALSE(pWin == nullptr);
        checkHidden(extra_windows, *pWin);
    }

    window_t *pWin = &w_last;
    REQUIRE_FALSE(pWin == nullptr); //should never fail

    //check order of all extra windows
    for (size_t i = 0; i < sz; ++i) {
        pWin = pWin->GetNext();
        REQUIRE_FALSE(pWin == nullptr);
    }

    REQUIRE(pWin->GetNext() == nullptr); // verify if all windows were checked
}
