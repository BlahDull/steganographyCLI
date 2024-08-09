CXX = g++
CXXFLAGS = -g -Wall
PKGCONFIG = pkg-config
PKGCONFIG_FLAGS = --cflags --libs opencv4

TARGET = stegotool
SRCS = stegotool.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ `$(PKGCONFIG) $(PKGCONFIG_FLAGS)`

%.o: %.cpp
	$(CXX) $(CXXFLAGS) `$(PKGCONFIG) $(PKGCONFIG_FLAGS)` -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
