IDIR =./include
ODIR =./bin
SDIR =./src
CC=g++

CFLAGS=-std=c++17 -O3 -g -I$(IDIR)
LIBS=-lm

_DEPS = calculator.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = calculator.o main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/main.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: $(ODIR)/main.exe

.PHONY: clean

clean:
	rm -f $(ODIR)/*