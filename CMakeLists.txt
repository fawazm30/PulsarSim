cmake_minimum_required(VERSION 3.20)
project(PulsarSim)

# Set C++ standard here...
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # throws and error if C++17 is not supported

# Suppress OpenGL deprecation warnings on macOS
add_definitions(-DGL_SILENCE_DEPRECATION)

# Find these three packages...
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# Add GLAD as a static library from external/glad/src/glad.c
add_library(glad STATIC external/glad/src/glad.c)
# and give it the include path external/glad/include
target_include_directories(glad PUBLIC external/glad/include)

# Collect all .cpp files under src/ into a variable called SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp)

# Add ImGui as a static library
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

# Create the executable from SOURCES
add_executable(PulsarSim ${SOURCES})

# Set include directories for the executable
target_include_directories(PulsarSim PUBLIC src external/glad/include)
# Link the libraries to the executable
target_link_libraries(PulsarSim glfw glad imgui OpenGL::GL glm::glm)