CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=

SRCDIR=src
OBJDIR=build
BINDIR=bin

SOURCES= $(shell find $(SRCDIR) -type f -name *.cpp)
OBJECTS= $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.cpp=.o))

BIN=$(BINDIR)/cagney

all: setup $(BIN)

$(BIN): $(OBJECTS) ; $(CC) $(LDFLAGS) $(OBJECTS) -o $@ -L../legit/bin -L../../lib/Botan-2.4.0 -llegit -lbotan-2 -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp ; $(CC) $(CFLAGS) -o $@ $< -I../legit/Legit -I../../lib/Botan-2.4.0/build/include

clean: ; rm -f $(BINDIR)/* $(OBJDIR)/*

setup: ; mkdir -p $(BINDIR) $(OBJDIR)

print-%  : ; @echo $* = $($*)

