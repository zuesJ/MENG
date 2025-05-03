CC = gcc
CFLAGS = -Wextra
BUILD_DIR = build/

LIBRARY = libMENG.a
BUILD_NAME = "MENG"

SRC := $(filter-out src/main.c, $(wildcard src/*.c))
OBJ := $(patsubst src/%.c, $(BUILD_DIR)%.o, $(SRC))

HEADER = src/VENG.h
LIBS = 

all: $(LIBRARY)
debug: src/main.c $(LIBRARY)
	@echo "Building Debug App . . ."
	@$(CC) $< -o $(BUILD_NAME) $(OBJ) $(LIBS) -L ./ -lMENG
	@echo "Done!"
	@echo ""

$(LIBRARY): build_dir $(OBJ)
	@echo "Compiling lib . . ."
	@ar rcs $@ $(OBJ)
	@echo "Done!"
	@echo ""

$(BUILD_DIR)%.o: src/%.c
	@echo "Compiling $<"
	@$(CC) -c $< -o $@ $(CFLAGS) $(LIBS)

build_dir:
	@echo "Making build directory"
	@mkdir -p $(BUILD_DIR)

clean:
	@echo "Cleaning . . ."
	@rm $(LIBRARY)
	@rm $(BUILD_NAME)
	@rm -rf $(BUILD_DIR)