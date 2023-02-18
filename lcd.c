#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "lcd.h"
#include "lcd.pio.h"
#include "font.h"
#include "term.h"

volatile bool rotate = LCD_ROTATE;
volatile int dirty;
int brightness = BRIGHTNESS_DEFAULT;
bool rotated;

void core1_main() {
  // Load the pio program
  lcd_pio_init();
  sleep_ms(100);
  
  // Initialise the LCD
  lcd_pio_set_dc(0);
  lcd_pio_put(0x11); // Cmd: Sleep Out
  lcd_pio_set_dc(0);
  lcd_pio_put(0x29); // Cmd: Display ON

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
        int32_t fgcolor = colors[term_attr_fgcolor(term_attrs[pos])];
        int32_t bgcolor = colors[term_attr_bgcolor(term_attrs[pos])];
        if (term_cursor_visible && x == term_cursor_x && y == term_cursor_y) {
          fgcolor = colors[0];
          bgcolor = colors[15];
        }
        int32_t aacolor = bgcolor;
        if (bgcolor == 0) {
          aacolor = (fgcolor & 0xfefefe) >> 1;
        } else {
          aacolor = ((fgcolor & 0xf8f8f8) >> 3) + ((bgcolor & 0xf8f8f8) >> 3) + ((bgcolor & 0xfcfcfc) >> 2) + ((bgcolor & 0xfefefe) >> 1);
        }
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
