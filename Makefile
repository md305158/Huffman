CC=g++
CFLAGS = -std=c++17 -Wall -Wextra -Wshadow -pedantic -Werror
TARGET = huffman

SRCDIR=src
OBJDIR=obj
INCDIR=inc
$(shell mkdir -p $(OBJDIR))

SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJ=$(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

%.o : %.cpp

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -I $(INCDIR)

clean:
	@-rm -r $(OBJDIR) $(TARGET)
