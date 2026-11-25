cmake_minimum_required(VERSION 4.3)
project(bancodigital C)

set(CMAKE_C_STANDARD 11)

add_executable(bancodigital main.c
        banco.c
        meubanco.h
)
