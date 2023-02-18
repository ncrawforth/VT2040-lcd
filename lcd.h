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

void lcd_init();
void lcd_invalidate();
void lcd_brighten();
void lcd_darken();
void lcd_invert();
void lcd_rotate();
