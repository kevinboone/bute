VERSION := 0.1a
NAME    := bute
TARGET  := $(NAME)

# --gc-sections removes unusueds (sections) functions
LDFLAGS := --gc-sections 

# --ffunction-sections puts each function in its own section, so
#   the linker can easily remove unusued functions
CFLAGS  := -O3 -Wall -fno-builtin -ffunction-sections -fdata-sections 
SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS    := $(OBJECTS:.o=.deps)

all: $(TARGET) 

uname_m := $(shell uname -m)
ifeq ($(uname_m),x86_64)
  CRT := cnolib_amd64
else
ifeq ($(uname_m),armv7l)
  CRT := cnolib_arm
else
  $(error Unsupported architecture)
endif
endif

build/$(CRT).o: src/$(CRT).S
	@mkdir -p build/
	$(AS) -o build/$(CRT).o -c src/$(CRT).S

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -DNAME=\"$(NAME)\" -MD -MF $(@:.o=.deps) -c -o $@ $< 

$(TARGET): build/$(CRT).o $(OBJECTS) 
	$(LD) $(LDFLAGS) -s -o $(TARGET) build/$(CRT).o $(OBJECTS) 

clean:
	rm -rf build $(TARGET) 

-include $(DEPS)

.PHONY: clean


