#!/usr/bin/python3
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

import serial
import struct
import sys
import time
import getopt

MAX_INIT = 5

CMD_GET = 0x00
CMD_GET_ID = 0x02
CMD_READ = 0x11
CMD_GO = 0x21
CMD_WRITE = 0x31
CMD_ERASE = 0x43
CMD_ERASE_EXTENDED = 0x44
BLOCK_SIZE = 256

INIT = b'\x7f'
ACK = b'\x79'
NACK = b'\x1f'

CMD_ERASE_MASS = 0
CMD_ERASE_MASS_BANK_1 = 1
CMD_ERASE_MASS_BANK_2 = 2


def calc_xor(data):
    xor = 0
    for i in data:
        xor ^= i
    return xor


def calc_comp(data):
    return 0xff ^ data


def print_im(s):
    sys.stderr.write(s)
    sys.stderr.flush()


def dump(name, data):
    sys.stderr.write(name)
    count = len(data)
    for i in range(0, count, 16):
        left = count - i
        if left > 16:
            left = 16
        for j in range(0, left):
            sys.stderr.write("%02x " % data[i + j])
        sys.stderr.write("\n")


def compare(a, b, quiet=True):
    lena = len(a)
    lenb = len(b)
    if lena != lenb:
        if not quiet:
            sys.stderr.write("lengths differ %d != %d" % (lena, lenb))
        return -1
    for i in range(0, lena):
        if a[i] != b[i]:
            if not quiet:
                sys.stderr.write("differ at byte %d" % i)
            return i
    return 0


class stm32_ser:
    def __init__(self, port):
        self.ser = serial.Serial(port, 115200,
                                 timeout=2,
                                 parity=serial.PARITY_EVEN)

    def bootmode(self, on):
        self.ser.setRTS(not on)

    def reset(self, on):
        self.ser.setDTR(not on)

    def read(self, count):
        return self.ser.read(count)

    def write(self, data):
        self.ser.write(data)

    def set_timeout(self, to):
        to = self.ser.timeout
        self.ser.timeout = to

    def get_ack(self):
        data = self.read(1)
        if not data:
            raise Exception("Timeout")
        if data == ACK:
            return
        elif data == NACK:
            raise Exception("Error, got NACK")
        else:
            raise Exception("Error, got %02x response" % data)

    def get_resp(self):
        r = self.read(1)
        count = r[0] + 1
        data = self.read(count)
        self.get_ack()

        return data

    def send_init(self):
        count = 0
        to = self.set_timeout(1)
        while True:
            self.write(INIT)
            data = self.read(1024)
            if data and data[-1:] == ACK:
                break
            else:
                count += 1
                if count > MAX_INIT:
                    raise Exception("Maximum retries reached")
                continue

        self.set_timeout(to)

    def send_cmd(self, cmd):
        comp = calc_comp(cmd)
        cmd_data = bytes((cmd, comp))
        self.write(cmd_data)
        self.get_ack()

    def cmd_get(self):
        self.send_cmd(CMD_GET)
        return self.get_resp()

    def cmd_get_id(self):
        self.send_cmd(CMD_GET_ID)
        return self.get_resp()

    def write_data(self, data):
        xor = calc_xor(data)
        data += bytes((xor,))
        self.write(data)
        self.get_ack()

    def cmd_go(self, addr):
        self.send_cmd(CMD_GO)

        data = struct.pack(">I", addr)
        self.write_data(data)

    def send_addr(self, addr):
        data = struct.pack(">I", addr)
        self.write_data(data)

    def cmd_read_block(self, addr, size):
        self.send_cmd(CMD_READ)
        self.send_addr(addr)
        self.send_cmd(size - 1)

        return self.read(size)

    def cmd_read(self, addr, size, progress=False):
        data = b''
        for i in range(0, size, BLOCK_SIZE):
            if progress:
                print_im(".")
            left = size - i
            if left > BLOCK_SIZE:
                left = BLOCK_SIZE
            data += self.cmd_read_block(addr, left)
            addr += BLOCK_SIZE
        if progress:
            print_im("\n")
        return data

    def cmd_write_block(self, addr, data):
        self.send_cmd(CMD_WRITE)

        self.send_addr(addr)

        size = len(data)

        wdata = bytes((size - 1,))
        wdata += data
        self.write_data(wdata)

    def cmd_write(self, addr, data, progress=False):
        size = len(data)
        for i in range(0, size, BLOCK_SIZE):
            if progress:
                print_im(".")
            left = size - i
            if left > BLOCK_SIZE:
                left = BLOCK_SIZE
            self.cmd_write_block(addr, data[i:i+left])
            addr += BLOCK_SIZE
        if progress:
            print_im("\n")

    def cmd_erase_ext_special(self, cmd):
        self.send_cmd(CMD_ERASE_EXTENDED)

        if cmd == CMD_ERASE_MASS:
            dcmd = 0xffff
        elif cmd == CMD_ERASE_MASS_BANK_1:
            dcmd = 0xfffe
        elif cmd == CMD_ERASE_MASS_BANK_2:
            dcmd = 0xfffd

        wdata = struct.pack(">H", dcmd)
        xor = calc_xor(wdata)
        wdata += bytes((xor,))
        self.write(wdata)
        for i in range(0, 8):
            try:
                self.get_ack()
            except:
                pass

    def cmd_erase_ext(self, page_list):
        self.send_cmd(CMD_ERASE_EXTENDED)

        size = len(page_list)

        wdata = struct.pack(">H", size - 1)

        for i in range(0, size):
            wdata += struct.pack(">H", page_list[i])

        self.write_data(wdata)


def usage(name):
    sys.stderr.write("syntax: %s [-b <base address>] "
                     "[-e <erase count>] <serdev> <image>\n" % name)
    sys.exit(1)


def main():
    progname = sys.argv[0]

    addr = 0x8000000
    n_erase = 0

    try:
        opts, args = getopt.getopt(sys.argv[1:], "a:e:")
    except getopt.GetoptError as err:
        print(str(err))
        usage(progname)
    for o, a in opts:
        if o == "-a":
            addr = int(a, 0)
        elif o == '-e':
            if a == 'all':
                n_erase = -1
            else:
                n_erase = int(a)

    if len(args) < 2:
        usage(progname)

    dev = args[0]
    imfile = args[1]

    s = stm32_ser(dev)

    s.bootmode(True)

    s.reset(True)
    time.sleep(.2)
    s.reset(False)
    time.sleep(.2)

    s.send_init()

    get_res = s.cmd_get()
    erase_cmd = get_res[7]
    if erase_cmd != CMD_ERASE_EXTENDED:
        sys.stderr.write("FIXME: Only extended erase supported\n")
        sys.exit(1)

    ident = s.cmd_get_id()
    if len(ident) != 2:
        dump("invalid chip identifier", ident)
    else:
        sys.stderr.write("chip identifier %02x:%02x\n" %
                         (ident[0], ident[1]))

    data = open(imfile, 'rb').read()

    if n_erase > 0:
        sys.stderr.write("Erasing %d block(s)\n" % n_erase)
        s.cmd_erase_ext(range(0, n_erase))
    elif n_erase == -1:
        sys.stderr.write("Mass Erase\n")
        s.cmd_erase_ext_special(CMD_ERASE_MASS)

    sys.stderr.write("Writing\n")
    s.cmd_write(addr, data, True)

    sys.stderr.write("Read back\n")
    rdata = s.cmd_read(addr, len(data), True)

    sys.stderr.write("Compare\n")
    err = compare(data, rdata, False)
    if err == 0:
        sys.stderr.write("OK\n")

    sys.stderr.write("Starting\n")
    s.cmd_go(addr)

    s.bootmode(False)


if __name__ == '__main__':
    try:
        main()
    except Exception as err:
        sys.stderr.write(str(err))
