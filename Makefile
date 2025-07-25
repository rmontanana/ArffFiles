SHELL := /bin/bash
.DEFAULT_GOAL := help
.PHONY: help build test clean conan-build

f_debug = build_debug
test_targets = unit_tests_arffFiles
n_procs = -j 16

define ClearTests
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			echo ">>> Cleaning $$t..." ; \
			rm -f $(f_debug)/tests/$$t ; \
		fi ; \
	done
	@nfiles="$(find . -name "*.gcda" -print0)" ; \
	if test "${nfiles}" != "" ; then \
		find . -name "*.gcda" -print0 | xargs -0 rm 2>/dev/null ;\
	fi ; 
endef

clean: ## Clean the tests info
	@echo ">>> Cleaning Debug ArffFiles tests...";
	$(call ClearTests)
	@echo ">>> Done";

build: ## Build a debug version of the project
	@echo ">>> Building Debug Folding...";
	@if [ -d $(f_debug) ]; then rm -rf $(f_debug); fi
	@mkdir $(f_debug); 
	conan install . -of $(f_debug) -s build_type=Debug -b missing 
	cmake -B $(f_debug) -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$(f_debug)/conan_toolchain.cmake -DENABLE_TESTING=ON
	cmake --build $(f_debug) -t $(test_targets) $(n_procs)
	@echo ">>> Done";

opt = ""
test: ## Run tests (opt="-s") to verbose output the tests
	@echo ">>> Running ArffFiles tests...";
	@$(MAKE) clean
	@cmake --build $(f_debug) -t $(test_targets) $(n_procs)
	@for t in $(test_targets); do \
		if [ -f $(f_debug)/tests/$$t ]; then \
			cd $(f_debug)/tests ; \
			./$$t $(opt) ; \
		fi ; \
	done
	@echo ">>> Done";

conan-build: ## Build Conan package locally
	@echo ">>> Building Conan package...";
	@conan create . --profile default
	@echo ">>> Done";

help: ## Show help message
	@IFS=$$'\n' ; \
	help_lines=(`fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##/:/'`); \
	printf "%s\n\n" "Usage: make [task]"; \
	printf "%-20s %s\n" "task" "help" ; \
	printf "%-20s %s\n" "------" "----" ; \
	for help_line in $${help_lines[@]}; do \
		IFS=$$':' ; \
		help_split=($$help_line) ; \
		help_command=`echo $${help_split[0]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		help_info=`echo $${help_split[2]} | sed -e 's/^ *//' -e 's/ *$$//'` ; \
		printf '\033[36m'; \
		printf "%-20s %s" $$help_command ; \
		printf '\033[0m'; \
		printf "%s\n" $$help_info; \
	done
