TARGET := main

OCAMLOPT ?= ocamlopt
OCAMLDEP ?= ocamldep
OCAMLFLAGS ?= -O2

SOURCES := utils.ml io.ml kwds.ml location.ml token.ml grammar.ml lexer.ml main.ml
MODULES := $(basename $(SOURCES))
CMXS := $(addsuffix .cmx,$(MODULES))
CMIS := $(addsuffix .cmi,$(MODULES))
OBJECTS := $(addsuffix .o,$(MODULES))
DEPENDENCIES := .depend

.DEFAULT_GOAL := all

.PHONY: all clean rebuild

all: $(TARGET)

$(TARGET): $(CMXS)
	$(OCAMLOPT) $(OCAMLFLAGS) -o $@ $^

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $<

$(DEPENDENCIES): $(SOURCES)
	$(OCAMLDEP) -native $(SOURCES) > $@

ifeq ($(filter clean rebuild,$(MAKECMDGOALS)),)
-include $(DEPENDENCIES)
endif

clean:
	$(RM) $(TARGET) $(CMXS) $(CMIS) $(OBJECTS) $(DEPENDENCIES)

rebuild: clean
	$(MAKE) all
