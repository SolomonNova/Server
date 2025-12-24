#pragma once
/*
    File name: Stack.h
    Created at: 22-12-25
    Author: Solomon
*/

#ifndef STACK_H
#define STACK_H

#include <string.h>
#include <stdbool.h>

typedef struct Stack
{
    size_t top; // index
    size_t capacity;
    size_t elementSize;
    void* array;
} Stack;

bool initialize(Stack* s, size_t capacity, size_t elementSize);
bool resizeStack(Stack* s, size_t sizeMultiplier);

#endif