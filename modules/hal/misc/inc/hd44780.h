#ifndef HD44780_H
#define HD44780_H

typedef void _hd44780_write_byte(unsigned int b);
typedef void _hd44780_delay(void);

typedef struct {
  _hd44780_write_byte *write_byte;
  _hd44780_delay *delay;
} hd44780_data_t;

void hd44780_init(hd44780_data_t *d);
void hd44780_write_str(hd44780_data_t *d, unsigned int addr, const char *str);
void hd44780_clear(hd44780_data_t *d);
void hd44780_home(hd44780_data_t *d);
void hd44780_disp(hd44780_data_t *d, int en, int cursor, int blink);
void hd44780_shift(hd44780_data_t *d, int cursor, int right);

#endif
