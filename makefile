# The name of the source files
SOURCES = main.c shell.c

# The name of the executable
EXE = shell

# Flags for compilation (adding warnings are always good)
CFLAGS = -Wall -g -Wfatal-errors -Werror -Wextra

LIBRARIES =

# Use the GCC frontend program when linking
LD = gcc

# The first target, this will be the default target if none is specified
# This target tells "make" to make the "all" target
default: all

# Having an "all" target is customary, so one could write "make all"
# It depends on the executable program
all: $(EXE)

# This will link the executable from the object files
$(EXE): $(OBJECTS)
	$(LD) $(SOURCES) -o $(EXE) $(LIBRARIES) $(CFLAGS)

# Target to clean up after us
clean:
	-rm -f $(EXE)      # Remove the executable file
