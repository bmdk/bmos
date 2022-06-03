#ifndef STM32_ETH_PHY_H
#define STM32_ETH_PHY_H

int phy_read(unsigned int phy, unsigned int reg);
void phy_write(unsigned int phy, unsigned int reg, unsigned int val);

#ifdef CONFIG_ETH_PHY_ADDR
#define PHY_ADDR CONFIG_ETH_PHY_ADDR
#else
#define PHY_ADDR 0
#endif

static inline int phy_reset(void)
{
  unsigned int count, reg;

  phy_write(PHY_ADDR, 0, BIT(15)); /* reset phy */

  hal_delay_us(100);
  while (phy_read(PHY_ADDR, 0) & BIT(15))
    ;

  hal_delay_us(100);

  // Restart and enable auto-negotiate
  phy_write(PHY_ADDR, 0, BIT(12) | BIT(9));
  count = 50;
  while ((phy_read(PHY_ADDR, 0) & BIT(9)) && count) {
    hal_delay_us(100);
    count--;
  }
  if (count == 0) {
    xslog(LOG_INFO, "phy read timeout\n");
    return -1;
  }
  count = 5000;
  while (count > 0) {
    reg = phy_read(PHY_ADDR, 31);
    if (reg & BIT(12))
      break;
    hal_delay_us(1000);
    count--;
  }
  if (count == 0) {
    xslog(LOG_INFO, "eth autonegotiate fail");
    return -1;
  }

  xslog(LOG_INFO, "eth 10%s:%s-duplex", (reg & BIT(3)) ? "0": "", (reg & BIT(
                                                                     4)) ? "full" : "half");
  return (reg >> 3) & 0x3;
}

#endif
