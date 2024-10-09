# Name of the program (can be passed as an argument)
TARGET = ex3

# Cross-compiler
CC = arm-linux-gnueabihf-gcc-6.4.1

# Source files
SRCS = ex3.c hex.c led.c button.c

# Compilation flags
CFLAGS = -Wall

# Deployment directory
DEPLOY_DIR = /export/drv/labo01

# Main rule, compiles the target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(SRCS)
	@echo "Compiling $(TARGET)"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean rule to remove compiled files
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)

# Deploy rule to copy the executable to the target directory
deploy: $(TARGET)
	@echo "Setting the execution permission"
	chmod +x $(TARGET)
	@echo "Copying $(TARGET) to $(DEPLOY_DIR)"
	sudo cp $(TARGET) $(DEPLOY_DIR)

.PHONY: all clean deploy
