# Copyright (c) 2019-2021 Brian Thomas Murphy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

export BUILD_DIR = build
export ROOT ?= $(shell pwd)/../..

PROG += f411bp
PROG += f401bp
PROG += f407deb
PROG += f407debm
PROG += h743wa
PROG += h743nucleo
PROG += h745nucleo
PROG += h735dk
PROG += l4r
PROG += f767
PROG += f429
PROG += f469d
PROG += f746
PROG += l432
PROG += l452
PROG += l496
PROG += g474n
PROG += g431kbn
PROG += wb55n

PROG_TRG = $(addprefix board-, $(PROG))

all: $(PROG_TRG)

board-%:
	$(MAKE) -f Makefile.board PROG=$*

clean:
	rm -fr $(BUILD_DIR)
