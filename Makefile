ROOT = $(PWD)
BMOS_ROOT = $(ROOT)

export ROOT
export BMOS_ROOT

PRODUCTS += proto
PRODUCTS += proto-boot
PRODUCTS += proto-pico
PRODUCTS += esptemp

PRODUCTS += proto-boot-avr

%-product:
	$(MAKE) -C products/$*

%-product-clean:
	$(MAKE) -C products/$* clean

all: $(addsuffix -product, $(PRODUCTS))

clean: $(addsuffix -product-clean, $(PRODUCTS))

indent:
	./tools/indent
