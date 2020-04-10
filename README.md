# ESP8266 examples

I'm using https://github.com/pfalcon/esp-open-sdk.

## Assumptions

Makefile's assume the following:

```make
XTENSA_TOOLS_ROOT ?= $(HOME)/x-tools/xtensa-lx106-elf/bin
SDK_BASE	  ?= $(HOME)/x-tools/sdk
```

The `sdk` and `xtensa-lx106-elf` folders are from the `esp-open-sdk`.
