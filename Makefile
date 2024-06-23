CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -Werror -g -march=native -flto=auto -fno-omit-frame-pointer

TARGET = bot

SRC = bot.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

