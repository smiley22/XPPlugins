TOPTARGETS := all clean

SUBDIRS := Util CycleQuickLooks BetterMouseYoke

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)