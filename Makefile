CXX := g++
CXXFLAGS := `pkg-config --libs --cflags ghoti.io-pool` -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -O3 -g
LDFLAGS := -L /usr/lib -lstdc++ -lm
BUILD := ./build
OBJ_DIR := $(BUILD)/objects
GEN_DIR := $(BUILD)/generated
APP_DIR := $(BUILD)/apps

BASE_NAME := libghoti.io-wave.so
MAJOR_VERSION := 0
MINOR_VERSION := 0.0
SO_NAME := $(BASE_NAME).$(MAJOR_VERSION)
TARGET := $(SO_NAME).$(MINOR_VERSION)

INCLUDE := -I include/ -I include/wave
LIBOBJECTS := $(OBJ_DIR)/client.o \
							$(OBJ_DIR)/response.o \
							$(OBJ_DIR)/request.o \
							$(OBJ_DIR)/server.o

TESTFLAGS := `pkg-config --libs --cflags gtest`


WAVELIBRARY := -L $(APP_DIR) -lghoti.io-wave


all: $(APP_DIR)/$(TARGET) ## Build the shared library

####################################################################
# Dependency Variables
####################################################################
DEP_CLIENT = \
	include/wave/client.hpp
DEP_RESPONSE = \
	include/wave/response.hpp
DEP_REQUEST = \
	include/wave/request.hpp
DEP_SERVER = \
	include/wave/server.hpp
DEP_WAVE = \
	$(DEP_CLIENT) \
	$(DEP_RESPONSE) \
	$(DEP_REQUEST) \
	$(DEP_SERVER) \
	include/wave.hpp

####################################################################
# Object Files
####################################################################

$(LIBOBJECTS) :
	@echo "\n### Compiling $@ ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@ -fPIC

$(OBJ_DIR)/client.o: \
				src/client.cpp \
				$(DEP_CLIENT)

$(OBJ_DIR)/response.o: \
				src/response.cpp \
				$(DEP_)

$(OBJ_DIR)/request.o: \
				src/request.cpp \
				$(DEP_)

$(OBJ_DIR)/server.o: \
				src/server.cpp \
				$(DEP_)

$(OBJ_DIR)/wave.o: \
				src/wave.cpp \
				$(DEP_WAVE)

####################################################################
# Shared Library
####################################################################

$(APP_DIR)/$(TARGET): \
				$(LIBOBJECTS)
	@echo "\n### Compiling Ghoti.io Wave Shared Library ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS) -Wl,-soname,$(SO_NAME)
	@ln -f -s $(TARGET) $(APP_DIR)/$(SO_NAME)
	@ln -f -s $(SO_NAME) $(APP_DIR)/$(BASE_NAME)

####################################################################
# Unit Tests
####################################################################

$(APP_DIR)/test: \
				test/test.cpp \
				$(DEP_WAVE) \
				$(APP_DIR)/$(TARGET)
	@echo "\n### Compiling Wave Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS) $(WAVELIBRARY)

####################################################################
# Commands
####################################################################

.PHONY: all clean cloc docs docs-pdf install test test-watch watch

watch: ## Watch the file directory for changes and compile the target
	@while true; do \
					make all; \
					echo "\033[0;32m"; \
					echo "#########################"; \
					echo "# Waiting for changes.. #"; \
					echo "#########################"; \
					echo "\033[0m"; \
					inotifywait -qr -e modify -e create -e delete -e move src include test Makefile --exclude '/\.'; \
					done

test-watch: ## Watch the file directory for changes and run the unit tests
	@while true; do \
					make test; \
					echo "\033[0;32m"; \
					echo "#########################"; \
					echo "# Waiting for changes.. #"; \
					echo "#########################"; \
					echo "\033[0m"; \
					inotifywait -qr -e modify -e create -e delete -e move src include test Makefile --exclude '/\.'; \
					done

test: ## Make and run the Unit tests
test: \
				$(APP_DIR)/test
	@echo "\033[0;32m"
	@echo "############################"
	@echo "### Running normal tests ###"
	@echo "############################"
	@echo "\033[0m"
	env LD_LIBRARY_PATH="$(APP_DIR)" $(APP_DIR)/test --gtest_brief=1

clean: ## Remove all contents of the build directories.
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(GEN_DIR)/*

install: ## Install the library globally, requires sudo
	# Install the Shared Library
	@mkdir -p /usr/local/lib/ghoti.io
	@cp $(APP_DIR)/$(TARGET) /usr/local/lib/ghoti.io/
	@ln -f -s $(TARGET) /usr/local/lib/ghoti.io/$(SO_NAME)
	@ln -f -s $(SO_NAME) /usr/local/lib/ghoti.io/$(BASE_NAME)
	@echo "/usr/local/lib/ghoti.io" > /etc/ld.so.conf.d/ghoti.io-wave.conf
	# Install the headers
	@mkdir -p /usr/local/include/ghoti.io/wave
	@cp include/wave.hpp /usr/local/include/ghoti.io/
	@#cp include/wave/*.hpp /usr/local/include/ghoti.io/wave/
	# Install the pkgconfig files
	@mkdir -p /usr/local/share/pkgconfig
	@cp pkgconfig/ghoti.io-wave.pc /usr/local/share/pkgconfig/
	# Run ldconfig
	@ldconfig >> /dev/null 2>&1
	@echo "Ghoti.io Wave installed"

docs: ## Generate the documentation in the ./docs subdirectory
	doxygen

docs-pdf: docs ## Generate the documentation as a pdf, in ./docs/latex/refman.pdf
	cd ./docs/latex/ && make

cloc: ## Count the lines of code used in the project
	cloc src include test Makefile

help: ## Display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "%-15s %s\n", $$1, $$2}'

