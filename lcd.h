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
  0xf70000, /* Red */ \
  0x00f700, /* Green */ \
  0xf7f700, /* Yellow */ \
  0x0000f7, /* Blue */ \
  0xf700f7, /* Magenta */ \
  0x00f7f7, /* Cyan */ \
  0xf7f7f7, /* White */ \
  0xbfbfbf, /* Bright black */ \
  0xffbfbf, /* Bright red */ \
  0xbfffbf, /* Bright green */ \
  0xffffbf, /* Bright yellow */ \
  0xbfbfff, /* Bright blue */ \
  0xffbfff, /* Bright magenta */ \
  0xbfffff, /* Bright cyan */ \
  0xffffff, /* Bright white */

void lcd_init();
void lcd_invalidate();
void lcd_brighten();
void lcd_darken();
void lcd_invert();
void lcd_rotate();
