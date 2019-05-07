TOPTARGETS := all clean

SUBDIRS := Util CycleQuickLooks BetterMouseYoke ToggleMouseLook A320UE

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)