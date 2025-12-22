cmake_minimum_required(VERSION 3.20)
project(Pixeler4PC LANGUAGES CXX C)

set(EN_GIT_BUILD OFF) # GIT SFML 3.0.2
set(EN_LOCAL_BUILD ON) # Локальний SFML 3.0.2
set(EN_STATIC_SFML_LIB ON) # Локальний SFML 3.0.2. Якщо бібліотеки зібрані статично - ON. Інакше OFF.
set(EN_ANDROID_BUILD OFF) # Android Сxxdroid SFML 2.5.1 

if(NOT EN_ANDROID_BUILD) # PC
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

endif()

if(EN_GIT_BUILD)
    include(FetchContent)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
    FetchContent_MakeAvailable(SFML)

elseif(EN_LOCAL_BUILD)
    if(EN_STATIC_SFML_LIB)
        set(SFML_STATIC_LIBRARIES TRUE)

    endif()
    find_package(SFML 3.0.2 REQUIRED COMPONENTS Graphics)

elseif(EN_ANDROID_BUILD)
    find_package(SFML COMPONENTS system window graphics REQUIRED)

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

if(NOT EN_ANDROID_BUILD) # PC
    target_link_libraries(main PRIVATE SFML::Graphics)

else() # Cxxdroid
    target_link_libraries(main
        sfml-system
        sfml-window
        sfml-graphics
    )

endif() # if(NOT EN_ANDROID_BUILD)

