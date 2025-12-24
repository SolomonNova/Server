#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "Stack.h"

/////////////////////////////////////////
/////////////////////////////////////////
bool initialize(Stack* s, size_t capacity, size_t elementSize)
{
    s->top = 0;
    s->capacity = capacity;
    s->elementSize = elementSize;
    s->array = calloc(s->capacity, s->elementSize);
    if (s->array == NULL) return false;

    return true;
}

/////////////////////////////////////////
/////////////////////////////////////////
bool resizeStack(Stack* s, size_t sizeMultiplier)
{
    s->capacity = s->capacity * sizeMultiplier;
    s->array = realloc(s->array, s->capacity);
    if (s->array == NULL) return false;

    return true;
}

/////////////////////////////////////////
/////////////////////////////////////////
bool push(Stack* s, void* value)
{
    if (s->top == s->capacity - 1)
        if (resizeStack(s, 2)) return false;

    void* base = s->array;
    base + (s->top + 1) * s->elementSize = value;

    // yet to be completed
}

int main()
{
    Stack s;
    bool isInitialized = initialize(&s, 10, 4);

    printf("stack capacity before: %zu\n", s.capacity);

    bool resized = resizeStack(&s, 2);
    printf("resize status: %s\n", resized ? "true" : "false");
    printf("stack capacity after: %zu\n", s.capacity);



    return 0;
}
