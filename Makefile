# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Libraries to link
LIBS = -lm -lpthread -lpaho-mqtt3c -lcjson -lsqlite3 -lcurl

# Target application name
TARGET = mqtt_app

# Source files and object files
SRCS = main.c mqtt_client.c memory_search.c db_helper.c firebase_helper.c
OBJS = $(SRCS:.c=.o)

# Header dependencies
DEPS = mqtt_thr.h db_helper.h firebase_helper.h

# Default target to build the application
all: $(TARGET)

# Rule to link object files and create the executable with libraries
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile .c files into .o files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET) *.db

# Run the application
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run