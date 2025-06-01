# Executable names
SERVER_EXEC_NAME = exe
CLIENT_EXEC_NAME = client

# Folder structure
SRC_DIR = ./src
BUILD_DIR = ./build
BIN_DIR = ./bin

# Executable paths
SERVER_EXEC = $(BIN_DIR)/$(SERVER_EXEC_NAME)
CLIENT_EXEC = $(BIN_DIR)/$(CLIENT_EXEC_NAME)

# Commands
RM = rm -f
CC = gcc
MKDIR_P = mkdir -p

# Compilation flags
CCFLAGS = -Wall -MMD -MP -Werror=uninitialized -O0 -lrt
LDFLAGS = -lm -lprotobuf-c

# File structure
PROTOBUF_SRC = $(SRC_DIR)/package/protocol/protobuf/dist/src/message.pb-c.c
PROTOCOL_SRC = $(SRC_DIR)/package/protocol/protobuf/src/protocol.c
CLIENT_SRC = $(SRC_DIR)/client.c
SERVER_SRC = $(wildcard $(shell find $(SRC_DIR) -type f -name "*.c" ! -name "client.c"))

# Object files
PROTOBUF_OBJ = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(PROTOBUF_SRC:.c=.o))
PROTOCOL_OBJ = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(PROTOCOL_SRC:.c=.o))
CLIENT_OBJ   = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(CLIENT_SRC:.c=.o)) $(PROTOBUF_OBJ) $(PROTOCOL_OBJ)
SERVER_OBJ   = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SERVER_SRC:.c=.o)) $(PROTOBUF_OBJ) $(PROTOCOL_OBJ)

# Dependency files
DEP = $(CLIENT_OBJ:.o=.d) $(SERVER_OBJ:.o=.d)

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC)

# Build server executable
$(SERVER_EXEC): $(SERVER_OBJ)
	@echo "\n==== Linking Server ===="
	@$(MKDIR_P) $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "==== Server build done ====\n"

# Build client executable
$(CLIENT_EXEC): $(CLIENT_OBJ)
	@echo "\n==== Linking Client ===="
	@$(MKDIR_P) $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "==== Client build done ====\n"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "\n==== Compiling $< ===="
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@
	@echo "==== Done: $< ====\n"

# Run targets
run-server: $(SERVER_EXEC)
	@echo "\n==== Running Server ===="
	$(SERVER_EXEC)
	@echo "==== Server exited ====\n"

run-client: $(CLIENT_EXEC)
	@echo "\n==== Running Client ===="
	$(CLIENT_EXEC)
	@echo "==== Client exited ====\n"

# Debug mode
debug: CCFLAGS += -g
debug: rebuild

# Rebuild everything
rebuild: distclean all

# Clean targets
clean:
	@echo "==== Cleaning build files ===="
	$(RM) -r $(BUILD_DIR)
	@echo "==== Build files cleaned ===="

distclean: clean
	@echo "==== Cleaning binaries ===="
	$(RM) -r $(BIN_DIR)
	@echo "==== Binaries cleaned ===="

# Include dependencies
-include $(DEP)

docs:
	doxygen Doxyfile

docs-clean:
	rm -rf docs/html docs/latex

# Phony targets
.PHONY: all clean distclean rebuild debug run-server run-client docs docs-clean
