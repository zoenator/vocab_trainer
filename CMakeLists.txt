cmake_minimum_required(VERSION 3.15)
project(VocabTrainer)

set(CMAKE_C_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(Vocab_cli src/vocab_cli.c src/paths.c)
add_executable(VocabDaemon src/vocab_daemon.c src/paths.c)

install(TARGETS Vocab_cli VocabDaemon DESTINATION bin)