cmake_minimum_required(VERSION 2.8.4)
project(zdb)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -std=c99")

set(SOURCE_FILES
    break.c
    break.h
    break_defs.h
    common_defs.h
    elf.c
    elf.h
    elf_defs.h
    main.c
    stab.h)

add_executable(zdb ${SOURCE_FILES})