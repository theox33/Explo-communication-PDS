# Variables for directories and command
PROTOC_C   := protoc-c
SRC_DIR    := ./src
DIST_DIR   := ./dist
PROTO_FILE := $(SRC_DIR)/message.proto

# When invoking protoc-c, the generated files are named using the proto filename,
# in this case, message_pb-c.c and message_pb-c.h will be created in $(DIST_DIR).

# Default target
all: generate

# Target to generate the C and header files from the .proto file
generate:
	@echo "Generating C source and header files from $(PROTO_FILE)..."
	@mkdir -p $(DIST_DIR)
	$(PROTOC_C) --c_out=$(DIST_DIR) $(PROTO_FILE)
	@echo "Generation complete. Files can be found in $(DIST_DIR)"

# Clean target to remove generated files
clean:
	@echo "Cleaning up generated files..."
	@rm -f $(DIST_DIR)/src/*_pb-c.c $(DIST_DIR)/src/*_pb-c.h
	@echo "Clean complete."

.PHONY: all generate clean
