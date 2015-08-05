#
# Master Makefile (for convienience)
#
# Calls CMake to build in $(BUILD) and install to $(PREFIX). Known
# working versions of CMake are 2.6.* (not 2.4)
#

### global settings
SRC_DIR:=$(shell pwd)
PREFIX:=$(SRC_DIR)/root
BUILD:=$(SRC_DIR)/build

## which setting to build?
BUILD_TYPE:=RELWITHDEBINFO
VERBOSE:=

FLAGS:= $(EXTRA_FLAGS)
OTHER:=

### sword settings
PROJECT_SRC:=$(SRC_DIR)
PROJECT_BUILD:=$(abspath $(BUILD))
PROJECT_DONE:=$(PROJECT_BUILD)/done
PROJECT_NEED:= \
	$(wildcard $(PROJECT_SRC)/*/CMakeLists.txt) \
	$(wildcard $(PROJECT_SRC)/*/extrasources.make) \


PROJECT_DEP:= \
	$(wildcard $(PROJECT_SRC)/*/*.h) \
	$(wildcard $(PROJECT_SRC)/*/*.hpp) \
	$(wildcard $(PROJECT_SRC)/*/*.cc) \
	$(wildcard $(PROJECT_SRC)/*/*.cpp) \
	$(wildcard $(PROJECT_SRC)/*/*/*.cpp) \
	$(wildcard $(PROJECT_SRC)/*/*/*.h) \
	$(wildcard $(PROJECT_SRC)/*/*/*/*.cpp) \
	$(wildcard $(PROJECT_SRC)/*/*/*/*.h)

all: $(PROJECT_DONE)

class:
	(cd $(SRC_DIR)/templates; ant createclass)

test :
	(cd $(SRC_DIR)/templates; ant createtestclass)

gcheck: GUI=--gui
gcheck: check
check: $(PROJECT_DONE)
	@ cd $(SRC_DIR)/tests && \
	  $(RUN) $(PROJECT_BUILD)/tests/immortal-tester $(GUI) $(TEST)

.PHONY: doc
doc:
	doxygen
	ln -f -s ./html/index.html ./doc/doxygen.html

# $1: Target
#
define cmake_build
$(value $(1)_BUILD)/Makefile: $(value $(1)_NEED)
	mkdir  -p $(value $(1)_BUILD)
	cd $(value $(1)_BUILD) \
	&& set CXXFLAGS "$(FLAGS)" && set CFLAGS "$(FLAGS)" $(OTHER) \
	&& cmake \
		-DCMAKE_INSTALL_PREFIX=$(abspath $(PREFIX)) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		$(value $(1)_SRC)

$(value $(1)_DONE): $(value $(1)_BUILD)/Makefile $(value $(1)_DEP)
	+cd $(value $(1)_BUILD) && $(MAKE) VERBOSE=$(VERBOSE)
	touch $(value $(1)_DONE)

rm-$(1):
	rm -f  $(value $(1)_DONE)
	rm -rf $(value $(1)_BUILD)

clean:  rm-$(1)
.PHONY: rm-$(1)
endef

# and... ACTION...
$(eval $(call cmake_build,PROJECT))
