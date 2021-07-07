# vim: :set noexpandtab

# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall

# the build target executable:
TARGET = SampleCompiler

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o ../Build/Debug/$(TARGET) lfio.c dinput.c parse.c lex.c main.c dynarray.c

clean:
	$(RM) $(TARGET)
