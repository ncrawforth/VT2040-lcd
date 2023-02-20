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
  0xef0000, /* Red */ \
  0x00ef00, /* Green */ \
  0xefef00, /* Yellow */ \
  0x0000ef, /* Blue */ \
  0xef00ef, /* Magenta */ \
  0x00efef, /* Cyan */ \
  0xefefef, /* White */ \
  0x7f7f7f, /* Bright black */ \
  0xff7f7f, /* Bright red */ \
  0x7fff7f, /* Bright green */ \
  0xffff7f, /* Bright yellow */ \
  0x7f7fff, /* Bright blue */ \
  0xff7fff, /* Bright magenta */ \
  0x7fffff, /* Bright cyan */ \
  0xffffff, /* Bright white */

void lcd_init();
void lcd_invalidate();
void lcd_brighten();
void lcd_darken();
void lcd_invert();
void lcd_rotate();
