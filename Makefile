ROOT = $(PWD)

export ROOT

PRODUCTS += proto
PRODUCTS += proto-boot
PRODUCTS += proto-pico
PRODUCTS += esptemp

%-product:
	$(MAKE) -C products/$*

%-product-clean:
	$(MAKE) -C products/$* clean

all: $(addsuffix -product, $(PRODUCTS))

clean: $(addsuffix -product-clean, $(PRODUCTS))

indent:
	./tools/indent
