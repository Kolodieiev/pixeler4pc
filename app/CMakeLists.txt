cmake_minimum_required(VERSION 3.20)
project(Pixeler4PC LANGUAGES CXX C)

set(EN_GIT_BUILD ON) # GIT SFML 3.0.2
set(EN_LOCAL_BUILD OFF) # Локальний SFML 3.0.2
set(EN_STATIC_SFML_LIB OFF) # Локальний SFML 3.0.2. Якщо бібліотеки зібрані статично - ON. Інакше OFF.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(EN_GIT_BUILD)
    include(FetchContent)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
    FetchContent_MakeAvailable(SFML)

elseif(EN_LOCAL_BUILD)
    if(EN_STATIC_SFML_LIB)
        set(SFML_STATIC_LIBRARIES TRUE)

    endif()
    find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics)

else()
    message(FATAL_ERROR "Потрібно обрати один із варіантів збірки")

endif()

file(GLOB_RECURSE CPP_SOURCES
    "src/*.cpp"
    "src/**/*.cpp"
)

file(GLOB_RECURSE C_SOURCES
    "src/*.c"
    "src/**/*.c"
)

add_executable(main ${CPP_SOURCES} ${C_SOURCES}) 

set_source_files_properties(${C_SOURCES} PROPERTIES LANGUAGE C)

target_include_directories(main PUBLIC  ${CMAKE_SOURCE_DIR}/src)
target_compile_features(main PRIVATE cxx_std_17)
target_compile_options(main PRIVATE -Wno-unknown-pragmas)

target_link_libraries(main PRIVATE SFML::Graphics)
