CC = g++
OBJS = src/nps.o
TARGET = bin/nps

.SUFFIXES : .cpp .o

all : $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -Wall -o $@ $(OBJS)

clean :
	rm -f $(OBJS) $(TARGET)
