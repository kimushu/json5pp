TARGET = json5pp-test
CXX = g++
CXXFLAGS = -I.. -std=c++11 -s -O2
TIMEOUT = 5

ifeq ($(OS),Windows_NT)
SUFFIX = exe
else
SUFFIX = run
endif

Q ?= @
DUT = ../json5pp.hpp
CASES := $(sort $(wildcard input/*.json) $(wildcard input/*.cpp))
LOG = result.json

.PHONY: all
all: test

.PHONY: test
test: $(LOG)

.PHONY: clean
clean:
	$(Q)rm -rf output/*.$(SUFFIX)
	$(Q)rm -f $(LOG)

BUILD = $(CXX) -o $@ $(CXXFLAGS) -DTIMEOUT=$(TIMEOUT) $(filter %.cpp,$^)
ESC := $(shell echo -ne "\e")
COLORIZE = sed -e "s/\[PASS\]/$(ESC)[32m&$(ESC)[0m/" -e "s/\[FAIL\]/$(ESC)[31m&$(ESC)[0m/"

output/fail.$(SUFFIX): fail.cpp $(DUT)
	@echo "// Compiling $@ ..."
	$(Q)$(BUILD) -DPARSE_METHOD=parse

output/fail5.$(SUFFIX): fail.cpp $(DUT)
	@echo "// Compiling $@ ..."
	$(Q)$(BUILD) -DPARSE_METHOD=parse5

output/pass_%.$(SUFFIX): input/pass_%.hpp pass.cpp $(DUT)
	@echo "// Compiling $@ ..."
	$(Q)$(BUILD) -include $< -DPARSE_METHOD=parse

output/pass5_%.$(SUFFIX): input/pass5_%.hpp pass.cpp $(DUT)
	@echo "// Compiling $@ ..."
	$(Q)$(BUILD) -include $< -DPARSE_METHOD=parse5

output/exec_%.$(SUFFIX): input/exec_%.cpp exec.cpp $(DUT)
	@echo "// Compiling $@ ..."
	$(Q)$(BUILD)

$(LOG): output/fail.$(SUFFIX) output/fail5.$(SUFFIX) $(filter %.json,$(CASES)) \
	$(patsubst input/%.json,output/%.$(SUFFIX),$(filter input/pass%,$(CASES))) \
	$(patsubst input/%.cpp,output/%.$(SUFFIX),$(filter input/%.cpp,$(CASES)))
	$(Q)echo "{" | tee $@
	$(Q)\
	for x in $(filter input/fail_%.json,$^); do \
		./output/fail.$(SUFFIX) $$x | tee -a $@ | $(COLORIZE); \
	done; \
	for x in $(filter input/fail5_%.json,$^); do \
		./output/fail5.$(SUFFIX) $$x | tee -a $@ | $(COLORIZE); \
	done; \
	for x in $(filter output/%.$(SUFFIX),$^); do \
		n=$${x##*/}; j=$${n%.*}.json; c=compare/$$j; \
		unset o; test -f $$c && o=output/$$j; \
		./$$x input/$$j $$o | tee -a $@ | $(COLORIZE); \
		if [ "x" != "x$$o" ]; then \
			echo "  \"$$c\":" | tee -a $@; \
			if cmp $$c $$o > /dev/null 2>&1; then \
				echo "    \"[PASS] output compare succeeded.\"," | tee -a $@ | $(COLORIZE); \
			else \
				echo "    \"[FAIL] output compare failed.\"," | tee -a $@ | $(COLORIZE); \
			fi; \
		fi; \
	done; \
	pass=$$(grep -c -F '[PASS]' $@); echo "  \"passed\": $$pass," | tee -a $@; \
	fail=$$(grep -c -F '[FAIL]' $@); echo "  \"failed\": $$fail" | tee -a $@; \
	echo "}" | tee -a $@; test $$fail -eq 0
