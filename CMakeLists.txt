cmake_minimum_required(VERSION 3.15)
project(VocabTrainer C)

set(CMAKE_C_STANDARD 11)

# System dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Global includes
include_directories(${PROJECT_SOURCE_DIR}/include)

# Executables
add_executable(Vocab_cli 
    src/vocab_cli.c 
    src/paths.c
    src/db.c
    src/sm2.c
)

add_executable(VocabDaemon 
    src/vocab_daemon.c 
    src/paths.c 
    src/db.c 
    src/utils.c 
    src/eval.c 
    src/sm2.c 
    src/ui.c
)

add_executable(Vocab_GUI 
    src/vocab_gui.c
    src/paths.c 
    src/db.c 
)

# Link GTK3 to GUI
target_include_directories(Vocab_GUI PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(Vocab_GUI PRIVATE ${GTK3_LIBRARIES})
target_compile_options(Vocab_GUI PRIVATE ${GTK3_CFLAGS_OTHER})

# Install rules
install(TARGETS Vocab_cli VocabDaemon Vocab_GUI DESTINATION bin)