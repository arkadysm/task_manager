CONFIG = Debug
BUILDDIR = build/$(CONFIG)/obj

.PHONY: all generate-cmake-files

all: generate-cmake-files
	@cmake --build "$(BUILDDIR)" --config $(CONFIG)

$(filter-out all,$(MAKECMDGOALS)): generate-cmake-files
	@cmake --build "$(BUILDDIR)" --config $(CONFIG) -- $@

generate-cmake-files:
	@cmake -DCMAKE_BUILD_TYPE:STRING=$(CONFIG) -S "$(CURDIR)" -B "$(BUILDDIR)"
