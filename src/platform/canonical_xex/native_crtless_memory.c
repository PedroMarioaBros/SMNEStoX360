/* Small freestanding memory routines for the canonic NES core.
 * Keep the Xbox title CRT-free; this file does not contain a startup
 * routine, heap allocator, printf, filesystem or any platform API.
 */
#include <stddef.h>
void *memset(void *dest, int byte, size_t n) {
    unsigned char *p=(unsigned char *)dest;
    size_t i;
    for(i=0u;i<n;i++) p[i]=(unsigned char)byte;
    return dest;
}
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d=(unsigned char *)dest;
    const unsigned char *s=(const unsigned char *)src;
    size_t i;
    for(i=0u;i<n;i++) d[i]=s[i];
    return dest;
}
