

CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

a.out: *.cpp *.hpp
	g++ $(CFLAGS) -o main *.cpp *.hpp $(LDFLAGS)

.PHONY: test clean

test: main
	./main

clean:
	rm -f main