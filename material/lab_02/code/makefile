# Name of the program (can be passed as an argument)
TARGET = ex03

# Cross-compiler
CC = arm-linux-gnueabihf-gcc-6.4.1

# Source files
SRCS = $(TARGET).c hex.c led.c logger.c switch.c button.c

# Compilation flags
CFLAGS = -Wall

# Deployment directory
DEPLOY_DIR = /export/drv/labo02/

# Main rule, compiles the target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(SRCS)
	@echo "Compiling $(TARGET)"
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

# Rule to clean up the project
clean:
	@echo "Cleaning up"
	rm -f $(TARGET)

deploy:
	@echo "Deploying $(TARGET)"
	cp $(TARGET) $(DEPLOY_DIR)
