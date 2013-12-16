#.SECONDEXPANSION:
#$(LOCAL_PATH)/%-protocol.c: $(wayland_protocoldir)/$$(notdir $$**).xml \
#	      %-server-protocol.h %-client-protocol.h
#	wayland-scanner code < $< > $@
#$(LOCAL_PATH)/%-server-protocol.h: $(wayland_protocoldir)/$$(notdir $$**).xml
#	wayland-scanner server-header < $< > $@
#$(LOCAL_PATH)/%-client-protocol.h: $(wayland_protocoldir)/$$(notdir $$**).xml
#	wayland-scanner client-header < $< > $@

define __WAYLAND_SCANNER_RULES

$(WAYLAND_SCANNER_DEST)/$(1)-protocol.c: $(2)
	wayland-scanner code < $$< > $$@

$(WAYLAND_SCANNER_DEST)/$(1)-client-protocol.h: $(2)
	wayland-scanner client-header < $$< > $$@

$(WAYLAND_SCANNER_DEST)/$(1)-server-protocol.h: $(2)
	wayland-scanner server-header < $$< > $$@

.PHONY: $(1)-protocol

$(1)-protocol: \
	$(WAYLAND_SCANNER_DEST)/$(1)-protocol.c \
	$(WAYLAND_SCANNER_DEST)/$(1)-client-protocol.h \
	$(WAYLAND_SCANNER_DEST)/$(1)-server-protocol.h

endef

$(foreach __file, $(WAYLAND_SCANNER_SRC), \
	$(eval __base := $(notdir $(basename $(__file)))) \
	$(eval $(call __WAYLAND_SCANNER_RULES,$(__base),$(__file))) \
)
