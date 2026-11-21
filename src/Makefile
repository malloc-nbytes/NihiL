# NOTE: run premake.py to generate config.mk
-include config.mk

DEBUG_FLAGS := -O0 -g3

SRCDIR := .
OBJDIR := obj
BINDIR := bin
INCLUDE := include

TARGET := NihiL

# Find all .c files
SOURCES := $(shell find $(SRCDIR) -type f -name '*.c')
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

PREFIX ?= /usr/local
DESTDIR ?=

# Default target
.PHONY: all
all: $(BINDIR)/$(TARGET)

# Create directories
$(OBJDIR) $(BINDIR):
	mkdir -p $@

# Compile .c to .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link everything
$(BINDIR)/$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.PHONY: debug
debug: CFLAGS := $(filter-out -O2,$(CFLAGS)) $(DEBUG_FLAGS)
debug: all

.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -m 755 "$(BINDIR)/$(TARGET)" "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

.PHONY: uninstall
uninstall:
	-rm -f "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: distclean
distclean:
	rm -f config.mk
	rm -f include/config.h
	rm -f ../*.html

.PHONY: run
run: all
	./$(BINDIR)/$(TARGET)

.PHONY: run-debug
run-debug: debug
	./$(BINDIR)/$(TARGET)

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all         - Build optimized release version (default)"
	@echo "  debug       - Build with -O0 and full debug symbols (-g3)"
	@echo "  run         - Build and run release version"
	@echo "  run-debug   - Build and run debug version"
	@echo "  clean       - Remove build artifacts"
	@echo "  distclean   - Remove premake artifacts"
	@echo "  help        - Show this help"

# Prevent deleting intermediate files
.PRECIOUS: $(OBJDIR)/%.o
