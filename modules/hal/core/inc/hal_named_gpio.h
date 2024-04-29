#ifndef HAL_NAMED_GPIO_H
#define HAL_NAMED_GPIO_H

typedef struct _named_gpio_t {
  const char *name;
  gpio_handle_t gpio;
  unsigned char type;
  unsigned char val;
} named_gpio_t;

void named_gpio_init(const named_gpio_t *named_gpio, unsigned int count);

#endif
