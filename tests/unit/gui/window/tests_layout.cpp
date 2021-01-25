#include "catch2/catch.hpp"

#include "sound_enum.h"
#include "ScreenHandler.hpp"
#include "cmsis_os.h" //HAL_GetTick
#include "mock_windows.hpp"
#include "mock_display.hpp"
#include <memory>

//stubbed header does not have C linkage .. to be simpler
static uint32_t hal_tick = 0;
uint32_t HAL_GetTick() { return hal_tick; }

static constexpr Rect16 DispRect = { 0, 0, MockDisplay::Cols(), MockDisplay::Rows() };

static void TestRectColor(Rect16 rect, color_t color) {
    for (int16_t X = rect.Left(); X < rect.Width(); ++X) {
        for (int16_t Y = rect.Top(); Y < rect.Height(); ++Y) {
            REQUIRE(MockDisplay::Instance().GetpixelNativeColor(X, Y) == color);
        }
    }
};

//Minuend: The number that is to be subtracted from.
//Subtrahend: The number that is to be subtracted.
static void TestRectDiffColor(Rect16 Minuend, Rect16 Subtrahend, color_t MinuendColor, color_t DiffColor) {
    for (int16_t X = Minuend.Left(); X < Minuend.Width(); ++X) {
        for (int16_t Y = Minuend.Top(); Y < Minuend.Height(); ++Y) {
            if (Subtrahend.Contain(point_i16_t({ X, Y }))) {
                REQUIRE(MockDisplay::Instance().GetpixelNativeColor(X, Y) == DiffColor);
            } else {
                REQUIRE(MockDisplay::Instance().GetpixelNativeColor(X, Y) == MinuendColor);
            }
        }
    }
};

static void TestRectDraw(Rect16 rect, color_t color) {
    window_t win(nullptr, rect);
    win.SetBackColor(color);
    win.Draw();
    TestRectColor(rect, color);
};

static void TestDispRectDraw(Rect16 rect, color_t color_win, color_t color_disp) {
    MockDisplay::Instance().clear(color_disp);
    TestRectColor(DispRect, color_disp);
    TestRectDraw(rect, color_win);
    TestRectDiffColor(DispRect, rect, color_disp, color_win);
};

TEST_CASE("Window layout tests", "[window]") {
    MockDisplay::Instance().clear(COLOR_BLACK);
    TestRectColor(DispRect, COLOR_BLACK);

    SECTION("RECT") {
        TestDispRectDraw(Rect16(0, 0, 0, 0), COLOR_BLUE, COLOR_WHITE);

        TestDispRectDraw(DispRect, COLOR_BLUE, COLOR_WHITE);

        TestDispRectDraw({ 1, 0, MockDisplay::Cols() - 1, MockDisplay::Rows() }, COLOR_RED, COLOR_BLACK);

        TestDispRectDraw(Rect16(10, 20, 1, 2), COLOR_BLUE, COLOR_WHITE);

        TestDispRectDraw(Rect16(0, 0, 1, 2), COLOR_RED, COLOR_BLUE);

        TestDispRectDraw(Rect16(10, 20, 1, 2), COLOR_WHITE, COLOR_BLACK);

        TestDispRectDraw(Rect16(10, 20, 1, 2), COLOR_BLACK, COLOR_BLUE);
    }
};
