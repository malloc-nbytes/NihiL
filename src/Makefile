TARGET := main
TEST_TARGET := parser_test

OCAMLOPT ?= ocamlopt
OCAMLDEP ?= ocamldep
OCAMLFLAGS ?= -O2

SOURCES := err.ml utils.ml io.ml kwds.ml location.ml token.ml expr.ml stmt.ml visitor.ml type.ml semantic_analysis.ml parser.ml lexer.ml main.ml
MODULES := $(basename $(SOURCES))
CMXS := $(addsuffix .cmx,$(MODULES))
CMIS := $(addsuffix .cmi,$(MODULES))
OBJECTS := $(addsuffix .o,$(MODULES))
DEPENDENCIES := .depend

.DEFAULT_GOAL := all

.PHONY: all clean rebuild test

all: $(TARGET)

$(TARGET): $(CMXS)
	$(OCAMLOPT) $(OCAMLFLAGS) -o $@ $^

$(TEST_TARGET): $(filter-out main.cmx,$(CMXS)) parser_test.cmx
	$(OCAMLOPT) $(OCAMLFLAGS) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

parser_test.cmx: parser.cmx lexer.cmx

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $<

$(DEPENDENCIES): $(SOURCES)
	$(OCAMLDEP) -native $(SOURCES) > $@

ifeq ($(filter clean rebuild,$(MAKECMDGOALS)),)
-include $(DEPENDENCIES)
endif

clean:
	$(RM) $(TARGET) $(TEST_TARGET) $(CMXS) $(CMIS) $(OBJECTS) parser_test.cmx parser_test.cmi parser_test.o $(DEPENDENCIES)

rebuild: clean
	$(MAKE) all
