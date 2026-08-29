TARGET := main
TEST_TARGET := scope_test
SEMANTIC_ANALYSIS_TEST_TARGET := semantic_analysis_test

OCAMLOPT ?= ocamlopt
OCAMLDEP ?= ocamldep
OCAMLFLAGS ?= -O2

SOURCES := err.ml io.ml type.ml keyword.ml location.ml token.ml lexer.ml expr.ml stmt.ml parser.ml ast_json.ml scope.ml vis.ml resolver.ml main.ml
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

$(TEST_TARGET): $(filter-out main.cmx,$(CMXS)) scope_test.cmx
	$(OCAMLOPT) $(OCAMLFLAGS) -o $@ $^

$(SEMANTIC_ANALYSIS_TEST_TARGET): $(filter-out main.cmx,$(CMXS)) semantic_analysis_test.cmx
	$(OCAMLOPT) $(OCAMLFLAGS) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

scope_test.cmx: scope.cmx

semantic_analysis_test.cmx: semantic_analysis.cmx

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLFLAGS) -c $<

$(DEPENDENCIES): $(SOURCES)
	$(OCAMLDEP) -native $(SOURCES) > $@

ifeq ($(filter clean rebuild,$(MAKECMDGOALS)),)
-include $(DEPENDENCIES)
endif

clean:
	$(RM) $(TARGET) $(TEST_TARGET) $(SEMANTIC_ANALYSIS_TEST_TARGET) $(CMXS) $(CMIS) $(OBJECTS) scope_test.cmx scope_test.cmi scope_test.o semantic_analysis_test.cmx semantic_analysis_test.cmi semantic_analysis_test.o $(DEPENDENCIES)

rebuild: clean
	$(MAKE) all
