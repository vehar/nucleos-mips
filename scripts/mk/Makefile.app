##############################################################################
#  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
#
#  This file is part of Nucleos kernel.
#
#  Nucleos kernel is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, version 2 of the License.
##############################################################################

# build applications (mainly servers)
# target's applications
app-y := $(sort $(addprefix $(src)/,$(app-y)))

# rule to create app
define rule_link_app
quiet_cmd_link_app_$(1) = LD      $$@
      cmd_link_app_$(1) = $$(LD) -o $$@ $$(filter $$($(1)-obj-y),$$^) $$(extobj-y) $$($$(notdir $(1))-extobj-y) $$(ld_flags) 

# This handles also the case if host. app. is not in the same directory
# as this Makefiel is.
#
# e.g. progs-y := subdir/appl
#      subdir/appl-obj-y :=  subdir/obj1.o subdir/obj2.o
$(1)-obj-y := $$(filter %.o,$$(addprefix $$(src)/,$$($$(subst $$(src)/,,$(1))-obj-y)))

# add to object list
objs-y += $$($(1)-obj-y)

# add to directory list
subdir-y += $$(filter-out %.o,$$(addprefix $$(src)/,$$($$(subst $$(src)/,,$(1))-obj-y)))

# flags for library
# Note that this is used only on prerequisite side
ld_flags_$(1) := $$(LDFLAGS) $$(ldflags-y) $$(LDFLAGS_$$(notdir $(1)))

# depends on external object but doesn't build it
$(1): $$($(1)-obj-y) $$(extobj-y) $$($$(notdir $(1))-extobj-y) $$(call get_libs,$$(ld_flags_$(1))) FORCE
	$$(call if_changed,link_app_$(1))

# add into target list
targets += $(1)
endef

# generate app-y goals
$(foreach app,$(app-y),$(eval $(call rule_link_app,$(app))))

# Create aout binary from elf32
# param input elf32 file
# param output aout file
define rule_create_aoutbin
# create a header from ELF32 boot
quiet_cmd_create_header_$(2) = GEN     $$@
      cmd_create_header_$(2) = scripts/tools/ehdr2ahdr -c $$($(2)_a_cpu) -f $$($(2)_a_flags) -s $$($(2)_stackheap) -l $$($(2)_a_hdrlen) -i $$< -o $$@

$$(src)/.tmp.$(2).ahdr: scripts/tools/ehdr2ahdr 
$$(src)/.tmp.$(2).ahdr: $$(src)/$(1) FORCE
	$$(call if_changed,create_header_$(2))

targets += $$(src)/.tmp.$(2).ahdr

OBJCOPYFLAGS_$(2) := -O binary
OBJCOPYFLAGS_$(2) += $$(if $$(strip $$($(2)_keepsyms)),--keep-file-symbols,-S)

quiet_cmd_create_rawbin_$(2) = OBJCOPY $$@
      cmd_create_rawbin_$(2) = $$(OBJCOPY) $$(OBJCOPYFLAGS_$(2)) $$< $$@

# create a raw binary from ELF32 boot
$$(src)/.tmp.$(2).rawbin: $$(src)/$(1) FORCE
	$$(call if_changed,create_rawbin_$(2))

targets += $$(src)/.tmp.$(2).rawbin

quiet_cmd_create_aout_$(2) = GEN     $$@
      cmd_create_aout_$(2) = cat $$(src)/.tmp.$(2).ahdr $$(src)/.tmp.$(2).rawbin > $$@

# create aout binary by adding aout header at begin
$$(src)/$(2): $$(src)/.tmp.$(2).ahdr $$(src)/.tmp.$(2).rawbin FORCE
	$$(call if_changed,create_aout_$(2))

targets += $$(src)/$(2)
endef

# Create aout from elf. Input must be in the following 
# format and order: 
#   e2a-y := ELF32,AOUT 
# Note: Binaries are separated by comma.
ifneq ($(strip $(e2a-y)),)
$(foreach e2a,$(e2a-y),$(eval $(call rule_create_aoutbin,$(firstword $(subst $(comma), ,$(e2a))),$(lastword $(subst $(comma), ,$(e2a))))))
endif