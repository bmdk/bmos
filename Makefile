ROOT = $(PWD)

export ROOT

PRODUCTS += proto
PRODUCTS += proto-boot

%-product:
	$(MAKE) -C products/$*

%-product-clean:
	$(MAKE) -C products/$* clean

all: $(addsuffix -product, $(PRODUCTS))

clean: $(addsuffix -product-clean, $(PRODUCTS))

indent:
	./tools/indent
