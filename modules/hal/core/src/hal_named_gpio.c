#include <hal_gpio.h>
#include <hal_named_gpio.h>
#include <io.h>
#include <shell.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const named_gpio_t *named_gpio;
  unsigned int count;
} named_gpio_reg_t;

static named_gpio_reg_t named_gpio_reg;

static void _ngpio_init()
{
  int i;
  const named_gpio_t *g;
  named_gpio_reg_t *reg = &named_gpio_reg;

  for (i = 0; i < reg->count; i++) {
    g = &reg->named_gpio[i];
    gpio_set(g->gpio, g->val);
    gpio_init(g->gpio, g->type);
  }
}

void named_gpio_init(const named_gpio_t *named_gpio, unsigned int count)
{
  named_gpio_reg_t *reg = &named_gpio_reg;

  reg->named_gpio = named_gpio;
  reg->count = count;

  _ngpio_init();
}

static const named_gpio_t *find_named(const char *name)
{
  int i;
  named_gpio_reg_t *reg = &named_gpio_reg;

  for (i = 0; i < reg->count; i++)
    if (strcmp(name, reg->named_gpio[i].name) == 0)
      return &reg->named_gpio[i];

  return NULL;
}

int cmd_ngpio(int argc, char *argv[])
{
  int i;
  const named_gpio_t *g;
  named_gpio_reg_t *reg = &named_gpio_reg;
  char cmd;

  if (argc < 2)
    cmd = 'i';
  else
    cmd = argv[1][0];

  switch (cmd) {
  case 'i':
    for (i = 0; i < reg->count; i++) {
      g = &reg->named_gpio[i];
      if (g->type == GPIO_INPUT)
        xprintf("%-14s (i): %d\n", g->name, gpio_get(g->gpio));
    }
    break;
  case 'l':
    for (i = 0; i < reg->count; i++) {
      g = &reg->named_gpio[i];
      xprintf("%-14s (%c): %d\n", g->name,
              (g->type == GPIO_INPUT) ? 'i': 'o',
              gpio_get(g->gpio));
    }
    break;
  case 'r':
    if (argc < 3)
      return -1;
    g = find_named(argv[2]);
    if (g)
      xprintf("val: %d\n", gpio_get(g->gpio));
    break;
  case 'w':
    if (argc < 4)
      return -1;
    g = find_named(argv[2]);
    if (g && (g->type == GPIO_OUTPUT))
      gpio_set(g->gpio, atoi(argv[3]));
  }

  return 0;
}

SHELL_CMD(ngpio, cmd_ngpio);
