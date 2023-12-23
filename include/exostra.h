/*
 * exostra.h : Exostra Window Manager (https://github.com/aremmell/exostra)
 *
 * Copyright: © 2023-2024 Ryan M. Lederman <lederman@gmail.com>
 * Version:   0.0.1
 * License:   The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _EXOSTRA_H_INCLUDED
# define _EXOSTRA_H_INCLUDED

# include <cstdint>
# include <functional>
# include <type_traits>
# include <string>
# include <memory>
# include <queue>
# include <mutex>

// TODO: remove me
# define EWM_COLOR_565

// Disables mutex locks required in multi-threaded environments.
# define EWM_NOMUTEXES

// Available logging levels.
# define EWM_LOG_LEVEL_NONE    0
# define EWM_LOG_LEVEL_ERROR   1
# define EWM_LOG_LEVEL_WARNING 2
# define EWM_LOG_LEVEL_INFO    3
# define EWM_LOG_LEVEL_DEBUG   4
# define EWM_LOG_LEVEL_VERBOSE 5

// Enabled logging level (setting to any level except EWM_LOG_LEVEL_NONE increases
// the resulting binary size substantially!).
# define EWM_LOG_LEVEL EWM_LOG_LEVEL_VERBOSE //EWM_LOG_LEVEL_NONE

// Enables runtime assertions. Upon a failed assertion, prints the expression that
// evaluated to false, as well as the backtrace leading up to the failed assertion
// (if available), then enters an infinite loop. Implies EWM_LOG_LEVEL >=
// EWM_LOG_LEVEL_ERROR.
# define EWM_ENABLE_ASSERTIONS

# if defined(ESP32) || defined(ESP8266)
#  include <esp_debug_helpers.h>
#  define EWM_BACKTRACE_FRAMES 5
#  define print_backtrace() \
    ets_install_putc1([](char c) \
    { \
        putc(c, stderr); \
    }); \
    esp_backtrace_print(EWM_BACKTRACE_FRAMES)
# else
#  define print_backtrace
# endif

# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
#  include <typeinfo>
#  include <cxxabi.h>
# endif

# define _EWM_LOG(pfx, fmt, ...) \
    do { \
        printf("[%c] %s (%s:%d): " fmt "\n", pfx, __FUNCTION__, basename(__FILE__), \
            __LINE__ __VA_OPT__(,) __VA_ARGS__); \
    } while(false)
# define _EWM_LOG_E(fmt, ...) _EWM_LOG('E', fmt __VA_OPT__(,) __VA_ARGS__)
# define _EWM_LOG_W(fmt, ...) _EWM_LOG('W', fmt __VA_OPT__(,) __VA_ARGS__)
# define _EWM_LOG_I(fmt, ...) _EWM_LOG('I', fmt __VA_OPT__(,) __VA_ARGS__)
# define _EWM_LOG_D(fmt, ...) _EWM_LOG('D', fmt __VA_OPT__(,) __VA_ARGS__)
# define _EWM_LOG_V(fmt, ...) _EWM_LOG('V', fmt __VA_OPT__(,) __VA_ARGS__)

# if defined(EWM_ENABLE_ASSERTIONS)
#  if EWM_LOG_LEVEL < EWM_LOG_LEVEL_ERROR
#   undef EWM_LOG_LEVEL
#   define EWM_LOG_LEVEL EWM_LOG_LEVEL_ERROR
#  endif
#  define EWM_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            EWM_LOG_E("!!! ASSERT: '" #expr "'"); \
            print_backtrace(); \
            while (true); \
        } \
    } while (false)
# else
#  define EWM_ASSERT(expr)
# endif

# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_ERROR
#  define EWM_LOG_E(fmt, ...) _EWM_LOG_E(fmt __VA_OPT__(,) __VA_ARGS__)
# else
#  define EWM_LOG_E(fmt, ...)
# endif
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_WARNING
#  define EWM_LOG_W(fmt, ...) _EWM_LOG_W(fmt __VA_OPT__(,) __VA_ARGS__)
# else
#  define EWM_LOG_W(fmt, ...)
# endif
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_INFO
#  define EWM_LOG_I(fmt, ...) _EWM_LOG_I(fmt __VA_OPT__(,) __VA_ARGS__)
# else
#  define EWM_LOG_I(fmt, ...)
# endif
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_DEBUG
#  define EWM_LOG_D(fmt, ...) _EWM_LOG_D(fmt __VA_OPT__(,) __VA_ARGS__)
# else
#  define EWM_LOG_D(fmt, ...)
# endif
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
#  define EWM_LOG_V(fmt, ...) _EWM_LOG_V(fmt __VA_OPT__(,) __VA_ARGS__)
# else
#  define EWM_LOG_V(fmt, ...)
# endif

# define EWM_BOOL2STR(b) ((b) ? "true" : "false")
# define EWM_CONST(type, name, value) \
    static constexpr PROGMEM type name = value

# if defined(EWM_GFX_ADAFRUIT) && __has_include(<Adafruit_GFX.h>)
#  include <Adafruit_GFX.h>
#  if defined(_ADAFRUIT_RA8875_H)
    using IGfxDisplay = Adafruit_RA8875;
#  else
#   include <Adafruit_SPITFT.h>
    using IGfxDisplay = Adafruit_SPITFT;
#  endif
    using IGfxContext16 = GFXcanvas16;
#  if defined(__AVR__)
#   include <avr/pgmspace.h>
#  elif defined(ESP32) || defined(ESP8266)
#   include <pgmspace.h>
#  endif
#  if !defined(pgm_read_byte)
#   define pgm_read_byte(addr) (*(reinterpret_cast<const uint8_t*>(addr)))
#  endif
#  if !defined(pgm_read_word)
#   define pgm_read_word(addr) (*(reinterpret_cast<const uint16_t*>(addr)))
#  endif
#  if !defined(pgm_read_dword)
#   define pgm_read_dword(addr) (*(reinterpret_cast<const uint32_t*>(addr)))
#  endif
#  if !defined(pgm_read_pointer)
#   if !defined(__INT_MAX__) || (__INT_MAX__ > 0xffff)
#    define pgm_read_pointer(addr) (static_cast<void*>(pgm_read_dword(addr)))
#   else
#    define pgm_read_pointer(addr) (static_cast<void*>(pgm_read_word(addr)))
#   endif
#  endif
# elif defined(EWM_GFX_ARDUINO) && __has_include(<Arduino_GFX_Library.h>)
#  include <Arduino_GFX_Library.h>
#  if defined(LITTLE_FOOT_PRINT)
#   error "required Arduino_Canvas implementation unavailable due to LITTLE_FOOT_PRINT"
#  endif
#  if defined(ATTINY_CORE)
#   error "required GFXfont implementation unavailable due to ATTINY_CORE"
#  endif
    using IGfxContext16 = Arduino_Canvas;
# else
#  error "define EWM_GFX_ADAFRUIT or EWM_GFX_ARDUINO, and install the relevant \
library in order to select a low-level graphics driver"
# endif

namespace exostra
{
    /** Window identifier. */
    using WindowID = uint8_t;

    /** Represents an invalid window identifier. */
    EWM_CONST(WindowID, WID_INVALID, 0);

    /** Window style bitmask. */
    using Style = uint16_t;

    /** State bitmask. */
    using State = uint16_t;

    /** Window message parameter type. */
    using MsgParam = uint32_t;

    /** Window message parameter component type. */
    using MsgParamWord = uint16_t;

    /** Physical display driver. */
    using GfxDisplay = IGfxDisplay;

    /** Pointer to physical display driver. */
    using GfxDisplayPtr = std::shared_ptr<GfxDisplay>;

# if defined(EWM_COLOR_565)
    using Color      = uint16_t;      /**< Color type (16-bit 565 RGB). */
#  if !defined(ADAFRUIT_RA8875)
    using GfxContext = IGfxContext16; /**< Graphics context (16-bit 565 RGB). */
#  else
    using GfxContext = Adafruit_RA8875;
#  endif
# elif defined(EWM_COLOR_888)
#  error "24-bit RGB mode is not yet implemented"
# else
#  error "define EWM_COLOR_565 or EWM_COLOR_888 in order to select a color mode"
# endif

    /** Pointer to graphics context (e.g. canvas/frame buffer). */
    using GfxContextPtr = std::shared_ptr<GfxContext>;

    /** Font type. */
    using Font = GFXfont;

    /** Coordinate in 3D space (e.g. X, Y, or Z). */
    using Coord = int16_t;

    /** Extent (e.g. width, height). */
    using Extent = uint16_t;

    /** Point in 2D space. */
    struct Point
    {
        Point() = default;

        template<typename T1, typename T2>
        Point(T1 xAxis, T2 yAxis)
        {
            x = static_cast<Coord>(xAxis);
            y = static_cast<Coord>(yAxis);
        }

        Coord x = 0; /**< X-axis value. */
        Coord y = 0; /**< Y-axis value. */
    };

    /** Two points in 2D space (left/top, right/bottom). */
    struct Rect
    {
        Rect() = default;

        explicit Rect(Coord l, Coord t, Extent r, Extent b)
            : left(l), top(t), right(r), bottom(b)
        {
        }

        bool operator==(const Rect& rhs) const noexcept
        {
            return left   == rhs.left &&
                   top    == rhs.top &&
                   right  == rhs.right &&
                   bottom == rhs.bottom;
        }

        bool operator !=(const Rect& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        Coord left   = 0; /**< X-axis value of the left edge. */
        Coord top    = 0; /**< Y-axis value of the top edge. */
        Coord right  = 0; /**< X-axis value of the right edge. */
        Coord bottom = 0; /**< Y-axis value of the bottom edge. */

        Extent width() const noexcept
        {
            EWM_ASSERT(right >= left);
            return static_cast<Extent>(right - left);
        }

        Extent height() const noexcept
        {
            EWM_ASSERT(bottom >= top);
            return static_cast<Extent>(bottom - top);
        }

        bool empty() const noexcept
        {
            return width() == 0 || height() == 0;
        }

        Point getTopLeft() const noexcept { return { left, top }; }
        Point getBottomRight() const noexcept { return { right, bottom }; }

        void inflate(Extent px) noexcept
        {
            left   -= px;
            top    -= px;
            right  += px;
            bottom += px;
        }

        void deflate(Extent px) noexcept
        {
            EWM_ASSERT(px < width());
            EWM_ASSERT(px < height());
            left   += px;
            top    += px;
            right  -= px;
            bottom -= px;
        }

        bool overlapsRect(const Rect& other) const noexcept
        {
            if ((top >= other.top && top <= other.bottom) ||
                (bottom <= other.bottom && bottom >= other.top)) {
                if ((left <= other.left && right >= other.left) ||
                    (right >= other.right && left <= other.right)) {
                    return true;
                }
            }
            if ((left >= other.left && left <= other.right) ||
                (right <= other.right && right >= other.left)) {
                if ((top <= other.top && bottom >= other.top) ||
                    (bottom >= other.bottom && top <= other.bottom)) {
                    return true;
                }
            }
            if ((left <= other.left && top <= other.top &&
                 right >= other.right && bottom >= other.bottom) ||
                (left >= other.left && top >= other.top &&
                 right <= other.right && bottom <= other.bottom)) {
                return true;
            }
            return false;
        }

        bool intersectsRect(const Rect& other) const noexcept
        {
            return overlapsRect(other) || other.overlapsRect(*this);
        }

        Rect getIntersection(const Rect& other) const noexcept
        {
            if (intersectsRect(other)) {
                return Rect(
                    max(left, other.left),
                    max(top, other.top),
                    min(right, other.right),
                    min(bottom, other.bottom)
                );
            }
            return Rect();
        }

        void mergeRect(const Rect& rect) noexcept
        {
            left   = min(left, rect.left);
            top    = min(top, rect.top);
            right  = max(right, rect.right);
            bottom = max(bottom, rect.bottom);
        }

        std::queue<Rect> subtractRect(const Rect& other) const
        {
            std::queue<Rect> rects;
            if (intersectsRect(other)) {
                auto mergedRect = *this;
                mergedRect.mergeRect(other);
                const auto alignedLeft   = mergedRect.left   >= other.left;
                const auto alignedTop    = mergedRect.top    >= other.top;
                const auto alignedRight  = mergedRect.right  <= other.right;
                const auto alignedBottom = mergedRect.bottom <= other.bottom;
                if (!alignedLeft || !alignedTop || !alignedRight || !alignedBottom) {
                    const auto topNoAlignTop       = max(mergedRect.top, top);
                    const auto topNoAlignBottom    = min(mergedRect.bottom, other.bottom);
                    const auto bottomNoAlignTop    = min(mergedRect.bottom, other.top);
                    const auto bottomNoAlignBottom = min(mergedRect.bottom, bottom);
                    const auto leftNoAlignRight    = min(mergedRect.right, other.right);
                    const auto leftNoAlignLeft     = max(mergedRect.left, left);
                    const auto rightNoAlignRight   = min(mergedRect.right, right);
                    const auto rightNoAlignLeft    = max(mergedRect.left, other.left);
                    if (alignedLeft || alignedRight) {
                        if (alignedLeft && !alignedRight) {
                            rects.emplace(
                                leftNoAlignRight,
                                max(mergedRect.top, top),
                                rightNoAlignRight,
                                min(mergedRect.bottom, bottom)
                            );
                        }
                        if (alignedRight && !alignedLeft) {
                            rects.emplace(
                                leftNoAlignLeft,
                                max(mergedRect.top, top),
                                rightNoAlignLeft,
                                min(mergedRect.bottom, bottom)
                            );
                        }
                        if (!alignedTop && !alignedBottom) {
                            rects.emplace(
                                max(mergedRect.left, left),
                                topNoAlignTop,
                                min(mergedRect.right, right),
                                bottomNoAlignTop
                            );
                            rects.emplace(
                                max(mergedRect.left, left),
                                topNoAlignBottom,
                                min(mergedRect.right, right),
                                bottomNoAlignBottom
                            );
                        }
                    }
                    if (alignedTop || alignedBottom) {
                        if (alignedTop && !alignedBottom) {
                            rects.emplace(
                                max(mergedRect.left, left),
                                topNoAlignBottom,
                                min(mergedRect.right, right),
                                bottomNoAlignBottom
                            );
                        }
                        if (alignedBottom && !alignedTop) {
                            rects.emplace(
                                max(mergedRect.left, left),
                                topNoAlignTop,
                                min(mergedRect.right, right),
                                bottomNoAlignTop
                            );
                        }
                        if (!alignedLeft && !alignedRight) {
                            rects.emplace(
                                leftNoAlignRight,
                                max(mergedRect.top, top),
                                rightNoAlignRight,
                                min(mergedRect.bottom, bottom)
                            );
                            rects.emplace(
                                leftNoAlignLeft,
                                max(mergedRect.top, top),
                                rightNoAlignLeft,
                                min(mergedRect.bottom, bottom)
                            );
                        }
                    }
                    if (!alignedLeft && !alignedTop && !alignedRight && !alignedBottom) {
                        Rect edgeRect(
                            min(other.left, left),
                            min(other.top, top),
                            max(other.left, left),
                            max(other.bottom, bottom)
                        );
                        rects.push(edgeRect);
                        edgeRect.right  = max(other.right, right);
                        edgeRect.bottom = max(other.top, top);
                        rects.push(edgeRect);
                        edgeRect.left   = min(other.right, right);
                        edgeRect.top    = min(other.top, top);
                        edgeRect.bottom = max(other.bottom, bottom);
                        rects.push(edgeRect);
                        edgeRect.left = min(other.left, left);
                        edgeRect.top  = min(other.bottom, bottom);
                        rects.push(edgeRect);
                    }
                }
            }
            return std::move(rects);
        }

        bool outsideRect(const Rect& other) const noexcept
        {
            return !other.pointWithin(left, top) &&
                   !other.pointWithin(right, top) &&
                   !other.pointWithin(left, bottom) &&
                   !other.pointWithin(right, bottom);
        }

        bool withinRect(const Rect& other) const noexcept
        {
            return other.pointWithin(left, top) &&
                   other.pointWithin(right, top) &&
                   other.pointWithin(left, bottom) &&
                   other.pointWithin(right, bottom);
        }

        bool pointWithin(Coord x, Coord y) const noexcept
        {
            return x >= left && x <= right && y >= top && y <= bottom;
        }
    };

    inline GFXglyph* getGlyphAtOffset(const GFXfont* font, uint8_t off)
    {
# ifdef __AVR__
        return &((static_cast<GFXglyph*>(pgm_read_pointer(&font->glyph)))[off]);
# else
        return font->glyph + off;
# endif
    }

    static void getCharBounds(uint8_t ch, uint8_t* cx, uint8_t* cy, uint8_t* xAdv,
        uint8_t* yAdv, int8_t* xOff, int8_t* yOff, uint8_t textSize = 1,
        const GFXfont* font = nullptr) noexcept
    {
        uint8_t first = font ? pgm_read_byte(&font->first) : 0;
        bool okCh = font ? ch >= first && ch <= pgm_read_byte(&font->last) : false;
        auto glyph = okCh ? getGlyphAtOffset(font, ch - first) : nullptr;
        if (cx) { *cx = textSize * (font ? (okCh ? pgm_read_byte(&glyph->width) : 0) : 6); }
        if (cy) { *cy = textSize * (font ? (okCh ? pgm_read_byte(&glyph->height) : 0) : 8); }
        if (xAdv) { *xAdv = textSize * (okCh ? pgm_read_byte(&glyph->xAdvance) : 6); }
        if (yAdv) { *yAdv = okCh ? pgm_read_byte(&font->yAdvance) : textSize * 8; }
        if (xOff) { *xOff = okCh ? pgm_read_byte(&glyph->xOffset) : 0; }
        if (yOff) { *yOff = okCh ? pgm_read_byte(&glyph->yOffset) : 0; }
    }

    template<typename T1, typename T2>
    inline bool bitsHigh(const T1& bitmask, const T2& bits) noexcept
    {
        return (bitmask & bits) == bits;
    }

    using Mutex     = std::recursive_mutex;
    using ScopeLock = std::lock_guard<Mutex>;

    typedef enum
    {
        MSG_NONE     = 0,
        MSG_CREATE   = 1,
        MSG_DESTROY  = 2,
        MSG_DRAW     = 3,
        MSG_POSTDRAW = 4,
        MSG_INPUT    = 5,
        MSG_EVENT    = 6,
        MSG_RESIZE   = 7
    } Message;

    enum
    {
        STY_VISIBLE    =  1 << 0,
        STY_CHILD      =  1 << 1,
        STY_FRAME      =  1 << 2,
        STY_SHADOW     =  1 << 3,
        STY_TOPLEVEL   = (1 << 4) | STY_FRAME | STY_SHADOW,
        STY_AUTOSIZE   =  1 << 5,
        STY_FULLSCREEN =  1 << 6,
        STY_BUTTON     =  1 << 7,
        STY_LABEL      =  1 << 8,
        STY_PROMPT     =  (1 << 9) | STY_TOPLEVEL,
        STY_PROGBAR    =  1 << 10,
        STY_CHECKBOX   =  1 << 11
    };

    enum
    {
        STA_ALIVE   = 1 << 0, /**< Active (not yet destroyed). */
        STA_CHECKED = 1 << 1, /**< Checked/highlighted item. */
        STA_DIRTY   = 1 << 2  /**< Needs redrawing. */
    };

    enum
    {
        PBR_NORMAL        = 1 << 0, /**< Standard linear-fill progress bar. */
        PBR_INDETERMINATE = 1 << 1  /**< Marquee-style progress bar. */
    };

    enum
    {
        DT_CENTER   = 1 << 0, /**< Horizontal align center. */
        DT_SINGLE   = 1 << 1, /**< Single line of text. */
        DT_CLIP     = 1 << 2, /**< Text outside the rect will not be drawn. */
        DT_ELLIPSIS = 1 << 3  /**< Replace clipped text with '...' */
    };

    typedef enum
    {
        EVT_CHILD_TAPPED = 1
    } EventType;

    typedef enum
    {
        INPUT_TAP = 1
    } InputType;

    struct InputParams
    {
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
        std::string handledBy;
# endif
        InputType type = InputType(0);
        Coord x = 0;
        Coord y = 0;
    };

    static MsgParam makeMsgParam(const MsgParamWord& hiWord, const MsgParamWord& loWord)
    {
        return (static_cast<MsgParam>(hiWord) << 16) | (loWord & 0xffffU);
    }

    static MsgParamWord getMsgParamHiWord(const MsgParam& msgParam)
    {
        return ((msgParam >> 16) & 0xffffU);
    }

    static MsgParamWord getMsgParamLoWord(const MsgParam& msgParam)
    {
        return (msgParam & 0xffffU);
    }

    typedef enum
    {
        COLOR_SCREENSAVER = 1,

        COLOR_PROMPT_BG,
        COLOR_PROMPT_FRAME,
        COLOR_PROMPT_SHADOW,

        COLOR_WINDOW_TEXT,
        COLOR_WINDOW_BG,
        COLOR_WINDOW_FRAME,
        COLOR_WINDOW_SHADOW,

        COLOR_BUTTON_TEXT,
        COLOR_BUTTON_TEXT_PRESSED,
        COLOR_BUTTON_BG,
        COLOR_BUTTON_BG_PRESSED,
        COLOR_BUTTON_FRAME,
        COLOR_BUTTON_FRAME_PRESSED,

        COLOR_PROGRESS_BG,
        COLOR_PROGRESS_FILL,

        COLOR_CHECKBOX_CHECK_BG,
        COLOR_CHECKBOX_CHECK_FRAME,
        COLOR_CHECKBOX_CHECK
    } ColorID;

    typedef enum
    {
        METRIC_X_PADDING = 1,               /**< Extent */
        METRIC_Y_PADDING,                   /**< Extent */

        METRIC_DEF_TEXT_SIZE,               /**< uint8_t */

        METRIC_WINDOW_FRAME_PX,             /**< Extent */
        METRIC_CORNER_RADIUS_WINDOW,        /**< Coord */
        METRIC_CORNER_RADIUS_BUTTON,        /**< Coord */
        METRIC_CORNER_RADIUS_PROMPT,        /**< Coord */
        METRIC_CORNER_RADIUS_CHECKBOX,      /**< Coord */

        METRIC_DEF_BUTTON_CX,               /**< Extent */
        METRIC_DEF_BUTTON_CY,               /**< Extent */
        METRIC_BUTTON_LABEL_PADDING,        /**< Extent */
        METRIC_BUTTON_TAPPED_DURATION,      /**< uint32_t */

        METRIC_MAX_PROMPT_CX,               /**< Extent */
        METRIC_MAX_PROMPT_CY,               /**< Extent */

        METRIC_DEF_PROGBAR_HEIGHT,          /**< Extent */
        METRIC_PROGBAR_MARQUEE_CX_FACTOR,   /**< float */
        METRIC_PROGBAR_MARQUEE_STEP,        /**< float */

        METRIC_DEF_CHECKBOX_HEIGHT,         /**< Extent */
        METRIC_CHECKBOX_CHECK_AREA_PADDING, /**< Extent */
        METRIC_CHECKBOX_CHECK_MARK_PADDING, /**< Extent */
        METRIC_CHECKBOX_CHECK_DELAY         /**< uint32_t */
    } MetricID;

    struct Variant
    {
        enum
        {
            EMPTY  = 0,
            EXTENT = 1,
            COORD  = 2,
            UINT8  = 3,
            UINT32 = 4,
            FLOAT  = 5
        };

        Variant() = default;

        int getType() const noexcept { return _type; }

        Extent getExtent() const noexcept
        {
            EWM_ASSERT(_type == EXTENT);
            return _extentValue;
        }

        void setExtent(const Extent& extent) noexcept
        {
            _extentValue = extent;
            _type = EXTENT;
        }

        Coord getCoord() const noexcept
        {
            EWM_ASSERT(_type == COORD);
            return _coordValue;
        }

        void setCoord(const Coord& coord) noexcept
        {
            _coordValue = coord;
            _type = COORD;
        }

        uint8_t getUint8() const noexcept
        {
            EWM_ASSERT(_type == UINT8);
            return _uint8Value;
        }

        void setUint8(const uint8_t& octet) noexcept
        {
            _uint8Value = octet;
            _type = UINT8;
        }

        uint32_t getUint32() const noexcept
        {
            EWM_ASSERT(_type == UINT32);
            return _uint32Value;
        }

        void setUint32(const uint32_t& dword) noexcept
        {
            _uint32Value = dword;
            _type = UINT32;
        }

        float getFloat() const noexcept
        {
            EWM_ASSERT(_type == FLOAT);
            return _floatValue;
        }

        void setFloat(const float& flt) noexcept
        {
            _floatValue = flt;
            _type = FLOAT;
        }

    private:
        union
        {
            Extent _extentValue;
            Coord _coordValue;
            uint8_t _uint8Value;
            uint32_t _uint32Value;
            float _floatValue = 0.0f;
        };
        int _type = EMPTY;
    };

    class ITheme
    {
    public:
        enum DisplaySize
        {
            Small = 0,
            Medium,
            Large
        };

        virtual void setDisplayExtents(Extent, Extent) = 0;

        virtual Color getColor(ColorID) const = 0;
        virtual Variant getMetric(MetricID) const = 0;

        virtual void drawScreensaver(const GfxDisplayPtr&) const = 0;

        virtual void setDefaultFont(const Font*) = 0;
        virtual const Font* getDefaultFont() const = 0;

        virtual DisplaySize getDisplaySize() const = 0;
        virtual Extent getScaledValue(Extent) const = 0;

        virtual void drawWindowFrame(const GfxContextPtr&, const Rect&, Coord, Color) const = 0;
        virtual void drawWindowShadow(const GfxContextPtr&, const Rect&, Coord, Color) const = 0;
        virtual void drawWindowBackground(const GfxContextPtr&, const Rect&, Coord, Color) const = 0;
        virtual void drawText(const GfxContextPtr&, const char*, uint8_t, const Rect&,
            uint8_t, Color, const Font*) const = 0;

        virtual void drawProgressBarBackground(const GfxContextPtr&, const Rect&) const = 0;
        virtual void drawProgressBarProgress(const GfxContextPtr&, const Rect&, float) const = 0;
        virtual void drawProgressBarIndeterminate(const GfxContextPtr&, const Rect&, float) const = 0;

        virtual void drawCheckBox(const GfxContextPtr&, const char*, bool, const Rect&) const = 0;
    };

    using ThemePtr = std::shared_ptr<ITheme>;

    class DefaultTheme : public ITheme
    {
    public:
        DefaultTheme() = default;

        void setDisplayExtents(Extent width, Extent height) final
        {
            _displayWidth  = width;
            _displayHeight = height;
        }

        Color getColor(ColorID colorID) const final
        {
            switch (colorID) {
                case COLOR_SCREENSAVER:          return 0x0000;
                case COLOR_PROMPT_BG:            return 0xef5c;
                case COLOR_PROMPT_FRAME:         return 0x9cf3;
                case COLOR_PROMPT_SHADOW:        return 0xb5b6;
                case COLOR_WINDOW_TEXT:          return 0x0000;
                case COLOR_WINDOW_BG:            return 0xdedb;
                case COLOR_WINDOW_FRAME:         return 0x9cf3;
                case COLOR_WINDOW_SHADOW:        return 0xb5b6;
                case COLOR_BUTTON_TEXT:          return 0xffff;
                case COLOR_BUTTON_TEXT_PRESSED:  return 0xffff;
                case COLOR_BUTTON_BG:            return 0x8c71;
                case COLOR_BUTTON_BG_PRESSED:    return 0x738e;
                case COLOR_BUTTON_FRAME:         return 0x6b6d;
                case COLOR_BUTTON_FRAME_PRESSED: return 0x6b6d;
                case COLOR_PROGRESS_BG:          return 0xef5d;
                case COLOR_PROGRESS_FILL:        return 0x0ce0;
                case COLOR_CHECKBOX_CHECK_BG:    return 0xef5d;
                case COLOR_CHECKBOX_CHECK:       return 0x3166;
                case COLOR_CHECKBOX_CHECK_FRAME: return 0x9cf3;
                default:
                    EWM_ASSERT("!invalid color ID");
                    return Color(0);
            }
        }

        Variant getMetric(MetricID metricID) const final
        {
            Variant retval;
            switch (metricID) {
                case METRIC_X_PADDING:
                    retval.setExtent(abs(_displayWidth * 0.05f));
                break;
                case METRIC_Y_PADDING:
                    retval.setExtent(abs(_displayHeight * 0.05f));
                break;
                case METRIC_DEF_TEXT_SIZE:
                    retval.setUint8(1);
                break;
                case METRIC_WINDOW_FRAME_PX:
                    retval.setExtent(1);
                break;
                case METRIC_CORNER_RADIUS_WINDOW:
                    retval.setCoord(0);
                break;
                case METRIC_CORNER_RADIUS_BUTTON:
                    retval.setCoord(getScaledValue(4));
                break;
                case METRIC_CORNER_RADIUS_PROMPT:
                    retval.setCoord(getScaledValue(4));
                break;
                case METRIC_CORNER_RADIUS_CHECKBOX:
                    retval.setCoord(getScaledValue(0));
                break;
                case METRIC_DEF_BUTTON_CX:
                    retval.setExtent(abs(max(_displayWidth * 0.19f, 60.0f)));
                break;
                case METRIC_DEF_BUTTON_CY: {
                    const auto btnWidth = getMetric(METRIC_DEF_BUTTON_CX).getExtent();
                    retval.setExtent(abs(btnWidth * 0.52f));
                }
                break;
                case METRIC_BUTTON_LABEL_PADDING:
                    retval.setExtent(getScaledValue(10));
                break;
                case METRIC_BUTTON_TAPPED_DURATION:
                    retval.setUint32(200);
                break;
                case METRIC_MAX_PROMPT_CX:
                    retval.setExtent(abs(_displayWidth * 0.75f));
                break;
                case METRIC_MAX_PROMPT_CY:
                    retval.setExtent(abs(_displayHeight * 0.75f));
                break;
                case METRIC_DEF_PROGBAR_HEIGHT:
                    retval.setExtent(abs(_displayHeight * 0.10f));
                break;
                case METRIC_PROGBAR_MARQUEE_CX_FACTOR:
                    retval.setFloat(0.33f);
                break;
                case METRIC_PROGBAR_MARQUEE_STEP: {
                    static constexpr float step = 1.0f;
                    switch (getDisplaySize()) {
                        default:
                        case DisplaySize::Small:
                            retval.setFloat(step);
                        break;
                        case DisplaySize::Medium:
                            retval.setFloat(step * 2.0f);
                        break;
                        case DisplaySize::Large:
                            retval.setFloat(step * 4.0f);
                        break;
                    }
                }
                break;
                case METRIC_DEF_CHECKBOX_HEIGHT:
                    retval.setExtent(abs(_displayHeight * 0.10f));
                break;
                case METRIC_CHECKBOX_CHECK_AREA_PADDING:
                    retval.setExtent(getScaledValue(2));
                break;
                case METRIC_CHECKBOX_CHECK_MARK_PADDING:
                    retval.setExtent(getScaledValue(2));
                break;
                case METRIC_CHECKBOX_CHECK_DELAY:
                    retval.setUint32(200);
                break;
                default:
                    EWM_ASSERT(!"invalid metric ID");
                break;
            }
            return retval;
        }

        void drawScreensaver(const GfxDisplayPtr& display) const final
        {
            EWM_ASSERT(display);
            display->fillScreen(getColor(COLOR_SCREENSAVER));
        }

        void setDefaultFont(const Font* font) final
        {
            _defaultFont = font;
        }

        const Font* getDefaultFont() const final { return _defaultFont; }

        DisplaySize getDisplaySize() const final
        {
            if (_displayWidth <= 320 && _displayHeight <= 320) {
                return DisplaySize::Small;
            } else if (_displayWidth <= 480 && _displayHeight <= 480) {
                return DisplaySize::Medium;
            } else {
                return DisplaySize::Large;
            }
        }

        Extent getScaledValue(Extent value) const final
        {
            switch (getDisplaySize()) {
                default:
                case DisplaySize::Small:
                    return abs(value * 1.0f);
                case DisplaySize::Medium:
                    return abs(value * 2.0f);
                case DisplaySize::Large:
                    return abs(value * 3.0f);
            }
        }

        void drawWindowFrame(const GfxContextPtr& ctx, const Rect& rect,
            Coord radius, Color color) const final
        {
            auto tmp = rect;
            auto pixels = getMetric(METRIC_WINDOW_FRAME_PX).getExtent();
            while (pixels-- > 0) {
                EWM_ASSERT(ctx);
                ctx->drawRoundRect(tmp.left, tmp.top, tmp.width(), tmp.height(), radius, color);
                tmp.deflate(1);
            }
        }

        void drawWindowShadow(const GfxContextPtr& ctx, const Rect& rect,
            Coord radius, Color color) const final
        {
            const auto thickness = getMetric(METRIC_WINDOW_FRAME_PX).getExtent();
            EWM_ASSERT(ctx);
            ctx->drawLine(
                rect.left + radius + thickness,
                rect.bottom,
                rect.left + (rect.width() - (radius + (thickness * 2))),
                rect.bottom,
                color
            );
            ctx->drawLine(
                rect.right,
                rect.top + radius + thickness,
                rect.right,
                rect.top + (rect.height() - (radius + (thickness * 2))),
                color
            );
        }

        void drawWindowBackground(const GfxContextPtr& ctx, const Rect& rect,
            Coord radius, Color color) const final
        {
            EWM_ASSERT(ctx);
            ctx->fillRoundRect(rect.left, rect.top, rect.width(), rect.height(),
                radius, color);
        }

        void drawText(const GfxContextPtr& ctx, const char* text, uint8_t flags,
            const Rect& rect, uint8_t textSize, Color textColor, const Font* font) const final
        {
            EWM_ASSERT(ctx);
            ctx->setTextSize(textSize);
            ctx->setFont(font);

            const bool xCenter = bitsHigh(flags, DT_CENTER);
            const bool singleLine = bitsHigh(flags, DT_SINGLE);

            uint8_t xAdv = 0, yAdv = 0, yAdvMax = 0;
            int8_t xOff = 0, yOff = 0, yOffMin = 0;
            Extent xAccum = 0;
            Extent yAccum;
            if (singleLine) {
                int16_t x, y = rect.top + (rect.height() / 2);
                uint16_t w, h;
                ctx->getTextBounds(text, rect.left, y, &x, &y, &w, &h);
                yAccum = rect.top + (rect.height() / 2) + (h / 2) - 1;
            } else {
                yAccum = rect.top + getMetric(METRIC_Y_PADDING).getExtent();
            }

            const Extent xPadding =
                ((singleLine && !xCenter) ? 0 : getMetric(METRIC_X_PADDING).getExtent());
            const Extent xExtent = rect.right - xPadding;
            const char* cursor = text;

            while (*cursor != '\0') {
                xAccum = rect.left + xPadding;
                const char* old_cursor = cursor;
                std::deque<uint8_t> charXAdvs;
                bool clipped = false;
                while (xAccum <= xExtent && *cursor != '\0') {
                    /// TODO: handle \n and \r
                    getCharBounds(*cursor, nullptr, nullptr, &xAdv, &yAdv, &xOff,
                        &yOff, textSize, font);
                    if (xAccum + xAdv > xExtent) {
                        if (singleLine && bitsHigh(flags, DT_CLIP)) {
                            clipped = true;
                            break;
                        }
                        if (singleLine && bitsHigh(flags, DT_ELLIPSIS)) {
                            auto it = charXAdvs.rbegin();
                            if (it != charXAdvs.rend()) {
                                clipped = true;
                                xAccum -= (*it);
                                charXAdvs.pop_back();
                                cursor--;
                                break;
                            }
                        }
                    }
                    charXAdvs.push_back(xAdv);
                    xAccum += xAdv;
                    cursor++;
                    if (yAdv > yAdvMax) { yAdvMax = yAdv; }
                    if (yOff > yOffMin) { yOffMin = yOff; }
                }
                size_t rewound = 0;
                if (!singleLine) {
                    for (size_t rewind = 0; rewind < (cursor - old_cursor); rewind++) {
                        if (*(cursor - rewind) == ' ') {
                            rewound = rewind;
                            cursor -= rewind;
                            while (rewind > 0) {
                                xAccum -= charXAdvs.at(charXAdvs.size() - rewind);
                                rewind--;
                            }
                            break;
                        }
                    }
                }
                const Extent drawnWidth = xAccum - (rect.left + xPadding);
                xAccum = xCenter
                    ? rect.left + (rect.width() / 2) - (drawnWidth / 2)
                    : rect.left + xPadding;
                while (old_cursor < cursor) {
                    ctx->drawChar(
                        xAccum,
                        yAccum,
                        *old_cursor++,
                        textColor,
                        textColor
# if defined(EWM_GFX_ADAFRUIT)
                        , textSize
# endif
                    );
                    xAccum += charXAdvs[
                        charXAdvs.size() - 2 - ((cursor + rewound) - old_cursor - 1)
                    ];
                }
                if (!singleLine) {
                    if (rewound > 0) { cursor++; }
                    yAccum += yAdvMax + yOffMin;
                } else {
                    if (clipped && bitsHigh(flags, DT_ELLIPSIS)) {
                        getCharBounds('.', nullptr, nullptr, &xAdv, &yAdv,
                            &xOff, &yOff, textSize, font);
                        for (uint8_t ellipsis = 0; ellipsis < 3; ellipsis++) {
                            ctx->drawChar(
                                xAccum,
                                yAccum,
                                '.',
                                textColor,
                                textColor
# if defined(EWM_GFX_ADAFRUIT)
                                , textSize
# endif
                            );
                            xAccum += xAdv;
                        }
                    }
                    break;
                }
            }
        }

        void drawProgressBarBackground(const GfxContextPtr& ctx, const Rect& rect) const final
        {
            EWM_ASSERT(ctx);
            ctx->fillRect(rect.left, rect.top, rect.width(), rect.height(),
                getColor(COLOR_PROGRESS_BG));
        }

        void drawProgressBarProgress(const GfxContextPtr& ctx, const Rect& rect, float percent) const final
        {
            EWM_ASSERT(percent >= 0.0f && percent <= 100.0f);
            auto barRect = rect;
            barRect.deflate(getMetric(METRIC_WINDOW_FRAME_PX).getExtent() * 2);
            barRect.right = barRect.left + abs(barRect.width() * (min(100.0f, percent) / 100.0f));
            EWM_ASSERT(ctx);
            ctx->fillRect(barRect.left, barRect.top, barRect.width(),
                barRect.height(), getColor(COLOR_PROGRESS_FILL));
        }

        void drawProgressBarIndeterminate(const GfxContextPtr& ctx, const Rect& rect, float counter) const final
        {
            EWM_ASSERT(counter >= 0.0f && counter <= 100.0f);
            auto barRect = rect;
            barRect.deflate(getMetric(METRIC_WINDOW_FRAME_PX).getExtent() * 2);
            Extent marqueeWidth
                = (barRect.width() * getMetric(METRIC_PROGBAR_MARQUEE_CX_FACTOR).getFloat());
            Coord offset
                = (barRect.width() + marqueeWidth) * (min(100.0f, counter) / 100.0f);
            static Coord reverseOffset = marqueeWidth;
            Coord x = 0;
            Extent width = 0;
            if (offset < marqueeWidth) {
                x = barRect.left;
                if (counter <= __FLT_EPSILON__) {
                    reverseOffset = marqueeWidth;
                }
                width = offset;
            } else {
                Coord realOffset = reverseOffset > 0
                    ? offset - reverseOffset--
                    : offset;
                x = min(
                    static_cast<Coord>(barRect.left + realOffset),
                    barRect.right
                );
                width = min(
                    marqueeWidth,
                    static_cast<Extent>(barRect.right - x)
                );
            }
            EWM_ASSERT(ctx);
            ctx->fillRect(x, barRect.top, width, barRect.height(),
                getColor(COLOR_PROGRESS_FILL));
        }

        void drawCheckBox(const GfxContextPtr& ctx, const char* lbl, bool checked, const Rect& rect) const final
        {
            const auto radius = getMetric(METRIC_CORNER_RADIUS_CHECKBOX).getCoord();
            drawWindowBackground(ctx, rect, radius, getColor(COLOR_WINDOW_BG));
            auto checkableRect = Rect(
                rect.left,
                rect.top + getMetric(METRIC_CHECKBOX_CHECK_AREA_PADDING).getExtent(),
                rect.left + (rect.height() - (getMetric(METRIC_CHECKBOX_CHECK_AREA_PADDING).getExtent() * 2)),
                rect.top + (rect.height() - getMetric(METRIC_CHECKBOX_CHECK_AREA_PADDING).getExtent())
            );
            checkableRect.top = rect.top + ((rect.height() / 2) - (checkableRect.height() / 2));
            EWM_ASSERT(ctx);
            ctx->fillRoundRect(
                checkableRect.left,
                checkableRect.top,
                checkableRect.width(),
                checkableRect.height(),
                radius,
                getColor(COLOR_CHECKBOX_CHECK_BG)
            );
            drawWindowFrame(ctx, checkableRect, radius, getColor(COLOR_CHECKBOX_CHECK_FRAME));
            if (checked) {
                auto rectCheckMark = checkableRect;
                rectCheckMark.deflate(getMetric(METRIC_CHECKBOX_CHECK_MARK_PADDING).getExtent());
                ctx->fillRoundRect(
                    rectCheckMark.left,
                    rectCheckMark.top,
                    rectCheckMark.width(),
                    rectCheckMark.height(),
                    radius,
                    getColor(COLOR_CHECKBOX_CHECK)
                );
            }
            Rect textRect(
                checkableRect.right + (getMetric(METRIC_CHECKBOX_CHECK_MARK_PADDING).getExtent() * 2),
                rect.top,
                checkableRect.right + (rect.width() - checkableRect.width()),
                rect.top + rect.height()
            );
            drawText(
                ctx,
                lbl,
                DT_SINGLE | DT_ELLIPSIS,
                textRect,
                getMetric(METRIC_DEF_TEXT_SIZE).getUint8(),
                getColor(COLOR_WINDOW_TEXT),
                getDefaultFont()
            );
        }

    private:
        Extent _displayWidth     = 0;
        Extent _displayHeight    = 0;
        const Font* _defaultFont = nullptr;
    };

    struct PackagedMessage
    {
        Message msg = MSG_NONE;
        MsgParam p1 = 0;
        MsgParam p2 = 0;
    };

    using PackagedMessageQueue = std::queue<PackagedMessage>;

    class IWindow;
    class IWindowContainer
    {
    public:
        virtual bool hasChildren() = 0;
        virtual size_t childCount() = 0;
        virtual std::shared_ptr<IWindow> getChildByID(WindowID) = 0;
        virtual bool setForegroundWindow(const std::shared_ptr<IWindow>&) = 0;
        virtual void recalculateZOrder() = 0;
        virtual bool addChild(const std::shared_ptr<IWindow>&) = 0;
        virtual bool removeChildByID(WindowID) = 0;
        virtual void removeAllChildren() = 0;
        virtual void forEachChild(const std::function<bool(const std::shared_ptr<IWindow>&)>&) = 0;
        virtual void forEachChildReverse(const std::function<bool(const std::shared_ptr<IWindow>&)>&) = 0;
    };

    class IWindow : public IWindowContainer
    {
    public:
        virtual std::shared_ptr<IWindow> getParent() const = 0;

        virtual GfxContextPtr getGfxContext() const = 0;

        virtual Rect getRect() const noexcept = 0;
        virtual void setRect(const Rect&) noexcept = 0;

        virtual Rect getClientRect() const noexcept = 0;

        virtual Rect getDirtyRect() const noexcept = 0;
        virtual void markRectDirty(const Rect&) noexcept = 0;

        virtual Style getStyle() const noexcept = 0;
        virtual void setStyle(Style) noexcept = 0;

        virtual WindowID getID() const noexcept = 0;

        virtual uint8_t getZOrder() const noexcept = 0;
        virtual void setZOrder(uint8_t) noexcept = 0;

        virtual State getState() const noexcept = 0;
        virtual void setState(State) noexcept = 0;

        virtual std::string getText() const = 0;
        virtual void setText(const std::string&) = 0;

        virtual Color getBgColor() const noexcept = 0;
        virtual void setBgColor(Color) noexcept = 0;
        virtual Color getTextColor() const noexcept = 0;
        virtual void setTextColor(Color) noexcept = 0;
        virtual Color getFrameColor() const noexcept = 0;
        virtual void setFrameColor(Color) noexcept = 0;
        virtual Color getShadowColor() const noexcept = 0;
        virtual void setShadowColor(Color) noexcept = 0;

        virtual Coord getCornerRadius() const noexcept = 0;
        virtual void setCornerRadius(Coord) noexcept = 0;

        virtual bool routeMessage(Message, MsgParam, MsgParam) = 0;
        virtual bool queueMessage(Message, MsgParam, MsgParam) = 0;
        virtual bool processQueue() = 0;
        virtual bool processInput(InputParams*) = 0;

        virtual bool redraw(bool = false) = 0;
        virtual bool redrawChildren(bool = false) = 0;
        virtual bool hide() noexcept = 0;
        virtual bool show() noexcept = 0;
        virtual bool isVisible() const noexcept = 0;
        virtual bool isAlive() const noexcept = 0;
        virtual bool isDirty() const noexcept = 0;
        virtual bool setDirty(bool,bool=true) noexcept = 0;
        virtual bool isDrawable() const noexcept = 0;

        virtual bool destroy() = 0;

        virtual std::string toString() const = 0;

    protected:
        virtual bool onCreate(MsgParam, MsgParam) = 0;
        virtual bool onDestroy(MsgParam, MsgParam) = 0;
        virtual bool onDraw(MsgParam, MsgParam) = 0;
        virtual bool onPostDraw(MsgParam, MsgParam) = 0;
        virtual bool onInput(MsgParam, MsgParam) = 0;
        virtual bool onEvent(MsgParam, MsgParam) = 0;
        virtual bool onResize(MsgParam, MsgParam) = 0;

        virtual bool onTapped(Coord, Coord) = 0;
    };

    using WindowPtr          = std::shared_ptr<IWindow>;
    using WindowContainerPtr = std::shared_ptr<IWindowContainer>;

    class WindowContainer : public IWindowContainer
    {
    public:
        using WindowDeque = std::deque<WindowPtr>;

        WindowContainer() = default;
        virtual ~WindowContainer() = default;

        bool hasChildren() override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            return !_children.empty();
        }

        size_t childCount() override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            return _children.size();
        }

        WindowPtr getChildByID(WindowID id) override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            for (const auto& win : _children) {
                if (id == win->getID()) {
                    return win;
                }
            }
            return nullptr;
        }

        bool setForegroundWindow(const WindowPtr& win) override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            bool success = false;
            if (!win->getParent() && bitsHigh(win->getStyle(), STY_TOPLEVEL)) {
                for (auto it = _children.begin(); it != _children.end(); it++) {
                    if ((*it)->getID() == win->getID()) {
                        _children.erase(it);
                        _children.push_back(win);
                        success = true;
                        break;
                    }
                }
                if (success) {
                    recalculateZOrder();
                }
            }
            return success;
        }

        void recalculateZOrder() override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            uint8_t zOrder = 0;
            for (const auto& win : _children) {
                win->setZOrder(zOrder++);
            }
        }

        bool addChild(const WindowPtr& child) override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            if (getChildByID(child->getID()) != nullptr) {
                return false;
            }
            uint8_t zOrder = 0;
            auto last = _children.rbegin();
            if (last != _children.rend()) {
                zOrder = (*last)->getZOrder() + 1;
            }
            child->setZOrder(zOrder);
            _children.push_back(child);
            return true;
        }

        bool removeChildByID(WindowID id) override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            for (auto it = _children.begin(); it != _children.end(); it++) {
                if (id == (*it)->getID()) {
                    _children.erase(it);
                    recalculateZOrder();
                    return true;
                }
            }
            return false;
        }

        void removeAllChildren() override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            _children.clear();
        }

        void forEachChild(const std::function<bool(const WindowPtr&)>& cb) override
        {
            if (!cb) {
                return;
            }
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            for (auto child : _children) {
                if (!cb(child)) {
                    break;
                }
            }
        }

        void forEachChildReverse(const std::function<bool(const WindowPtr&)>& cb) override
        {
            if (!cb) {
                return;
            }
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_childMtx);
# endif
            for (auto it = _children.rbegin(); it != _children.rend(); it++) {
                if (!cb((*it))) {
                    break;
                }
            }
        }

    protected:
        WindowDeque _children;
# if !defined(EWM_NOMUTEXES)
        Mutex _childMtx;
# endif
    };

    enum
    {
        WMS_SSAVER_ENABLED = 1 << 0,
        WMS_SSAVER_ACTIVE  = 1 << 1,
        WMS_SSAVER_DRAWN   = 1 << 2
    };

    class WindowManager : public std::enable_shared_from_this<WindowManager>
    {
    public:
        struct Config
        {
            uint32_t minRenderIntervalMsec  = 0U;
            uint32_t minHitTestIntervalMsec = 0U;
        };

        static constexpr uint32_t DefaultMinRenderIntervalMsec  = 100U;
        static constexpr uint32_t DefaultMinHitTestIntervalMsec = 200U;

        WindowManager() = delete;

        explicit WindowManager(
            const GfxDisplayPtr& gfxDisplay,
            const ThemePtr& theme,
            const Font* defaultFont,
            const Config* config = nullptr
        ) : _registry(std::make_shared<WindowContainer>()),
            _gfxDisplay(gfxDisplay), _theme(theme)
        {
            EWM_ASSERT(_registry);
            EWM_ASSERT(_gfxDisplay);
            EWM_ASSERT(_theme);
            _theme->setDefaultFont(defaultFont);
            if (config != nullptr) {
                _config = *config;
            } else {
                _config.minRenderIntervalMsec  = DefaultMinRenderIntervalMsec;
                _config.minHitTestIntervalMsec = DefaultMinHitTestIntervalMsec;
            }
        }

        virtual ~WindowManager()
        {
            tearDown();
        }

        void setState(State state) noexcept { _state = state; }
        State getState() const noexcept { return _state; }

        Config getConfig() const noexcept { return _config; }
        void setConfig(const Config& config) noexcept { _config = config; }

        void enableScreensaver(uint32_t activateAfterMsec) noexcept
        {
            _ssaverActivateAfter = activateAfterMsec;
            _ssaverEpoch = millis();
            setState(getState() | WMS_SSAVER_ENABLED);
            EWM_LOG_D("screensaver enabled (%ums)", activateAfterMsec);
        }

        void disableScreensaver() noexcept
        {
            EWM_CONST(State, flags,
                WMS_SSAVER_ENABLED | WMS_SSAVER_ACTIVE | WMS_SSAVER_DRAWN);
            setState(getState() & ~flags);
            EWM_LOG_D("screensaver disabled");
        }

        virtual void tearDown()
        {
            _registry->forEachChild([](const WindowPtr& child)
            {
                child->destroy();
                return true;
            });
            _registry->removeAllChildren();
        }

        GfxDisplayPtr getGfxDisplay() const { return _gfxDisplay; }
        ThemePtr getTheme() const { return _theme; }

        Extent getDisplayWidth() const noexcept { return _gfxDisplay->width(); }
        Extent getDisplayHeight() const noexcept { return _gfxDisplay->height(); }

        Rect getDisplayRect() const noexcept
        {
            return Rect(0, 0, getDisplayWidth(), getDisplayHeight());
        }

        template<class TWindow>
        std::shared_ptr<TWindow> createWindow(
            const WindowPtr& parent,
            WindowID id,
            Style style,
            Coord x,
            Coord y,
            Extent width,
            Extent height,
            const std::string& text = std::string(),
            const std::function<bool(const std::shared_ptr<TWindow>&)>& preCreateHook = nullptr
        )
        {
            if (id == WID_INVALID) {
                EWM_LOG_E("%hhu is a reserved window ID", WID_INVALID);
                return nullptr;
            }
            if (bitsHigh(style, STY_FULLSCREEN)) {
                x = 0;
                y = 0;
                width = getDisplayWidth();
                height = getDisplayHeight();
            }
            Rect rect(x, y, x + width, y + height);
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            EWM_CONST(size_t, MaxClassName, 32);
            char* demangleBuf = reinterpret_cast<char*>(malloc(MaxClassName));
            size_t dbufSize = MaxClassName;
            int status = 0;
            const auto className = __cxxabiv1::__cxa_demangle(typeid(TWindow).name(),
                demangleBuf, &dbufSize, &status);
            EWM_ASSERT(className != nullptr && status == 0);
            std::shared_ptr<TWindow> win(
                std::make_shared<TWindow>(
                    shared_from_this(), parent, id, style, rect, text, className
                )
            );
            free(demangleBuf);
            demangleBuf = nullptr;
# else
            std::shared_ptr<TWindow> win(
                std::make_shared<TWindow>(
                    shared_from_this(), parent, id, style, rect, text
                )
            );
# endif
            if (bitsHigh(style, STY_CHILD) && !parent) {
                EWM_LOG_E("STY_CHILD && null parent");
                return nullptr;
            }
            if (bitsHigh(style, STY_TOPLEVEL) && parent) {
                EWM_LOG_E("STY_TOPLEVEL && parent");
                return nullptr;
            }
            if (preCreateHook && !preCreateHook(win)) {
                EWM_LOG_E("pre-create hook failed");
                return nullptr;
            }
            if (!win->routeMessage(MSG_CREATE)) {
                EWM_LOG_E("MSG_CREATE = false");
                return nullptr;
            }
            bool dupe = parent ? !parent->addChild(win) : !_registry->addChild(win);
            if (dupe) {
                EWM_LOG_E("duplicate window ID %hhu (parent: %hhu)",
                    id, parent ? parent->getID() : WID_INVALID);
                return nullptr;
            }
            if (bitsHigh(win->getStyle(), STY_AUTOSIZE)) {
                win->routeMessage(MSG_RESIZE);
            }
            win->redraw();
            return win;
        }

        template<class TPrompt>
        std::shared_ptr<TPrompt> createPrompt(
            const WindowPtr& parent,
            WindowID id,
            Style style,
            const std::string& text,
            const std::deque<typename TPrompt::ButtonInfo>& buttons,
            const typename TPrompt::ResultCallback& callback
        )
        {
            EWM_ASSERT(bitsHigh(style, STY_PROMPT));
            const Extent width = min(
                _theme->getMetric(METRIC_MAX_PROMPT_CX).getExtent(),
                static_cast<Extent>(
                    getDisplayWidth() - (_theme->getMetric(METRIC_X_PADDING).getExtent() * 2)
                )
            );
            const Extent height = min(
                _theme->getMetric(METRIC_MAX_PROMPT_CY).getExtent(),
                static_cast<Extent>(
                    getDisplayHeight() - (_theme->getMetric(METRIC_Y_PADDING).getExtent() * 2)
                )
            );
            auto prompt = createWindow<TPrompt>(
                parent,
                id,
                style,
                (getDisplayWidth() / 2) - (width / 2),
                (getDisplayHeight() / 2) - (height / 2),
                width,
                height,
                text,
                [&](const std::shared_ptr<TPrompt>& win)
                {
                    for (const auto& btn : buttons) {
                        if (!win->addButton(btn)) {
                            return false;
                        }
                    }
                    win->setResultCallback(callback);
                    return true;
                }
            );
            return prompt;
        }

        template<class TBar>
        std::shared_ptr<TBar> createProgressBar(
            const WindowPtr& parent,
            WindowID id,
            Style style,
            Coord x,
            Coord y,
            Extent width,
            Extent height,
            Style pbarStyle
        )
        {
            auto pbar = createWindow<TBar>(
                parent, id, style, x, y, width, height
            );
            if (pbar) {
                pbar->setProgressBarStyle(pbarStyle);
            }
            return pbar;
        }

        bool setForegroundWindow(const WindowPtr& win)
        {
            return _registry->setForegroundWindow(win);
        }

        void hitTest(Coord x, Coord y)
        {
            if (millis() - _lastHitTestTime < _config.minHitTestIntervalMsec) {
                return;
            }
            EWM_ASSERT(x >= 0 && y >= 0);
            EWM_ASSERT(x <= getDisplayWidth() && y <= getDisplayHeight());
            EWM_LOG_D("hit test at %hd/%hd", x, y);
            if (bitsHigh(getState(), WMS_SSAVER_ENABLED)) {
                _ssaverEpoch = millis();
                if (bitsHigh(getState(), WMS_SSAVER_ACTIVE)) {
                    return;
                }
            }
            [[maybe_unused]] bool claimed = false;
            _registry->forEachChildReverse([&](const WindowPtr& child)
            {
                if (!child->isDrawable()) {
                    return true;
                }
                EWM_LOG_V("interrogating %s re: hit test at %hd/%hd",
                    child->toString().c_str(), x, y);
                InputParams params;
                params.type = INPUT_TAP;
                params.x    = x;
                params.y    = y;
                if (child->processInput(&params)) {
                    EWM_LOG_V("%s claimed hit test at %hd/%hd",
                        params.handledBy.c_str(), x, y);
                    claimed = true;
                    return false;
                }
                return true;
            });
            if (!claimed) {
                EWM_LOG_V("hit test at %hd/%hd unclaimed", x, y);
            }
            _lastHitTestTime = millis();
        }

        bool isWindowEntirelyCovered(const WindowPtr& win)
        {
            bool covered = false;
            const auto rect = win->getRect();
            _registry->forEachChildReverse([&](const WindowPtr& other)
            {
                if (other == win) {
                    return false;
                }
                if (!other->isDrawable()) {
                    return true;
                }
                if (rect.withinRect(other->getRect())) {
                    covered = true;
                    return false;
                }
                return true;
            });
            return covered;
        }

        virtual void setDirtyRect(const Rect& rect)
        {
            _registry->forEachChild([=](const WindowPtr& win)
            {
                if (!win->isDrawable()) {
                    return true;
                }
                if (win->getRect().intersectsRect(rect)) {
                    const auto intersection = win->getRect().getIntersection(rect);
                    EWM_LOG_V("dirty rect = {%hd, %hd, %hd, %hd}, intersection"
                        " with %s is {%hd, %hd, %hd, %hd}", rect.left, rect.top, rect.right, rect.bottom,
                        win->toString().c_str(), intersection.left, intersection.top, intersection.right,
                        intersection.bottom);
                    win->markRectDirty(intersection);
                }
                return true;
            });
        }

        bool displayToWindow(const WindowPtr& win, Point& pt) const
        {
            const auto windowRect = win->getRect();
            if (windowRect.pointWithin(pt.x, pt.y)) {
                pt.x = pt.x - windowRect.left;
                pt.y = pt.y - windowRect.top;
                return true;
            }
            return false;
        }

        bool windowToDisplay(const WindowPtr& win, Point& pt) const
        {
            const auto windowRect = win->getRect();
            const Rect clientRect(0, 0, windowRect.width(), windowRect.height());
            if (clientRect.pointWithin(pt.x, pt.y)) {
                pt.x = pt.x + windowRect.left;
                pt.y = pt.y + windowRect.top;
                return true;
            }
            return false;
        }

        bool displayToWindow(const WindowPtr& win, Rect& rect) const
        {
            Point topLeft(rect.left, rect.top);
            Point bottomRight(rect.right, rect.bottom);
            if (displayToWindow(win, topLeft) && displayToWindow(win, bottomRight)) {
                rect.left   = topLeft.x;
                rect.top    = topLeft.y;
                rect.right  = bottomRight.x;
                rect.bottom = bottomRight.y;
                return true;
            }
            return false;
        }

        bool windowToDisplay(const WindowPtr& win, Rect& rect) const
        {
            Point topLeft(rect.left, rect.top);
            Point bottomRight(rect.right, rect.bottom);
            if (windowToDisplay(win, topLeft) && windowToDisplay(win, bottomRight)) {
                rect.left   = topLeft.x;
                rect.top    = topLeft.y;
                rect.right  = bottomRight.x;
                rect.bottom = bottomRight.y;
                return true;
            }
            return false;
        }

        virtual void render()
        {
            if (millis() - _lastRenderTime < _config.minRenderIntervalMsec) {
                return;
            }
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            static constexpr uint32_t reportInterval = 30000U;
            static uint32_t lastReport = 0;
            const auto beginTime = micros();
# endif
            bool updated = false;
            if (bitsHigh(getState(), WMS_SSAVER_ENABLED)) {
                if (millis() - _ssaverEpoch >= _ssaverActivateAfter) {
                    if (!bitsHigh(getState(), WMS_SSAVER_ACTIVE)) {
                        setState(getState() | WMS_SSAVER_ACTIVE);
                        EWM_LOG_D("activated screensaver");
                    }
                } else {
                    if (bitsHigh(getState(), WMS_SSAVER_ACTIVE)) {
                        setState(getState() & ~(WMS_SSAVER_ACTIVE | WMS_SSAVER_DRAWN));
                        setDirtyRect(getDisplayRect());
                        EWM_LOG_D("de-activated screensaver");
                    }
                }
            }
            if (bitsHigh(getState(), WMS_SSAVER_ACTIVE)) {
                if (!bitsHigh(getState(), WMS_SSAVER_DRAWN)) {
                    _theme->drawScreensaver(_gfxDisplay);
                    updated = true;
                    setState(getState() | WMS_SSAVER_DRAWN);
                }
            } else {
                _registry->forEachChild([&](const WindowPtr& win)
                {
                    while (win->processQueue()) { }
                    if (!win->isDrawable()) {
                        return true;
                    }
                    auto dirtyRect = win->getDirtyRect();
                    if (dirtyRect.empty()) {
                        return true;
                    }
                    bool obscuringRectSet = false;
                    Rect obscuringRect;
                    _registry->forEachChildReverse([&](const WindowPtr& above)
                    {
                        if (win == above) {
                            return false;
                        }
                        if (!above->isDrawable()) {
                            return true;
                        }
                        const auto aboveRect = above->getRect();
                        if (!obscuringRectSet) {
                            obscuringRect = aboveRect;
                            obscuringRectSet = true;
                        } else {
                            obscuringRect.mergeRect(aboveRect);
                        }
                        return true;
                    });
                    std::queue<Rect> dirtyRects;
                    if (!obscuringRect.empty()) {
                        dirtyRects = dirtyRect.subtractRect(obscuringRect);
                        if (dirtyRects.empty()) {
                            EWM_LOG_V("%s has no dirty rects left after subtracting the"
                                " obscuring rect; clearing dirty rect", win->toString().c_str());
                            win->markRectDirty(Rect());
                            return true;
                        }
                    } else {
                        dirtyRects.push(dirtyRect);
                    }

                    while (!dirtyRects.empty()) {
                        auto clientDirtyRect = dirtyRect = dirtyRects.front();
                        dirtyRects.pop();
                        if (!displayToWindow(win, clientDirtyRect)) {
                            EWM_ASSERT(!"failed to convert display to window coords");
                            return true;
                        }
                        auto ctx = win->getGfxContext();
# if defined(EWM_GFX_ADAFRUIT)
                        _gfxDisplay->startWrite();
                        _gfxDisplay->setAddrWindow(
                            dirtyRect.left,
                            dirtyRect.top,
                            dirtyRect.width(),
                            dirtyRect.height()
                        );
                        for (auto line = clientDirtyRect.top; line < clientDirtyRect.bottom; line++) {
                            const auto offset = ctx->getBuffer() + (line * ctx->width()) + clientDirtyRect.left;
                            _gfxDisplay->writePixels(offset, clientDirtyRect.width());
                        }
                        _gfxDisplay->endWrite();
# elif defined(EWM_GFX_ARDUINO)
#  error "not implemented"
# endif
                        _gfxDisplay->drawRect(
                            dirtyRect.left - 1,
                            dirtyRect.top - 1,
                            dirtyRect.width() + 1,
                            dirtyRect.height() + 1,
                            0xf81f
                        );
                    }
                    win->markRectDirty(Rect());
                    updated = true;

                    return true;
                });
            }

            if (updated) {
                _gfxDisplay->flush();
                _lastRenderTime = millis();
            }

# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            if (millis() - lastReport > reportInterval) {
                _renderAccumCount = max(1U, _renderAccumCount);
                _renderAvg        = _renderAccumTime / _renderAccumCount;
                _renderAccumTime  = 0U;
                _renderAccumCount = 0U;
                EWM_LOG_V("avg. render time: %uμs", _renderAvg);
                lastReport = millis();
                return;
            }
            _renderAccumTime += micros() - beginTime;
            _renderAccumCount++;
# endif
        }

        template<typename... TDisplayArgs>
        inline bool begin(uint8_t rotation, TDisplayArgs&&... args)
        {
            bool success = _gfxDisplay;
            if (success) {
# if defined(EWM_GFX_ADAFRUIT)
                /// TODO: possible in C++11 to deduce return type
                /// of begin() and capture it if it's bool?
                _gfxDisplay->begin(args...);
# elif defined(EWM_GFX_ARDUINO)
#  error "figure out arduino gfx contexts"
# endif
                _gfxDisplay->setRotation(rotation);
                _gfxDisplay->setCursor(0, 0);
            }
            EWM_ASSERT(success);
            if (success) {
                _theme->setDisplayExtents(getDisplayWidth(), getDisplayHeight());
                EWM_LOG_D("display: %hux%hu, rotation: %hhu", getDisplayWidth(),
                    getDisplayHeight(), rotation);
            }
            return success;
        }

    protected:
        Config _config;
        WindowContainerPtr _registry;
        GfxDisplayPtr _gfxDisplay;
        ThemePtr _theme;
        State _state                  = 0;
        uint32_t _ssaverEpoch         = 0U;
        uint32_t _ssaverActivateAfter = 0U;
        uint32_t _lastRenderTime      = 0U;
        uint32_t _lastHitTestTime     = 0U;
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
        uint32_t _renderAvg        = 0U;
        uint32_t _flushAvg         = 0U;
        uint32_t _renderAccumTime  = 0U;
        uint32_t _renderAccumCount = 0U;
        uint32_t _flushAccumTime   = 0U;
        uint32_t _flushAccumCount  = 0U;
# endif
    };

    using WindowManagerPtr = std::shared_ptr<WindowManager>;

    template<class TTheme, class TGfxDisplay>
    WindowManagerPtr createWindowManager(
        const std::shared_ptr<TGfxDisplay>& display,
        const std::shared_ptr<TTheme>& theme,
        const Font* defaultFont
    )
    {
        static_assert(std::is_base_of<GfxDisplay, TGfxDisplay>::value);
        static_assert(std::is_base_of<ITheme, TTheme>::value);
        return std::make_shared<WindowManager>(display, theme, defaultFont);
    }

    class Window : public IWindow, public std::enable_shared_from_this<IWindow>
    {
    public:
        Window() = delete;

        Window(
            const WindowManagerPtr& wm,
            const WindowPtr& parent,
            WindowID id,
            Style style,
            const Rect& rect,
            const std::string& text
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            , const char* className
# endif
        ) : _wm(wm), _parent(parent), _rect(rect), _dirtyRect(rect), _text(text),
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            _className(className),
# endif
            _style(style), _id(id)
        {

            if (bitsHigh(getStyle(), STY_TOPLEVEL) && !parent) {
                _ctx = std::make_shared<GfxContext>(rect.width(), rect.height());
                EWM_LOG_V("created %hux%hu gfx ctx for %s", rect.width(),
                    rect.height(), toString().c_str());
            } else {
                EWM_ASSERT(parent);
                _ctx = parent->getGfxContext();
                EWM_LOG_V("using parent's %hux%hu gfx ctx for %s", _ctx->width(),
                    _ctx->height(), toString().c_str());
            }

            EWM_ASSERT(_ctx && _ctx->getBuffer() != nullptr);
            setDirty(true, false);

            auto theme = _getTheme();
            EWM_ASSERT(theme);
            _bgColor     = theme->getColor(COLOR_WINDOW_BG);
            _textColor   = theme->getColor(COLOR_WINDOW_TEXT);
            _frameColor  = theme->getColor(COLOR_WINDOW_FRAME);
            _shadowColor = theme->getColor(COLOR_WINDOW_SHADOW);
        }

        virtual ~Window() = default;

        bool hasChildren() override { return _children.hasChildren(); }
        size_t childCount() override { return _children.childCount(); }
        WindowPtr getChildByID(WindowID id) override { return _children.getChildByID(id); }

        bool setForegroundWindow([[maybe_unused]] const WindowPtr& win) override
        {
            return false;
        }

        void recalculateZOrder() override { _children.recalculateZOrder(); }
        bool addChild(const WindowPtr& child) override { return _children.addChild(child); }
        bool removeChildByID(WindowID id) override { return _children.removeChildByID(id); }
        void removeAllChildren() override { return _children.removeAllChildren(); }

        void forEachChild(const std::function<bool(const WindowPtr&)>& cb) override
        {
            return _children.forEachChild(cb);
        }

        void forEachChildReverse(const std::function<bool(const WindowPtr&)>& cb) override
        {
            return _children.forEachChildReverse(cb);
        }

        WindowPtr getParent() const override { return _parent; }

        GfxContextPtr getGfxContext() const override { return _ctx; }

        Rect getRect() const noexcept override { return _rect; }

        void setRect(const Rect& rect) noexcept override
        {
            if (rect != _rect) {
                _rect = rect;
                setDirty(true);
            }
        }

        Rect getClientRect() const noexcept override
        {
            auto parent = getParent();
            const auto rect = getRect();
            if (bitsHigh(getStyle(), STY_TOPLEVEL) && !parent) {
                return Rect(0, 0, rect.width(), rect.height());
            } else {
                EWM_ASSERT(parent);
                const auto parentRect = parent->getRect();
                return Rect(
                    rect.left - parentRect.left,
                    rect.top - parentRect.top,
                    (rect.left - parentRect.left) + rect.width(),
                    (rect.top - parentRect.top) + rect.height()
                );
            }
        }

        Rect getDirtyRect() const noexcept
        {
            return _dirtyRect;
        }

        void markRectDirty(const Rect& rect) noexcept
        {
            if (!rect.empty()) {
                const auto windowRect = getRect();
                if (rect.left >= windowRect.left &&
                    (rect.left < _dirtyRect.left || _dirtyRect.left == 0)) {
                    _dirtyRect.left = rect.left;
                }
                if (rect.top >= windowRect.top &&
                    (rect.top < _dirtyRect.top || _dirtyRect.top == 0)) {
                    _dirtyRect.top = rect.top;
                }
                if (rect.right <= windowRect.right && rect.right > _dirtyRect.right) {
                    _dirtyRect.right = rect.right;
                }
                if (rect.bottom <= windowRect.bottom && rect.bottom > _dirtyRect.bottom) {
                    _dirtyRect.bottom = rect.bottom;
                }
            } else {
                _dirtyRect = Rect();
                forEachChild([](const WindowPtr& win)
                {
                    win->markRectDirty(Rect());
                    return true;
                });
            }
        }

        Style getStyle() const noexcept override { return _style; }

        void setStyle(Style style) noexcept override
        {
            if (style != _style) {
                _style = style;
                setDirty(true);
            }
        }

        WindowID getID() const noexcept override { return _id; }

        uint8_t getZOrder() const noexcept override { return _zOrder; }
        void setZOrder(uint8_t zOrder) noexcept override { _zOrder = zOrder; }

        State getState() const noexcept override { return _state; }
        void setState(State state) noexcept override { _state = state; }

        std::string getText() const override { return _text; }

        void setText(const std::string& text) override
        {
            if (text != _text) {
                _text = text;
                setDirty(true);
            }
        }

        Color getBgColor() const noexcept override { return _bgColor; }

        void setBgColor(Color color) noexcept override
        {
            if (color != _bgColor) {
                _bgColor = color;
                setDirty(true);
            }
        }

        Color getTextColor() const noexcept override { return _textColor; }

        void setTextColor(Color color) noexcept override
        {
            if (color != _textColor) {
                _textColor = color;
                setDirty(true);
            }
        }

        Color getFrameColor() const noexcept override { return _frameColor; }

        void setFrameColor(Color color) noexcept override
        {
            if (color != _frameColor) {
                _frameColor = color;
                setDirty(true);
            }
        }

        Color getShadowColor() const noexcept override { return _shadowColor; }

        void setShadowColor(Color color) noexcept override
        {
            if (color != _shadowColor) {
                _shadowColor = color;
                setDirty(true);
            }
        }

        Coord getCornerRadius() const noexcept override { return _cornerRadius; }

        void setCornerRadius(Coord radius) noexcept override
        {
            if (radius != _cornerRadius) {
                _cornerRadius = radius;
                setDirty(true);
            }
        }

        bool routeMessage(Message msg, MsgParam p1 = 0, MsgParam p2 = 0) override
        {
            bool handled = false;
            bool dirty   = false;
            switch (msg) {
                case MSG_CREATE: {
                    dirty = handled = onCreate(p1, p2);
                    if (handled) {
                        setState(getState() | STA_ALIVE);
                    }
                    break;
                }
                case MSG_DESTROY: {
                    handled = onDestroy(p1, p2);
                    setState(getState() & ~STA_ALIVE);
                    break;
                }
                case MSG_DRAW: {
                    if (!isDrawable()) {
                        break;
                    }
                    if (!isDirty() && p1 == 0U) {
                        break;
                    }
                    handled = onDraw(p1, p2);
                    setDirty(false);
                    break;
                }
                case MSG_POSTDRAW:
                    handled = onPostDraw(p1, p2);
                    break;
                case MSG_INPUT: {
                    dirty = handled = onInput(p1, p2);
                    break;
                }
                case MSG_EVENT: return onEvent(p1, p2);
                case MSG_RESIZE: {
                    dirty = handled = onResize(p1, p2);
                    break;
                }
                default:
                    EWM_ASSERT(false);
                    return false;
            }
            if (dirty) {
                setDirty(true);
            }
            return handled;
        }

        bool queueMessage(Message msg, MsgParam p1 = 0, MsgParam p2 = 0) override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_queueMtx);
# endif
            PackagedMessage pm;
            pm.msg    = msg;
            pm.p1     = p1;
            pm.p2     = p2;
            _queue.push(pm);
            return msg == MSG_INPUT && getMsgParamLoWord(p1) == INPUT_TAP;
        }

        bool processQueue() override
        {
# if !defined(EWM_NOMUTEXES)
            ScopeLock lock(_queueMtx);
# endif
            if (!_queue.empty()) {
                auto pm = _queue.front();
                _queue.pop();
                routeMessage(pm.msg, pm.p1, pm.p2);
            }
            forEachChild([&](const WindowPtr& child)
            {
                child->processQueue();
                return true;
            });
            return !_queue.empty();
        }

        bool processInput(InputParams* params) override
        {
            if (!isDrawable()) {
                return false;
            }
            if (!getRect().pointWithin(params->x, params->y)) {
                return false;
            }
            bool handled = false;
            forEachChildReverse([&](const WindowPtr& child)
            {
                handled = child->processInput(params);
                if (handled) {
                    return false;
                }
                return true;
            });
            if (!handled) {
                handled = queueMessage(
                    MSG_INPUT,
                    makeMsgParam(0, params->type),
                    makeMsgParam(params->x, params->y)
                );
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
                if (handled) {
                    params->handledBy = std::move(toString());
                }
# endif
            }
            return handled;
        }

        bool redraw(bool force = false) override
        {
            if (!isDrawable()) {
                return false;
            }
            bool redrawn = (isDirty() || force)
                ? routeMessage(MSG_DRAW, force ? 1U : 0U) : false;
            bool childRedrawn = false;
            if (redrawn) {
                forEachChild([](const WindowPtr& child)
                {
                    child->setDirty(true, false);
                    return true;
                });
            }
            childRedrawn = redrawChildren(force);
            return redrawn || childRedrawn;
        }

        bool redrawChildren(bool force = false) override
        {
            bool childRedrawn = false;
            forEachChild([&](const WindowPtr& child)
            {
                if (child->isDirty() || force) {
                    if (child->redraw(force)) {
                        childRedrawn = true;
                    }
                }
                return true;
            });
            return childRedrawn;
        }

        bool hide() noexcept override
        {
            if (!isVisible()) {
                return false;
            }
            setStyle(getStyle() & ~STY_VISIBLE);
            auto wm = _getWM();
            EWM_ASSERT(wm);
            wm->setDirtyRect(getRect());
            return true;
        }

        bool show() noexcept override
        {
            const auto topLevel = bitsHigh(getStyle(), STY_TOPLEVEL);
            EWM_ASSERT(!topLevel || !getParent());
            if (!topLevel && isVisible()) {
                return false;
            }
            auto shown = true;
            if (topLevel) {
                auto wm = _getWM();
                shown = wm->setForegroundWindow(shared_from_this());
            }
            setStyle(getStyle() | STY_VISIBLE);
            return shown && setDirty(true);
        }

        bool isVisible() const noexcept override
        {
            const auto rect = getRect();
            return bitsHigh(getStyle(), STY_VISIBLE) && !rect.empty();
        }

        bool isAlive() const noexcept override
        {
            return bitsHigh(getState(), STA_ALIVE);
        }

        bool isDirty() const noexcept override
        {
            return bitsHigh(getState(), STA_DIRTY);
        }

        bool setDirty(bool dirty, bool redrawWindow = true) noexcept override
        {
            if (dirty) {
                setState(getState() | STA_DIRTY);
                if (redrawWindow) {
                    return redraw();
                }
            } else {
                setState(getState() & ~STA_DIRTY);
            }
            return true;
        }

        bool isDrawable() const noexcept override
        {
            const auto wm = _getWM();
            const auto parent = getParent();
            return isVisible() &&
                   isAlive() &&
                   (!parent || (parent && parent->isDrawable())) &&
                   !getRect().outsideRect(wm->getDisplayRect());
        }

        bool destroy() override
        {
            hide();
            bool destroyed = routeMessage(MSG_DESTROY);
            forEachChild([&](const WindowPtr& child)
            {
                destroyed &= child->destroy();
                return true;
            });
            removeAllChildren();
            return destroyed;
        }

        std::string toString() const override
        {
            std::string retval;
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
            retval = _className;
# endif
            retval += " (id: " + std::to_string(getID()); /* + ", state: ";
            retval += bitmaskToString(getState()) + ", style: ";
            retval += bitmaskToString(getStyle()); */
            retval += ")";
            return std::move(retval);
        }

    protected:
        // ====== Begin message handlers ======

        // MSG_CREATE: p1 = 0, p2 = 0.
        bool onCreate(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            setCornerRadius(theme->getMetric(METRIC_CORNER_RADIUS_WINDOW).getCoord());
            return true;
        }

        // MSG_DESTROY: p1 = 0, p2 = 0.
        bool onDestroy([[maybe_unused]] MsgParam p1, [[maybe_unused]] MsgParam p2) override
        {
            return true;
        }

        // MSG_DRAW: p1 = 1 (force) || 0, p2 = 0.
        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            EWM_LOG_V("%s", toString().c_str());
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            theme->drawWindowBackground(_ctx, getClientRect(), getCornerRadius(), getBgColor());
            if (bitsHigh(getStyle(), STY_FRAME)) {
                theme->drawWindowFrame(_ctx, getClientRect(), getCornerRadius(), getFrameColor());
            }
            if (bitsHigh(getStyle(), STY_SHADOW)) {
                theme->drawWindowShadow(_ctx, getClientRect(), getCornerRadius(), getShadowColor());
            }
            return routeMessage(MSG_POSTDRAW);
        }

        // MSG_POSTDRAW: p1 = 0, p2 = 0.
        bool onPostDraw(MsgParam p1, MsgParam p2) override
        {
            markRectDirty(getRect());
            auto parent = getParent();
            if (parent) {
                parent->markRectDirty(getRect());
            }
            return true;
        }

        // MSG_INPUT: p1 = (loword: type), p2 = (hiword: x, loword: y).
        // Returns true if the input event was consumed by this window, false otherwise.
        bool onInput(MsgParam p1, MsgParam p2) override
        {
            InputParams params;
            params.type = static_cast<InputType>(getMsgParamLoWord(p1));
            params.x    = getMsgParamHiWord(p2);
            params.y    = getMsgParamLoWord(p2);
            switch (params.type) {
                case INPUT_TAP: return onTapped(params.x, params.y);
                default:
                    EWM_ASSERT(false);
                break;
            }
            return false;
        }

        // MSG_EVENT: p1 = EventType, p2 = child WindowID.
        bool onEvent(MsgParam p1, MsgParam p2) override { return true; }

        // MSG_RESIZE: p1 = 0, p2 = 0.
        bool onResize(MsgParam p1, MsgParam p2) override
        {
            EWM_ASSERT(bitsHigh(getStyle(), STY_AUTOSIZE));
            return false;
        }

        // ====== End message handlers ======

        bool onTapped([[maybe_unused]] Coord x, [[maybe_unused]] Coord y) override
        {
            return false;
        }

        WindowManagerPtr _getWM() const { return _wm; }

        ThemePtr _getTheme() const
        {
            auto wm = _getWM();
            return wm ? wm->getTheme() : nullptr;
        }

    protected:
        WindowContainer _children;
        PackagedMessageQueue _queue;
# if !defined(EWM_NOMUTEXES)
        Mutex _queueMtx;
# endif
        WindowManagerPtr _wm;
        WindowPtr _parent;
        GfxContextPtr _ctx;
        Rect _rect;
        Rect _dirtyRect;
        std::string _text;
# if EWM_LOG_LEVEL >= EWM_LOG_LEVEL_VERBOSE
        std::string _className;
# endif
        Style _style        = 0;
        WindowID _id        = WID_INVALID;
        uint8_t _zOrder     = 0;
        State _state        = 0;
        Color _bgColor      = 0;
        Color _textColor    = 0;
        Color _frameColor   = 0;
        Color _shadowColor  = 0;
        Coord _cornerRadius = 0;
    };

    class Button : public Window
    {
    public:
        using Window::Window;
        virtual ~Button() = default;

        bool onTapped(Coord x, Coord y) override
        {
            auto parent = getParent();
            EWM_ASSERT(parent);
            if (parent) {
                parent->queueMessage(MSG_EVENT, EVT_CHILD_TAPPED, getID());
            }
            _lastTapped = millis();
            return parent != nullptr;
        }

        bool onCreate(MsgParam p1, MsgParam p2) override
        {
            if (!Window::onCreate(p1, p2)) {
                return false;
            }

            auto theme = _getTheme();
            EWM_ASSERT(theme);
            setCornerRadius(theme->getMetric(METRIC_CORNER_RADIUS_BUTTON).getCoord());
            return true;
        }

        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            const bool pressed = (millis() - _lastTapped <
                theme->getMetric(METRIC_BUTTON_TAPPED_DURATION).getUint32());
            theme->drawWindowBackground(
                _ctx,
                getClientRect(),
                theme->getMetric(METRIC_CORNER_RADIUS_BUTTON).getCoord(),
                theme->getColor(pressed ? COLOR_BUTTON_BG_PRESSED : COLOR_BUTTON_BG)
            );
            theme->drawWindowFrame(
                _ctx,
                getClientRect(),
                theme->getMetric(METRIC_CORNER_RADIUS_BUTTON).getCoord(),
                theme->getColor(pressed ? COLOR_BUTTON_FRAME_PRESSED : COLOR_BUTTON_FRAME)
            );
            theme->drawText(
                _ctx,
                getText().c_str(),
                DT_SINGLE | DT_CENTER,
                getClientRect(),
                theme->getMetric(METRIC_DEF_TEXT_SIZE).getUint8(),
                theme->getColor(pressed ? COLOR_BUTTON_TEXT_PRESSED : COLOR_BUTTON_TEXT),
                theme->getDefaultFont()
            );
            return routeMessage(MSG_POSTDRAW);
        }

        bool onResize(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            Coord x, y;
            Extent width, height;
            auto rect = getRect();
            _ctx->getTextBounds(getText().c_str(), rect.left, rect.top, &x, &y, &width, &height);
            const auto maxWidth = max(width, theme->getMetric(METRIC_DEF_BUTTON_CX).getExtent());
            rect.right = rect.left + maxWidth +
                (theme->getMetric(METRIC_BUTTON_LABEL_PADDING).getExtent() * 2);
            rect.bottom = rect.top + theme->getMetric(METRIC_DEF_BUTTON_CY).getExtent();
            setRect(rect);
            /// TODO: if not autosize, clip label, perhaps with ellipsis.
            return true;
        }

    protected:
        u_long _lastTapped = 0UL;
    };

    class Label : public Window
    {
    public:
        using Window::Window;
        Label() = default;
        virtual ~Label() = default;

        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            theme->drawWindowBackground(_ctx, getClientRect(), getCornerRadius(), getBgColor());
            theme->drawText(
                _ctx,
                getText().c_str(),
                DT_SINGLE | DT_ELLIPSIS,
                getClientRect(),
                theme->getMetric(METRIC_DEF_TEXT_SIZE).getUint8(),
                getTextColor(),
                theme->getDefaultFont()
            );
            return routeMessage(MSG_POSTDRAW);
        }
    };

    class MultilineLabel : public Window
    {
    public:
        using Window::Window;
        MultilineLabel() = default;
        virtual ~MultilineLabel() = default;

        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            theme->drawWindowBackground(_ctx, getClientRect(), getCornerRadius(), getBgColor());
            theme->drawText(
                _ctx,
                getText().c_str(),
                DT_CENTER,
                getClientRect(),
                theme->getMetric(METRIC_DEF_TEXT_SIZE).getUint8(),
                getTextColor(),
                theme->getDefaultFont()
            );
            return routeMessage(MSG_POSTDRAW);
        }
    };

    class Prompt : public Window
    {
    public:
        EWM_CONST(WindowID, LabelID, 1);

        using ButtonInfo     = std::pair<WindowID, std::string>;
        using ResultCallback = std::function<void(WindowID)>;

        using Window::Window;
        Prompt() = default;
        virtual ~Prompt() = default;

        void setResultCallback(const ResultCallback& callback)
        {
            _callback = callback;
        }

        bool addButton(const ButtonInfo& bi)
        {
            auto wm = _getWM();
            EWM_ASSERT(wm);
            auto btn = wm->createWindow<Button>(
                shared_from_this(),
                bi.first,
                STY_CHILD | STY_VISIBLE | STY_AUTOSIZE | STY_BUTTON,
                0,
                0,
                0,
                0,
                bi.second
            );
            return btn != nullptr;
        }

        bool onCreate(MsgParam p1, MsgParam p2) override
        {
            auto wm = _getWM();
            EWM_ASSERT(wm);
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            setCornerRadius(theme->getMetric(METRIC_CORNER_RADIUS_PROMPT).getCoord());
            setBgColor(theme->getColor(COLOR_PROMPT_BG));
            setFrameColor(theme->getColor(COLOR_PROMPT_FRAME));
            setShadowColor(theme->getColor(COLOR_PROMPT_SHADOW));
            const auto rect = getRect();
            const auto xPadding = theme->getMetric(METRIC_X_PADDING).getExtent();
            const auto yPadding = theme->getMetric(METRIC_Y_PADDING).getExtent();
            const auto defBtnHeight = theme->getMetric(METRIC_DEF_BUTTON_CY).getExtent();
            _label = wm->createWindow<MultilineLabel>(
                shared_from_this(),
                LabelID,
                STY_CHILD | STY_VISIBLE | STY_LABEL,
                rect.left + xPadding,
                rect.top + yPadding,
                rect.width() - (xPadding * 2),
                rect.height() - ((yPadding * 3) + defBtnHeight),
                getText()
            );
            if (!_label) {
                return false;
            }
            _label->setBgColor(theme->getColor(COLOR_PROMPT_BG));
            auto rectLbl = _label->getRect();
            /// TODO: refactor this. prompts should have styles, such as
            // PROMPT_1BUTTON and PROMPT_2BUTTON. they should then take
            // the appropriate button metadata in their constructor
            bool first = true;
            uint8_t numButtons = 0;
            forEachChild([&](const WindowPtr& child)
            {
                if (bitsHigh(child->getStyle(), STY_BUTTON)) {
                    numButtons++;
                }
                return true;
            });
            forEachChild([&](const WindowPtr& child)
            {
                if (!bitsHigh(child->getStyle(), STY_BUTTON)) {
                    return true;
                }
                auto rectBtn = child->getRect();
                rectBtn.top = rectLbl.bottom + yPadding;
                rectBtn.bottom = rectBtn.top + defBtnHeight;
                auto width = rectBtn.width();
                if (first) {
                    first = false;
                    switch (numButtons) {
                        case 1: {
                            rectBtn.left
                                = rect.left + (rect.width() / 2) - (width / 2);
                        }
                        break;
                        case 2: {
                            rectBtn.left = rect.left + xPadding;
                        }
                        break;
                        default:
                            EWM_ASSERT(false);
                        return false;
                    }
                    rectBtn.right = rectBtn.left + width;
                } else {
                    rectBtn.right = rect.right - xPadding;
                    rectBtn.left = rectBtn.right - width;
                }
                child->setRect(rectBtn);
                return true;
            });
            return true;
        }

        bool onEvent(MsgParam p1, MsgParam p2) override
        {
            switch (static_cast<EventType>(p1)) {
                case EVT_CHILD_TAPPED:
                    hide();
                    if (_callback) {
                        _callback(static_cast<WindowID>(p2));
                    }
                return true;
                default:
                    EWM_ASSERT(false);
                break;
            }
            return false;
        }

    protected:
        WindowPtr _label;
        ResultCallback _callback;
    };

    class ProgressBar : public Window
    {
    public:
        using Window::Window;
        ProgressBar() = default;
        virtual ~ProgressBar() = default;

        Style getProgressBarStyle() const noexcept { return _barStyle; }

        void setProgressBarStyle(Style barStyle) noexcept
        {
            if (barStyle != _barStyle) {
                _barStyle = barStyle;
                setDirty(true);
            }
        }

        float getProgressValue() const noexcept { return _value; }

        void setProgressValue(float value) noexcept
        {
            if (abs(value) != abs(_value)) {
                _value = value;
                setDirty(true);
            }
        }

    protected:
        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            theme->drawProgressBarBackground(_ctx, getClientRect());
            theme->drawWindowFrame(_ctx, getClientRect(), getCornerRadius(), getFrameColor());
            bool drawn = false;
            if (bitsHigh(getProgressBarStyle(), PBR_NORMAL)) {
                theme->drawProgressBarProgress(_ctx, getClientRect(), getProgressValue());
                drawn = true;
            } else if (bitsHigh(getProgressBarStyle(), PBR_INDETERMINATE)) {
                theme->drawProgressBarIndeterminate(_ctx, getClientRect(), getProgressValue());
                drawn = true;
            }
            return drawn ? routeMessage(MSG_POSTDRAW) : false;
        }

        Style _barStyle = 0;
        float _value = 0.0f;
    };

    class CheckBox : public Window
    {
    public:
        using Window::Window;
        CheckBox() = default;
        virtual ~CheckBox() = default;

        void setChecked(bool checked)
        {
            if (isChecked() != checked) {
                if (checked) {
                    setState(getState() | STA_CHECKED);
                } else {
                    setState(getState() & ~STA_CHECKED);
                }
                setDirty(true);
            }
        }

        bool isChecked() const noexcept { return bitsHigh(getState(), STA_CHECKED); }

    protected:
        bool onDraw(MsgParam p1, MsgParam p2) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            theme->drawCheckBox(_ctx, getText().c_str(), isChecked(), getClientRect());
            return routeMessage(MSG_POSTDRAW);
        }

        bool onTapped(Coord x, Coord y) override
        {
            auto theme = _getTheme();
            EWM_ASSERT(theme);
            if (millis() - _lastToggle >=
                theme->getMetric(METRIC_CHECKBOX_CHECK_DELAY).getUint32()) {
                setChecked(!isChecked());
                _lastToggle = millis();
            }
            return true;
        }

    private:
        u_long _lastToggle = 0UL;
    };
} // namespace exostra

#endif // !_EXOSTRA_H_INCLUDED