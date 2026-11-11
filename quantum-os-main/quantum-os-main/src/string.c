/* Minimal freestanding string and memory routines (no libc). */
#include "kernel.h"

size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (u8)*a - (u8)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && *a == *b) { a++; b++; n--; }
    return n ? (u8)*a - (u8)*b : 0;
}

char *strncpy(char *d, const char *s, size_t n) {
    size_t i = 0;
    for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
    return d;
}

void *memset(void *d, int c, size_t n) {
    u8 *p = d;
    while (n--) *p++ = (u8)c;
    return d;
}

void *memcpy(void *d, const void *s, size_t n) {
    u8 *a = d; const u8 *b = s;
    while (n--) *a++ = *b++;
    return d;
}

void *memmove(void *d, const void *s, size_t n) {
    u8 *a = d; const u8 *b = s;
    if ((uintptr_t)a < (uintptr_t)b) while (n--) *a++ = *b++;
    else { a += n; b += n; while (n--) *--a = *--b; }
    return d;
}

int atoi(const char *s) {
    if (!s) return 0;
    int negative = 0;
    if (*s == '-' || *s == '+') {
        negative = (*s == '-');
        s++;
    }
    if (*s < '0' || *s > '9') return 0;

    u32 limit = negative ? 0x80000000U : 0x7FFFFFFFU;
    u32 value = 0;
    while (*s >= '0' && *s <= '9') {
        u32 digit = (u32)(*s++ - '0');
        if (value > (limit - digit) / 10U)
            return negative ? (-2147483647 - 1) : 2147483647;
        value = value * 10U + digit;
    }

    if (negative) {
        if (value == 0x80000000U) return (-2147483647 - 1);
        return -(int)value;
    }
    return (int)value;
}
