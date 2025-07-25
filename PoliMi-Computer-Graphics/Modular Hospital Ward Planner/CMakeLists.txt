cmake_minimum_required(VERSION 3.10)
project(Modular_Hospital_Ward_Planner C CXX)

# Enable C and C++ languages
enable_language(C)
enable_language(CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(APPLE)
    # macOS specific configurations

    # Vulkan Setup for macOS
    find_package(Vulkan REQUIRED)
    if(Vulkan_FOUND)
        message(STATUS "Vulkan found at ${Vulkan_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "Vulkan not found!")
    endif()

    # GLFW and GLM Setup for macOS
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Threads REQUIRED)

    # Define the project source and header files
    file(GLOB_RECURSE SOURCES src/*.cpp)
    file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)

    # Define the project executable
    add_executable(Modular_Hospital_Ward_Planner ${SOURCES} ${HEADERS})

    # Include directories
    target_include_directories(Modular_Hospital_Ward_Planner PRIVATE ${GLM_INCLUDE_DIRS})
    target_include_directories(Modular_Hospital_Ward_Planner PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_include_directories(Modular_Hospital_Ward_Planner PUBLIC ${Vulkan_INCLUDE_DIR})

    # Link libraries
    target_link_libraries(Modular_Hospital_Ward_Planner PRIVATE Vulkan::Vulkan glfw Threads::Threads)

    # Shader compilation for macOS
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

    file(GLOB GLSL_SOURCE_FILES "${CMAKE_SOURCE_DIR}/shaders/*.vert" "${CMAKE_SOURCE_DIR}/shaders/*.frag")

    set(SPIRV_BINARY_FILES "")
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME_WE)
        string(REPLACE "Shader" "" FILE_NAME ${FILE_NAME})
        get_filename_component(FILE_EXT ${GLSL} EXT)
        set(SPV_FILE ${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}${FILE_EXT}.spv)
        string(REPLACE ".vert" "Vert" SPV_FILE ${SPV_FILE})
        string(REPLACE ".frag" "Frag" SPV_FILE ${SPV_FILE})

        add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND glslangValidator -V ${GLSL} -o ${SPV_FILE}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${GLSL}"
        )
        list(APPEND SPIRV_BINARY_FILES ${SPV_FILE})
    endforeach()

    add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
    add_dependencies(Modular_Hospital_Ward_Planner Shaders)

    # Copy resources (textures, models, etc.) to the build directory
    file(COPY ${CMAKE_SOURCE_DIR}/assets/textures DESTINATION ${CMAKE_BINARY_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/models DESTINATION ${CMAKE_BINARY_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/Icons DESTINATION ${CMAKE_BINARY_DIR}/assets)

elseif(WIN32)
    # Windows specific configurations

    # Set paths to libraries (modify these based on your SDK and library installation)
    set(GLFW "C:/VulkanSDK/libs/glfw-3.4.bin.WIN64")
    set(GLM "C:/VulkanSDK/libs/glm")

    # List of directories to add to the include path
    list(APPEND INCLUDE_DIRS "${GLFW}/include" ${GLM} headers)

    # List of libraries to link to the executable
    list(APPEND LINK_LIBS "${GLFW}/lib-mingw-w64/libglfw3.a")
    file(GLOB_RECURSE SOURCES src/*.cpp)
    file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)

    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    find_package(Vulkan REQUIRED)

    foreach(dir IN LISTS Vulkan_INCLUDE_DIR INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PUBLIC ${dir})
    endforeach()

    foreach(lib IN LISTS Vulkan_LIBRARIES LINK_LIBS)
        target_link_libraries(${PROJECT_NAME} ${lib})
    endforeach()

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

    file(GLOB GLSL_SOURCE_FILES "${CMAKE_SOURCE_DIR}/shaders/*.vert" "${CMAKE_SOURCE_DIR}/shaders/*.frag")

    set(SPIRV_BINARY_FILES "")
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME_WE)
        string(REPLACE "Shader" "" FILE_NAME ${FILE_NAME})
        get_filename_component(FILE_EXT ${GLSL} EXT)
        set(SPV_FILE ${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}${FILE_EXT}.spv)
        string(REPLACE ".vert" "Vert" SPV_FILE ${SPV_FILE})
        string(REPLACE ".frag" "Frag" SPV_FILE ${SPV_FILE})

        add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND glslangValidator -V ${GLSL} -o ${SPV_FILE}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${GLSL}"
        )
        list(APPEND SPIRV_BINARY_FILES ${SPV_FILE})
    endforeach()

    add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
    add_dependencies(${PROJECT_NAME} Shaders)

    file(GLOB SHADER_SPV_FILES "${CMAKE_SOURCE_DIR}/shaders/*.spv")
    file(COPY ${SHADER_SPV_FILES} DESTINATION ${CMAKE_BINARY_DIR}/shaders)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/textures DESTINATION ${CMAKE_BINARY_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/models DESTINATION ${CMAKE_BINARY_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/Icons DESTINATION ${CMAKE_BINARY_DIR}/assets)
elseif(UNIX AND NOT APPLE)
    file(GLOB_RECURSE SOURCES src/*.cpp)
    file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)

    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    find_package(Vulkan REQUIRED)
    find_package(glfw3 REQUIRED)


    find_package(glm REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)

    foreach(dir IN LISTS Vulkan_INCLUDE_DIR INCLUDE_DIRS)
        target_include_directories(${PROJECT_NAME} PUBLIC ${dir})
    endforeach()

    foreach(lib IN LISTS Vulkan_LIBRARIES LINK_LIBS)
        target_link_libraries(${PROJECT_NAME} ${lib})
    endforeach()

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

    file(GLOB GLSL_SOURCE_FILES "${CMAKE_SOURCE_DIR}/shaders/*.vert" "${CMAKE_SOURCE_DIR}/shaders/*.frag")

    set(SPIRV_BINARY_FILES "")
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME_WE)
        string(REPLACE "Shader" "" FILE_NAME ${FILE_NAME})
        get_filename_component(FILE_EXT ${GLSL} EXT)
        set(SPV_FILE ${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}${FILE_EXT}.spv)
        string(REPLACE ".vert" "Vert" SPV_FILE ${SPV_FILE})
        string(REPLACE ".frag" "Frag" SPV_FILE ${SPV_FILE})

        add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND glslangValidator -V ${GLSL} -o ${SPV_FILE}
                DEPENDS ${GLSL}
                COMMENT "Compiling shader: ${GLSL}"
        )
        list(APPEND SPIRV_BINARY_FILES ${SPV_FILE})
    endforeach()

    add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
    add_dependencies(${PROJECT_NAME} Shaders)

    file(GLOB SHADER_SPV_FILES "${CMAKE_SOURCE_DIR}/shaders/*.spv")
    file(COPY ${SHADER_SPV_FILES} DESTINATION ${CMAKE_BINARY_DIR}/shaders)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/textures DESTINATION ${CMAKE_BINARY_DIR}/assets)

    file(COPY ${CMAKE_SOURCE_DIR}/assets/models DESTINATION ${CMAKE_BINARY_DIR}/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/assets/Icons DESTINATION ${CMAKE_BINARY_DIR}/assets)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

