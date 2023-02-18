// LCD is rotated 180 degrees
#define LCD_ROTATE false

// Number of backlight brightness steps
#define BRIGHTNESS_MAX 10

// Default backlight brightness
#define BRIGHTNESS_DEFAULT 2

// Select which GPIO pins to use
#define LCD_PIN_CS 8
#define LCD_PIN_RST 9
#define LCD_PIN_DC 10
#define LCD_PIN_MOSI 11
#define LCD_PIN_CLK 12
#define LCD_PIN_LED 13

// Select which PIO and state machine to use
#define LCD_PIO pio0
#define LCD_PIO_SM 0

// Clock divisor to use when sending commands (lower = faster)
// 6.25 is within the datasheet spec
#define LCD_PIO_CLKDIV_CMD 6.25f

// Clock divisor to use when sending pixel data (lower = faster)
// 1.5 is outside spec but works
#define LCD_PIO_CLKDIV_PIXELS 1.5f

// Colour palette
#define COLORS \
  0x000000, /* Black */ \
  0x9f5676, /* Red */ \
  0x769542, /* Green */ \
  0x9f7642, /* Yellow */ \
  0x4c76a9, /* Blue */ \
  0x7656a9, /* Magenta */ \
  0x4c9576, /* Cyan */ \
  0xdfdfdf, /* White */ \
  0x3f3f3f, /* Bright black */ \
  0xbf7696, /* Bright red */ \
  0x96b562, /* Bright green */ \
  0xbf9662, /* Bright yellow */ \
  0x6c96c9, /* Bright blue */ \
  0x9676c9, /* Bright magenta */ \
  0x6cb596, /* Bright cyan */ \
  0xffffff, /* Bright white */

void lcd_init();
void lcd_invalidate();
void lcd_brighten();
void lcd_darken();
void lcd_invert();
void lcd_rotate();
