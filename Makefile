# Compiler and Flags 
CC = gcc
CFLAGS = -g -Wall -std=gnu11 -Ifont -Itigr -Iutils

# Project Files
SRCS = main.c tigr/tigr.c utils/utils.c
OBJS = $(SRCS:.c=.o)
EXE = wscroller

# Detect OS, set linker flags
ifeq ($(OS),Windows_NT)
    # Windows-specific libraries for graphics and windowing.
    LDFLAGS = -lopengl32 -lgdi32 -mwindows
else
    # Linux/Unix-specific libraries for OpenGL, GLU, X11, and math.
    LDFLAGS = -lGL -lGLU -lX11 -lm
endif


# Default build target
all: $(EXE)

# Link object files and font header to final executable
$(EXE): $(OBJS) font/font.h
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

# Compile .c files to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f $(OBJS) $(EXE)

# Declare actions
.PHONY: all clean