.SECONDEXPANSION:
%-protocol.c: $(wayland_protocoldir)/$$(notdir $$**).xml \
	      %-server-protocol.h %-client-protocol.h
	wayland-scanner code < $< > $@
%-server-protocol.h: $(wayland_protocoldir)/$$(notdir $$**).xml
	wayland-scanner server-header < $< > $@
%-client-protocol.h: $(wayland_protocoldir)/$$(notdir $$**).xml
	wayland-scanner client-header < $< > $@
