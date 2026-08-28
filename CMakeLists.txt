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

# Create the executable from SOURCES
add_executable(PulsarSim ${SOURCES})

# Set include directories for the executable
target_include_directories(PulsarSim PUBLIC src external/glad/include)
# Link the libraries to the executable
target_link_libraries(PulsarSim glfw glad OpenGL::GL glm::glm)