# Executable name
EXEC_NAME = exe

# Folder structure
SRC_DIR = ./src
COMMON_DIR = ../common
BUILD_DIR = ./build
BIN_DIR = ./bin

# Executable path
EXEC = $(BIN_DIR)/$(EXEC_NAME)

# Commands
RM = rm -f
CC = gcc
MKDIR_P = mkdir -p

# Compilation flags
CCFLAGS = -Wall -MMD -MP
CCFLAGS += -lrt
CCFLAGS += -O0
CCFLAGS += -lssl -lcrypto
LDFLAGS = -lm
LDFLAGS += -lssl -lcrypto
LDFLAGS = -lm -lssl -lcrypto -lprotobuf-c


# Error handling 
CCFLAGS += -Werror=uninitialized

# Source files
SRC = $(wildcard $(shell find $(SRC_DIR) -type f -regex ".*\.c"))
SRC += $(wildcard $(shell find $(COMMON_DIR) -type f -regex ".*\.c"))

# Object files
OBJ = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRC:.c=.o))
OBJ := $(patsubst $(COMMON_DIR)/%, $(BUILD_DIR)/common/%, $(OBJ))
DEP = $(OBJ:.o=.d)

# Compilation rules
all: $(EXEC)

test: $(EXEC) run

debug: CCFLAGS += -g
debug: rebuild

rebuild: distclean all

# Link the object files to create the executable
$(EXEC): $(OBJ)
	@ echo "\n==== Linker ========="
	@ $(MKDIR_P) $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)
	@ echo "==== Linker done ====\n"

# Dependencies
-include $(DEP)

# Compile the source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@ echo "\n==== Build Object $< ========="
	@ $(MKDIR_P) $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@
	@ echo "==== Build Object done $< ====\n"

$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.c
	@ echo "\n==== Build Object $< ========="
	@ $(MKDIR_P) $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@
	@ echo "==== Build Object done $< ====\n"

run: 
	@ echo "\n==== Run ${EXEC_NAME} ========="
	${EXEC}
	@ echo "==== Run Alambix done ====\n"

# Clean the project
clean:
	@ echo "==== Cleaning build files ===="
	$(RM) -r $(BUILD_DIR)
	@ echo "==== Build files cleaned ====="

distclean: clean
	@ echo "==== Cleaning binary files ===="
	$(RM) -r $(BIN_DIR)
	@ echo "==== Binary files cleaned ====="


# Phony targets
.PHONY: all clean test run rebuild debug distclean