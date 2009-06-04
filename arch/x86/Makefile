# BITS is used as extension for files which are available in a 32 bit
# and a 64 bit version to simplify shared Makefiles.
# e.g.: obj-y += foo_$(BITS).o
export BITS

ifeq ($(CONFIG_X86_32),y)
  BITS := 32
  biarch := $(call cc-option,-m32)
  KBUILD_AFLAGS += $(biarch)
  KBUILD_CFLAGS += $(biarch)

  KBUILD_CFLAGS += $(call cc-option,-mpreferred-stack-boundary=2)

  # insert specific flags
  # @devel: Minix's compiler-generated symbol _EM_WSIZE giving the word size in
  #         bytes. We need this until we completely turn gcc. We use code16gcc
  #         for generating 16-bit code thus _EM_WSIZE is still 4. code16gcc uses
  #         32-bit size and address prefixes.
  KBUILD_CPPFLAGS += -D__CODE32__ -D_EM_WSIZE=4 -D_EM_PSIZE=4 -D_EM_LSIZE=4 

  # include compiler stuffs
  include arch/$(SRCARCH)/Makefile_32.cpu

  KBUILD_CFLAGS += $(cflags-y)
endif

boot := arch/x86/boot

# speed up the build
KBUILD_CFLAGS += -pipe

core-y += arch/$(SRCARCH)/kernel/
core-y += arch/$(SRCARCH)/monitor/
core-y += arch/$(SRCARCH)/boot/

# tied with gcc
__gcc-version = $(shell echo "__GNUC__ __GNUC_MINOR__ __GNUC_PATCHLEVEL__" | gcc -E - | grep -v "\#" | tr ' ' .)
__cc-version = $(__gcc-version) 
__cc-version-str = $(shell echo __VERSION__ | gcc -E - | grep -v "\#")

# gcc include directory
__cc-install-dir = $(dir $(shell $(CC) -print-libgcc-file-name))
__cc-include-dir = $(__cc-install-dir)include
__cc-lib-dir = $(__cc-install-dir)

archclean:
	$(Q)$(MAKE) $(clean)=$(boot)