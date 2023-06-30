CXX := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -Wno-error=unused-function -Wfatal-errors -std=c++20 -O3 -g
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
LIBOBJECTS := $(OBJ_DIR)/blob.o \
							$(OBJ_DIR)/client.o \
							$(OBJ_DIR)/clientSession.o \
							$(OBJ_DIR)/hasClientParameters.o \
							$(OBJ_DIR)/hasServerParameters.o \
							$(OBJ_DIR)/parser.o \
							$(OBJ_DIR)/parsing.o \
							$(OBJ_DIR)/response.o \
							$(OBJ_DIR)/message.o \
							$(OBJ_DIR)/server.o \
							$(OBJ_DIR)/serverSession.o

TESTFLAGS := `pkg-config --libs --cflags gtest`


WAVELIBRARY := -L $(APP_DIR) -lghoti.io-wave `pkg-config --libs --cflags ghoti.io-util ghoti.io-os ghoti.io-pool`


all: $(APP_DIR)/$(TARGET) ## Build the shared library

####################################################################
# Dependency Variables
####################################################################
DEP_MACROS = \
	include/wave/macros.hpp
DEP_PARSING = \
	include/wave/parsing.hpp
DEP_BLOB = \
	include/wave/blob.hpp
DEP_HASPARAMETERS = \
	include/wave/hasParameters.hpp
DEP_HASCLIENTPARAMETERS = \
	$(DEP_HASPARAMETERS) \
	include/wave/hasClientParameters.hpp
DEP_HASSERVERPARAMETERS = \
	$(DEP_HASPARAMETERS) \
	include/wave/hasServerParameters.hpp
DEP_MESSAGE = \
	$(DEP_BLOB) \
	$(DEP_PARSING) \
	include/wave/message.hpp
DEP_PARSER = \
	$(DEP_HASCLIENTPARAMETERS) \
	$(DEP_HASSERVERPARAMETERS) \
	$(DEP_BLOB) \
	$(DEP_PARSING) \
	$(DEP_MESSAGE) \
	include/wave/parser.hpp
DEP_RESPONSE = \
	include/wave/response.hpp
DEP_CLIENTSESSION = \
	$(DEP_HASCLIENTPARAMETERS) \
	$(DEP_PARSER) \
	$(DEP_MESSAGE) \
	include/wave/clientSession.hpp
DEP_SERVERSESSION = \
	$(DEP_HASSERVERPARAMETERS) \
	$(DEP_PARSER) \
	$(DEP_MESSAGE) \
	include/wave/serverSession.hpp
DEP_CLIENT = \
	$(DEP_HASCLIENTPARAMETERS) \
	$(DEP_CLIENTSESSION) \
	include/wave/client.hpp
DEP_SERVER = \
	$(DEP_HASSERVERPARAMETERS) \
	$(DEP_SERVERSESSION) \
	include/wave/server.hpp
DEP_WAVE = \
	$(DEP_HASCLIENTPARAMETERS) \
	$(DEP_HASSERVERPARAMETERS) \
	$(DEP_CLIENT) \
	$(DEP_CLIENTSESSION) \
	$(DEP_MACROS) \
	$(DEP_RESPONSE) \
	$(DEP_MESSAGE) \
	$(DEP_SERVER) \
	$(DEP_SERVERSESSION) \
	include/wave.hpp

####################################################################
# Object Files
####################################################################

$(LIBOBJECTS) :
	@echo "\n### Compiling $@ ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@ -fPIC

$(OBJ_DIR)/blob.o: \
				src/blob.cpp \
				$(DEP_BLOB)

$(OBJ_DIR)/client.o: \
				src/client.cpp \
				$(DEP_CLIENT)

$(OBJ_DIR)/clientSession.o: \
				src/clientSession.cpp \
				$(DEP_CLIENTSESSION)

$(OBJ_DIR)/hasClientParameters.o: \
				src/hasClientParameters.cpp \
				$(DEP_HASCLIENTPARAMETERS)

$(OBJ_DIR)/hasServerParameters.o: \
				src/hasServerParameters.cpp \
				$(DEP_HASSERVERPARAMETERS)

$(OBJ_DIR)/parser.o: \
				src/parser.cpp \
				$(DEP_PARSER)

$(OBJ_DIR)/parsing.o: \
				src/parsing.cpp \
				$(DEP_PARSING)

$(OBJ_DIR)/response.o: \
				src/response.cpp \
				$(DEP_RESPONSE)

$(OBJ_DIR)/message.o: \
				src/message.cpp \
				$(DEP_MESSAGE)

$(OBJ_DIR)/server.o: \
				src/server.cpp \
				$(DEP_SERVER)

$(OBJ_DIR)/serverSession.o: \
				src/serverSession.cpp \
				$(DEP_SERVERSESSION)

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
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS) `pkg-config --libs --cflags ghoti.io-pool ghoti.io-shared_string_view` -Wl,-soname,$(SO_NAME)
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

$(APP_DIR)/test-hasParameters: \
				test/test-hasParameters.cpp \
				$(DEP_HASPARAMETERS)
	@echo "\n### Compiling Wave HasParameters Test ###"
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $< $(LDFLAGS) $(TESTFLAGS)

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
				$(APP_DIR)/test-hasParameters \
				$(APP_DIR)/test
	@echo "\033[0;32m"
	@echo "############################"
	@echo "### Running normal tests ###"
	@echo "############################"
	@echo "\033[0m"
	env $(APP_DIR)/test-hasParameters --gtest_brief=1
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

docs-pdf: docs ## Generate the documentation as a pdf, in ./docs/wave-docs.pdf
	cd ./docs/latex/ && make
	mv -f ./docs/latex/refman.pdf ./docs/wave-docs.pdf

cloc: ## Count the lines of code used in the project
	cloc src include test Makefile

help: ## Display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "%-15s %s\n", $$1, $$2}'

