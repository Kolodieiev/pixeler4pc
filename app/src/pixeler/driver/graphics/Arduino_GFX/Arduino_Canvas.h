/**
 * @file Arduino_Canvas.h
 * @author Kolodieiev
 * @brief Цей клас портує логіку малювання Arduino_Canvas бібліотеки Arduino_GFX, задля швидкої розробки GUI для мікроконтролерів на ПК.
 * @version 0.1
 * @date 2025-10-28
 */

#pragma once
#pragma GCC optimize("O3")

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <mutex>
#include <thread>

#include "../U8g2/u8g2.h"
#include "../graphics_setup.h"

namespace pixeler
{
  // Default color definitions
#define COLOR_BLACK 0x0000
#define COLOR_NAVY 0x000F
#define COLOR_DARKGREEN 0x03E0
#define COLOR_DARKCYAN 0x03EF
#define COLOR_MAROON 0x7800
#define COLOR_PURPLE 0x780F
#define COLOR_OLIVE 0x7BE0
#define COLOR_LIGHTGREY 0xD69A
#define COLOR_LIME 0xA7E0
#define COLOR_DARKGREY 0x3186
#define COLOR_GREY 0xAD55
#define COLOR_BLUE 0x001F
#define COLOR_GREEN 0x07E0
#define COLOR_CYAN 0x07FF
#define COLOR_RED 0xF800
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE 0xFFFF
#define COLOR_ORANGE 0xFDA0
#define COLOR_GREENYELLOW 0xB7E0
#define COLOR_PINK 0xFE19
#define COLOR_BROWN 0x9A60
#define COLOR_GOLD 0xFEA0
#define COLOR_SILVER 0xC618
#define COLOR_SKYBLUE 0x867D
#define COLOR_VIOLET 0x915C

#define COLOR_TRANSPARENT 0xF81F  // Колір rgb(255, 0, 255)

  class Arduino_Canvas
  {
  public:
    Arduino_Canvas(uint16_t width, uint16_t height);
    ~Arduino_Canvas();

    bool begin(sf::RenderWindow* window);

    uint16_t* getFramebuffer();

    void flushMainBuff();

    void writePixelPreclipped(int16_t x, int16_t y, uint16_t color);

    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    void draw16bitRGBBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    void draw16bitRGBBitmapWithTranColor(int16_t x, int16_t y, const uint16_t* bitmap, uint16_t transparent_color, int16_t w, int16_t h);

    uint16_t width() const;
    uint16_t height() const;

    void setTextWrap(bool state);

    /*!
      @brief   Set text font color with transparant background
      @param   c   16-bit 5-6-5 Color to draw text with
      @note    For 'transparent' background, background and foreground
               are set to same color rather than using a separate flag.
    */
    void setTextColor(uint16_t c);

    /*!
      @brief   Set text font color with custom background color
      @param   c   16-bit 5-6-5 Color to draw text with
      @param   bg  16-bit 5-6-5 Color to draw background/fill with
    */
    void setTextColor(uint16_t c, uint16_t bg);

    /*!
      @brief  Fill the screen completely with one color. Update in subclasses if desired!
      @param  color 16-bit 5-6-5 Color to fill with
    */
    void fillScreen(uint16_t color);

    /*!
      @brief  Fill a rectangle completely with one color. Update in subclasses if desired!
      @param  x       Top left corner x coordinate
      @param  y       Top left corner y coordinate
      @param  w       Width in pixels
      @param  h       Height in pixels
      @param  color   16-bit 5-6-5 Color to fill with
    */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /*!
      @brief  Set text cursor location
      @param  x    X coordinate in pixels
      @param  y    Y coordinate in pixels
    */
    void setCursor(int16_t x, int16_t y);

    void setFont(const uint8_t* font);

    /*!
      @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
      @param  s   Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
    */
    void setTextSize(uint8_t s);

    /*!
      @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
      @param  s_x Desired text width magnification level in X-axis. 1 is default
      @param  s_y Desired text width magnification level in Y-axis. 1 is default
    */
    void setTextSize(uint8_t s_x, uint8_t s_y);

    /*!
      @brief  Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
      @param  s_x             Desired text width magnification level in X-axis. 1 is default
      @param  s_y             Desired text width magnification level in Y-axis. 1 is default
      @param  pixel_margin    Margin for each text pixel. 0 is default
    */
    void setTextSize(uint8_t s_x, uint8_t s_y, uint8_t pixel_margin);

    /*!
      @brief  Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
      @param  str The ascii string to measure
      @param  x   The current cursor X
      @param  y   The current cursor Y
      @param  x1  The boundary X coordinate, set by function
      @param  y1  The boundary Y coordinate, set by function
      @param  w   The boundary width, set by function
      @param  h   The boundary height, set by function
    */
    void getTextBounds(const char* string, int16_t x, int16_t y, int16_t& x1, int16_t& y1, uint16_t& w, uint16_t& h);

    void drawPixel(int16_t x, int16_t y, uint16_t color);

    /*!
      @brief  Write a line. Check straight or slash line and call corresponding function
      @param  x0      Start point x coordinate
      @param  y0      Start point y coordinate
      @param  x1      End point x coordinate
      @param  y1      End point y coordinate
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    /*!
      @brief  Write a line.  Bresenham's algorithm - thx wikpedia
      @param  x0      Start point x coordinate
      @param  y0      Start point y coordinate
      @param  x1      End point x coordinate
      @param  y1      End point y coordinate
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void writeSlashLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    /*!
      @brief  Draw a circle outline
      @param  x       Center-point x coordinate
      @param  y       Center-point y coordinate
      @param  r       Radius of circle
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

    /*!
      @brief  Draw a circle with filled color
      @param  x       Center-point x coordinate
      @param  y       Center-point y coordinate
      @param  r       Radius of circle
      @param  color   16-bit 5-6-5 Color to fill with
    */
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

    /*!
      @brief  Draw a rectangle with no fill color
      @param  x       Top left corner x coordinate
      @param  y       Top left corner y coordinate
      @param  w       Width in pixels
      @param  h       Height in pixels
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /*!
      @brief  Draw a rounded rectangle with no fill color
      @param  x       Top left corner x coordinate
      @param  y       Top left corner y coordinate
      @param  w       Width in pixels
      @param  h       Height in pixels
      @param  r       Radius of corner rounding
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

    /*!
      @brief  Draw a rounded rectangle with fill color
      @param  x       Top left corner x coordinate
      @param  y       Top left corner y coordinate
      @param  w       Width in pixels
      @param  h       Height in pixels
      @param  r       Radius of corner rounding
      @param  color   16-bit 5-6-5 Color to draw/fill with
    */
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);

    /*!
      @brief  Draw a triangle with no fill color
      @param  x0      Vertex #0 x coordinate
      @param  y0      Vertex #0 y coordinate
      @param  x1      Vertex #1 x coordinate
      @param  y1      Vertex #1 y coordinate
      @param  x2      Vertex #2 x coordinate
      @param  y2      Vertex #2 y coordinate
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

    /*!
      @brief  Draw a triangle with color-fill
      @param  x0      Vertex #0 x coordinate
      @param  y0      Vertex #0 y coordinate
      @param  x1      Vertex #1 x coordinate
      @param  y1      Vertex #1 y coordinate
      @param  x2      Vertex #2 x coordinate
      @param  y2      Vertex #2 y coordinate
      @param  color   16-bit 5-6-5 Color to fill/draw with
    */
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

    size_t print(const char* str);
    size_t print(char c);
    size_t write(const uint8_t* buffer, size_t size);
    size_t write(uint8_t c);

    /*!
      @brief  Draw a single character
      @param  x       Bottom left corner x coordinate
      @param  y       Bottom left corner y coordinate
      @param  c       The 8-bit font-indexed character (likely ascii)
      @param  color   16-bit 5-6-5 Color to draw chraracter with
      @param  bg      16-bit 5-6-5 Color to fill background with (if same as color, no background)
    */
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg);

  protected:
    void convertToRGBA(const uint16_t* buf565, uint8_t* out_buf_rgba8, unsigned pixels);
    void renderWindow();

    /*!
      @brief  Quarter-ellipse drawer, used to do circles and roundrects
      @param  x           Center-point x coordinate
      @param  y           Center-point y coordinate
      @param  rx          radius of x coordinate
      @param  ry          radius of y coordinate
      @param  cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
      @param  color       16-bit 5-6-5 Color to draw with
    */
    void writeEllipseHelper(int32_t x, int32_t y, int32_t rx, int32_t ry, uint8_t cornername, uint16_t color);

    /*!
      @brief  Quarter-circle drawer with fill, used for circles and roundrects
      @param  x       Center-point x coordinate
      @param  y       Center-point y coordinate
      @param  rx      Radius of x coordinate
      @param  ry      Radius of y coordinate
      @param  corners Mask bits indicating which quarters we're doing
      @param  delta   Offset from center-point, used for round-rects
      @param  color   16-bit 5-6-5 Color to fill with
    */
    void writeFillEllipseHelper(int32_t x, int32_t y, int32_t rx, int32_t ry, uint8_t cornername, int16_t delta, uint16_t color);

    /*!
      @brief  Draw an ellipse outline
      @param  x       Center-point x coordinate
      @param  y       Center-point y coordinate
      @param  rx      radius of x coordinate
      @param  ry      radius of y coordinate
      @param  start   degree of ellipse start
      @param  end     degree of ellipse end
      @param  color   16-bit 5-6-5 Color to draw with
    */
    void drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color);

    /*!
      @brief  Draw an ellipse with filled color
      @param  x       Center-point x coordinate
      @param  y       Center-point y coordinate
      @param  rx      radius of x coordinate
      @param  ry      radius of y coordinate
      @param  start   degree of ellipse start
      @param  end     degree of ellipse end
      @param  color   16-bit 5-6-5 Color to fill with
    */
    void fillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t color);

    /*!
      @brief  Draw a filled rectangle to the display. Not self-contained;
      should follow startWrite(). Typically used by higher-level
      graphics primitives; user code shouldn't need to call this and
      is likely to use the self-contained fillRect() instead.
      writeFillRect() performs its own edge clipping and rejection;
      see writeFillRectPreclipped() for a more 'raw' implementation.
      @param  x       Horizontal position of first corner.
      @param  y       Vertical position of first corner.
      @param  w       Rectangle width in pixels (positive = right of first
        corner, negative = left of first corner).
      @param  h       Rectangle height in pixels (positive = below first
        corner, negative = above first corner).
      @param  color   16-bit fill color in '565' RGB format.
      @note   Written in this deep-nested way because C by definition will
      optimize for the 'if' case, not the 'else' -- avoids branches
      and rejects clipped rectangles at the least-work possibility.
    */
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /*!
      @brief  Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
      @param  x       Top left corner x coordinate
      @param  y       Top left corner y coordinate
      @param  w       Width in pixels
      @param  h       Height in pixels
      @param  color   16-bit 5-6-5 Color to fill with
    */
    void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    uint16_t u8g2_font_get_word(const uint8_t* font, uint8_t offset);
    uint8_t u8g2_font_decode_get_unsigned_bits(uint8_t cnt);
    int8_t u8g2_font_decode_get_signed_bits(uint8_t cnt);
    void u8g2_font_decode_len(uint8_t len, uint8_t is_foreground, uint16_t color, uint16_t bg);

    /*!
      @brief   Given 8-bit red, green and blue values, return a 'packed'
              16-bit color value in '565' RGB format (5 bits red, 6 bits
              green, 5 bits blue). This is just a mathematical operation,
              no hardware is touched.
      @param   red    8-bit red brightnesss (0 = off, 255 = max).
      @param   green  8-bit green brightnesss (0 = off, 255 = max).
      @param   blue   8-bit blue brightnesss (0 = off, 255 = max).
      @return  'Packed' 16-bit color value (565 format).
    */
    uint16_t color565(uint8_t red, uint8_t green, uint8_t blue);

    /*!
      @brief  Get text cursor X location
      @returns    X coordinate in pixels
    */
    int16_t getCursorX(void) const;

    /*!
      @brief      Get text cursor Y location
      @returns    Y coordinate in pixels
    */
    int16_t getCursorY(void) const;

  protected:
    uint16_t* _framebuffer{nullptr};
    uint8_t* _opengl_rgba_buff{nullptr};

    sf::RenderWindow* _window{nullptr};

#if SFML_VERSION_MAJOR > 2
    sf::Image _image{{TFT_WIDTH, TFT_HEIGHT}, _opengl_rgba_buff};
    sf::Texture _texture{_image};
    sf::Sprite _sprite{_texture};

#else  // SFML_VERSION_MAJOR < 3
    sf::Image _image;
    sf::Texture _texture;
    sf::Sprite _sprite;

#endif  // #if SFML_VERSION_MAJOR > 2

    const size_t BUFF_SIZE;
    const uint16_t MAX_X;
    const uint16_t MAX_Y;

    uint16_t textcolor{0xFFFF};    ///< 16-bit background color for print()
    uint16_t textbgcolor{0xFFFF};  ///< 16-bit text color for print()

    int16_t cursor_x{0};  ///< x location to start print()ing text
    int16_t cursor_y{0};  ///< y location to start print()ing text

    int16_t _min_text_x{0};
    int16_t _min_text_y{0};
    int16_t _max_text_x;
    int16_t _max_text_y;

    const uint8_t* u8g2Font;
    const uint8_t* _u8g2_decode_ptr;

    uint16_t _encoding;
    uint16_t _u8g2_start_pos_upper_A;
    uint16_t _u8g2_start_pos_lower_a;
    uint16_t _u8g2_start_pos_unicode;
    int16_t _u8g2_target_x;
    int16_t _u8g2_target_y;
    uint8_t _utf8_state = 0;
    uint8_t _u8g2_glyph_cnt;
    uint8_t _u8g2_bits_per_0;
    uint8_t _u8g2_bits_per_1;
    uint8_t _u8g2_bits_per_char_width;
    uint8_t _u8g2_bits_per_char_height;
    uint8_t _u8g2_bits_per_char_x;
    uint8_t _u8g2_bits_per_char_y;
    uint8_t _u8g2_bits_per_delta_x;
    int8_t _u8g2_max_char_width;
    int8_t _u8g2_max_char_height;
    uint8_t _u8g2_first_char;
    uint8_t _u8g2_char_width;
    uint8_t _u8g2_char_height;
    int8_t _u8g2_char_x;
    int8_t _u8g2_char_y;
    int8_t _u8g2_delta_x;
    int8_t _u8g2_dx;
    int8_t _u8g2_dy;
    uint8_t _u8g2_decode_bit_pos;

    uint8_t textsize_x{1};      ///< Desired magnification in X-axis of text to print()
    uint8_t textsize_y{1};      ///< Desired magnification in Y-axis of text to print()
    uint8_t vtextsize_y{0};     ///< Desired magnification in Y-axis of text to print()
    uint8_t text_pixel_margin;  ///< Margin for each text pixel

    bool wrap{false};  // text_wrap ///< If set, 'wrap' text at right edge of display
    bool _is_inited{false};
  };
}  // namespace pixeler
