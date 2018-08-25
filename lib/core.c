#include <stdlib.h>
#include <stdio.h>

 struct ArrayRef {
    void* arr;
    unsigned int size;
    unsigned int capacity;
    unsigned int itemSize;
};

void printd(double v) {
    printf("%f\n", v);
}

void printds(struct ArrayRef* source) {
    struct ArrayRef ref = *source;

    for (void* i = ref.arr; i < ref.arr + ref.size * ref.itemSize; i += ref.itemSize) {
        printf("%f, ", *((double*) i));
    }
}

void createArray(struct ArrayRef* ref, unsigned int size, unsigned int capacity, unsigned int itemSize) {
    struct ArrayRef result;
    result.arr = malloc(capacity * itemSize);
    result.size = size;
    result.capacity = capacity,
    result.itemSize = itemSize;
    *ref = result;
}

void destroyArray(struct ArrayRef* ref) {
    free(ref->arr);
}

void mutableInsertArrayDouble(struct ArrayRef* source, unsigned int index, double value) {
    struct ArrayRef ref = *source;

    if (index >= ref.size) {
        if (index >= ref.capacity) {
            unsigned int newCapacity = ((int) ref.capacity * 1.5) + 1;

            ref.arr = realloc(ref.arr, newCapacity * ref.itemSize);
            ref.capacity = newCapacity;
        }

        ref.size++;
    }

    double* dest = ref.arr + (ref.itemSize * index);
    *dest = value;
}
