# SMB360 LibXenon bootstrap build.
# Intentionally uses an explicit source manifest: host-only/native-upstream
# adapters are NOT pulled into the Xenon link unless deliberately added.
.SUFFIXES:
ifeq ($(strip $(DEVKITXENON)),)
$(error "Please set DEVKITXENON in your environment")
endif
include $(DEVKITXENON)/rules

TARGET := smb360
BUILD := build-xenon
INCLUDES := include
NATHSOU_INCLUDE :=

# Minimal, independently linkable Xbox bootstrap.
XENON_CPP_SOURCES := \
  src/core/game.cpp \
  src/core/input_mapper.cpp \
  src/core/sha1.cpp \
  src/core/smb1_identity.cpp \
  src/core/ines.cpp \
  src/core/smb1_rom.cpp \
  src/core/state_hash.cpp \
  src/integration/nathsou_core.cpp \
  src/platform/xbox360/gameplay.cpp \
  src/platform/xbox360/input_adapter.cpp \
  src/platform/xbox360/runtime.cpp \
  src/platform/xbox360/audio.cpp \
  src/platform/xbox360/diagnostic.cpp \
  src/platform/xbox360/video.cpp \
  src/platform/xbox360/storage.cpp \
  src/platform/xbox360/telemetry.cpp \
  src/platform/xbox360/main.cpp

CFLAGS := -g -O2 -Wall -Wextra $(MACHDEP) $(INCLUDE)
CXXFLAGS := $(CFLAGS) -std=c++11
LDFLAGS := -g $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(notdir $@).map
LIBS := -lxenon -lm -lfat
LIBDIRS :=

# Optional real gameplay core. NATHSOU_ROOT may be an absolute CI checkout or
# a project-relative staged checkout. Keep its absolute include path intact.
ifneq ($(strip $(NATHSOU_ROOT)),)
NATHSOU_LIB := $(NATHSOU_ROOT)/codegen/lib
NATHSOU_C_SOURCES := \
  $(NATHSOU_LIB)/instructions.c \
  $(NATHSOU_LIB)/code.c \
  $(NATHSOU_LIB)/data.c \
  $(NATHSOU_LIB)/cpu.c \
  $(NATHSOU_LIB)/ppu.c \
  $(NATHSOU_LIB)/apu.c \
  $(NATHSOU_LIB)/state.c \
  $(NATHSOU_LIB)/common.c
XENON_CPP_SOURCES += src/integration/native_nathsou_core.cpp
NATHSOU_INCLUDE := -I$(NATHSOU_LIB)
CXXFLAGS += -DSMB360_WITH_NATHSOU_CORE=1
endif

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(sort $(dir $(addprefix $(CURDIR)/,$(XENON_CPP_SOURCES))) $(dir $(NATHSOU_C_SOURCES)))
export DEPSDIR := $(CURDIR)/$(BUILD)
CPPFILES := $(notdir $(XENON_CPP_SOURCES))
CFILES := $(notdir $(NATHSOU_C_SOURCES))
export LD := $(CXX)
export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(NATHSOU_INCLUDE) -I$(CURDIR)/$(BUILD) -I$(LIBXENON_INC)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBXENON_LIB)
.PHONY: $(BUILD) clean
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/platform-xenon.mk
clean:
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).elf32 $(OUTPUT).elf.map
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).elf32: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
