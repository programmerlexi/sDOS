#include "functions.h"
void* memcpy(void* dst, void* src, uint16_t count) {
    uint8_t *u_dst = (uint8_t*)dst;
    uint8_t *u_src = (uint8_t*)src;
    for (uint16_t i = 0; i < count; i++) {
        u_dst[i] = u_src[i];
    }
    return dst;
}

void* memset(void* dst, int value, uint16_t count) {
    uint8_t *u_dst = (uint8_t*)dst;
    for (uint16_t i = 0; i < count; i++) {
        u_dst[i] = (uint8_t)value;
    }
    return dst;
}

bool memcmp(void* ptr1, void* ptr2, uint16_t count) {
    uint8_t *u_ptr1 = (uint8_t*)ptr1;
    uint8_t *u_ptr2 = (uint8_t*)ptr2;
    for (uint16_t i = 0; i < count; i++) {
        if (u_ptr1[i] != u_ptr2[i]) return false;
    }
    return true;
}