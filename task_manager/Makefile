.PHONY: all generate-cmake-files

CONFIG = Debug
BUILDDIR = build/$(CONFIG)/obj

all: generate-cmake-files
	@cmake --build "$(BUILDDIR)" --config $(CONFIG)

$(filter-out all,$(MAKECMDGOALS)): generate-cmake-files
	@cmake --build "$(BUILDDIR)" --config $(CONFIG) -- $@

generate-cmake-files:
	@cmake -DCMAKE_BUILD_TYPE:STRING=$(CONFIG) -S "$(CURDIR)" -B "$(BUILDDIR)"
