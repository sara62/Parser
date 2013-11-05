SOURCEDIR = ./sources/
INCLUDEDIR = ./includes/
BUILDDIR = ./build/

OFILES = $(addprefix $(BUILDDIR),$(addsuffix .o,$(basename $(notdir $(join $(join( $(wildcard $(SOURCEDIR)*.c), $(wildcard $(SOURCEDIR)*.s)), $(wildcard $(SOURCEDIR)*.cpp))))))

CC = gcc
CPP = g++
CCFLAGS = -c -O2 -Wall -Wextra -I$(INCLUDEDIR)
CSTD = -std=c99
CPPSTD = -std=c++11
AS = as

all: a.out

a.out: $(OFILES)
	$(CC) $^ -o $@

$(BUILDDIR)%.o: $(SOURCEDIR)%.c
	$(CC) $(CCFLAGS) $(CSTD) $< -o $@

$(BUILDDIR)%.o: $(SOURCEDIR)%.s
	$(AS) $< -o $@
	
$(BUILDDIR)%.o: $(SOURCEDIR)%.cpp
	$(CPP) $(CCFLAGS) $(CPPSTD) $< -o $@

.PHONY: clean
clean:
	rm -f $(BUILDDIR)*
	rm a.out
