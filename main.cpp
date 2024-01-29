#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <cstdint>
#include <unistd.h>

#define time_ns() std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count()

const int WIDTH = 800;
const int HEIGHT = 600;

class VulkanHelloWorld {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Hello World", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        // Additional Vulkan initialization steps can be added here
    }

    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    void mainLoop() {
        uint64_t start_time = time_ns();
        printf("current_time %ld\n", start_time);
        uint64_t i = 0;
        while (!glfwWindowShouldClose(window)) {
            i++;
            //printf("%ld\n", ++i);
            glfwPollEvents();
            drawFrame();
        }
        uint64_t end_time = time_ns();
        float delta_time = (float)(end_time - start_time);
        printf("nanoseconds: %f\n", delta_time);
        delta_time = delta_time/1000000000;
        printf("seconds: %f\n", delta_time);
        printf("fps: %f\n", (float)(i / delta_time));
    }

    void drawFrame() {
        // TODO
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    VulkanHelloWorld app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "success end" << std::endl;
    return EXIT_SUCCESS;
}


//sudo apt install linux-tools-6.5.0-15-generic
//sudo apt install linux-cloud-tools-6.5.0-15-generic
//dont install linux-nvidia-6.2-tools-common

