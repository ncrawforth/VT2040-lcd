#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "lcd.h"
#include "lcd.pio.h"
#include "font.h"
#include "term.h"

int32_t colors[] = {LCD_COLORS};
volatile bool rotate = LCD_ROTATE;
volatile int dirty;
int brightness = BRIGHTNESS_DEFAULT;
bool rotated;

void lcd_pio_init() {
  // Install the PIO program
  uint offset = pio_add_program(LCD_PIO, &lcd_program);
  pio_sm_config c = lcd_program_get_default_config(offset);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&c, false, true, 8);
  // Initialise the MOSI pin
  pio_gpio_init(LCD_PIO, LCD_PIN_MOSI);
  pio_sm_set_consecutive_pindirs(LCD_PIO, LCD_PIO_SM, LCD_PIN_MOSI, 1, true);
  sm_config_set_out_pins(&c, LCD_PIN_MOSI, 1);
  // Initialise the CLK pin
  pio_gpio_init(LCD_PIO, LCD_PIN_CLK);
  pio_sm_set_consecutive_pindirs(LCD_PIO, LCD_PIO_SM, LCD_PIN_CLK, 1, true);
  sm_config_set_sideset_pins(&c, LCD_PIN_CLK);
  // Start the PIO program
  pio_sm_init(LCD_PIO, LCD_PIO_SM, offset, &c);
  pio_sm_set_enabled(LCD_PIO, LCD_PIO_SM, true);
}

void lcd_pio_set_dc(bool dc) {
  // Wait until the PIO stalls (meaning it has finished sending)
  uint32_t sm_stall_mask = 1u << (LCD_PIO_SM + PIO_FDEBUG_TXSTALL_LSB);
  LCD_PIO->fdebug = sm_stall_mask;
  while (!(LCD_PIO->fdebug & sm_stall_mask)) tight_loop_contents();
  // Set the Data/Cmd pin (0 = Cmd, 1 = Data)
  gpio_put(LCD_PIN_DC, dc);
}

void lcd_pio_put(uint8_t byte) {
  // Wait until there is space in the PIO's TX FIFO
  while (pio_sm_is_tx_fifo_full(LCD_PIO, LCD_PIO_SM)) tight_loop_contents();
  // Add 1 byte of data to the FIFO
  *(volatile uint8_t*)&LCD_PIO->txf[LCD_PIO_SM] = byte;
}

void core1_main() {
  // Load the pio program
  lcd_pio_init();

  // Hold the CS pin low
  gpio_init(LCD_PIN_CS);
  gpio_set_dir(LCD_PIN_CS, GPIO_OUT);
  gpio_put(LCD_PIN_CS, 0);

  // Hold the RST pin high
  gpio_init(LCD_PIN_RST);
  gpio_set_dir(LCD_PIN_RST, GPIO_OUT);
  gpio_put(LCD_PIN_RST, 1);

  // Initialise the DC pin
  gpio_init(LCD_PIN_DC);
  gpio_set_dir(LCD_PIN_DC, GPIO_OUT);

  // Wait for the LCD to settle
  sleep_ms(100);
  
  // Initialise the LCD
  lcd_pio_set_dc(0);
  pio_sm_set_clkdiv(LCD_PIO, LCD_PIO_SM, LCD_PIO_CLKDIV_CMD);
  lcd_pio_put(0x11); // Cmd: Sleep Out
  lcd_pio_put(0x29); // Cmd: Display ON

  // Fix the LCD gamma curve
  int gamma[] = {LCD_GAMMA};
  lcd_pio_set_dc(0);
  lcd_pio_put(0xe0); // Cmd: Positive Gamma Control
  lcd_pio_set_dc(1);
  for (int i = 0; i < 15; i++) lcd_pio_put(gamma[i]);
  lcd_pio_set_dc(0);
  lcd_pio_put(0xe1); // Cmd: Negative Gamma Control
  lcd_pio_set_dc(1);
  for (int i = 0; i < 15; i++) lcd_pio_put(0xff - gamma[14 - i]);

  // Clear the LCD
  lcd_pio_set_dc(0);
  lcd_pio_put(0x2c); // Cmd: Memory Write
  lcd_pio_set_dc(1);
  for (int i = 0; i < (320 * 480 * 3); i++) {
    lcd_pio_put(0); // Data: 0
  }

  // Wait a moment for screen to refresh
  sleep_ms(100);

  // Turn on the LCD backlight
  gpio_set_function(LCD_PIN_LED, GPIO_FUNC_PWM);
  pwm_set_clkdiv(pwm_gpio_to_slice_num(LCD_PIN_LED), 10.f);
  pwm_set_wrap(pwm_gpio_to_slice_num(LCD_PIN_LED), BRIGHTNESS_MAX * BRIGHTNESS_MAX);
  pwm_set_gpio_level(LCD_PIN_LED, brightness * brightness);
  pwm_set_enabled(pwm_gpio_to_slice_num(LCD_PIN_LED), 1);

  while (1) {
    while (dirty == 0) sleep_ms(10);
    dirty--;

    if (rotate) {
      rotate = false;
      lcd_pio_set_dc(0);
      pio_sm_set_clkdiv(LCD_PIO, LCD_PIO_SM, LCD_PIO_CLKDIV_CMD);
      lcd_pio_put(0x36); // Cmd: Memory Access Control
      lcd_pio_set_dc(1);
      if (rotated) {
        lcd_pio_put(0x0);
        rotated = 0;
      } else {
        lcd_pio_put(0xc0);
        rotated = 1;
      }
    }

    unsigned int pos = 0;
    for (unsigned int y = 0; y < TERM_HEIGHT; y++) {
      unsigned int top = y * 13 + 4;
      unsigned int bottom = top + 12;
      
      lcd_pio_set_dc(0);
      pio_sm_set_clkdiv(LCD_PIO, LCD_PIO_SM, LCD_PIO_CLKDIV_CMD);
      lcd_pio_put(0x2a); // Cmd: Column Address Set
      
      lcd_pio_set_dc(1);
      lcd_pio_put(top >> 8); // Data
      lcd_pio_put(top);
      lcd_pio_put(bottom >> 8);
      lcd_pio_put(bottom);
      
      lcd_pio_set_dc(0);
      lcd_pio_put(0x2c); // Cmd: Memory Write
      
      lcd_pio_set_dc(1);
      pio_sm_set_clkdiv(LCD_PIO, LCD_PIO_SM, LCD_PIO_CLKDIV_PIXELS);
      for (unsigned int x = 0; x < TERM_WIDTH; x++) {
        int attr = term_attrs[pos];
        int32_t fgcolor = term_attr_fgcolor(attr);
        int32_t bgcolor = term_attr_bgcolor(attr);
        fgcolor = colors[fgcolor];
        bgcolor = colors[bgcolor];
	//bgcolor = (((x % 64) * 255) / 63) * 0x10101;
        if (term_cursor_visible && x == term_cursor_x && y == term_cursor_y) {
          fgcolor = colors[0];
          bgcolor = colors[7];
        }
        int32_t aacolor = ((bgcolor & 0xfefefe) >> 1) + ((fgcolor & 0xfefefe) >> 1);
        int32_t underline = term_attr_underline(term_attrs[pos]) ? 0b1100 : 0;
        int32_t *c = &font[term_chars[pos] * 6];
        for (int32_t *w = c; w < c + 6; w++) {
          int32_t bitmap = *w | underline;
          for (int32_t i = 0x3000000; i > 0; i >>= 2) {
            int32_t p = bitmap & i;
            if (p == i) {
              lcd_pio_put(fgcolor);
              lcd_pio_put(fgcolor >> 8);
              lcd_pio_put(fgcolor >> 16);
            } else if (p == 0) {
              lcd_pio_put(bgcolor);
              lcd_pio_put(bgcolor >> 8);
              lcd_pio_put(bgcolor >> 16);
            } else {
              lcd_pio_put(aacolor);
              lcd_pio_put(aacolor >> 8);
              lcd_pio_put(aacolor >> 16);
            }
          }
        }
        pos++;
      }
    }
  }
}

void lcd_init() {
  // Start the LCD driver on CPU core 1
  multicore_launch_core1(core1_main);
}

void lcd_invalidate() {
  dirty = 2;
}

void lcd_brighten() {
  if (brightness < BRIGHTNESS_MAX) brightness++;
  pwm_set_gpio_level(LCD_PIN_LED, brightness * brightness);
}

void lcd_darken() {
  if (brightness > 1) brightness--;
  pwm_set_gpio_level(LCD_PIN_LED, brightness * brightness);
}

void lcd_invert() {
  int32_t tmp = colors[0];
  colors[0] = colors[7];
  colors[7] = tmp;
  tmp = colors[8];
  colors[8] = colors[15];
  colors[15] = tmp;
  for (int i = 0; i < 8; i++) {
    tmp = colors[i];
    colors[i] = colors[i + 8];
    colors[i + 8] = tmp;
  }
  lcd_invalidate();
}

void lcd_rotate() {
  rotate = true;
  lcd_invalidate();
}
