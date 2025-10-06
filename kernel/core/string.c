/**
 * QuantumOS String Library Implementation
 * 
 * Implementation of standard C string functions for freestanding kernel.
 */

#include "string.h"

// String length functions
size_t strlen(const char* str) {
    const char* s = str;
    while (*s) s++;
    return s - str;
}

size_t strnlen(const char* str, size_t maxlen) {
    const char* s = str;
    while (maxlen-- && *s) s++;
    return s - str;
}

// String comparison
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    if (n == 0) return 0;
    
    while (--n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strcasecmp(const char* str1, const char* str2) {
    while (*str1 && (tolower(*str1) == tolower(*str2))) {
        str1++;
        str2++;
    }
    return tolower(*(unsigned char*)str1) - tolower(*(unsigned char*)str2);
}

int strncasecmp(const char* str1, const char* str2, size_t n) {
    if (n == 0) return 0;
    
    while (--n && *str1 && (tolower(*str1) == tolower(*str2))) {
        str1++;
        str2++;
    }
    return tolower(*(unsigned char*)str1) - tolower(*(unsigned char*)str2);
}

// String copying
char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    
    while (n && (*dest = *src)) {
        dest++;
        src++;
        n--;
    }
    
    // Pad with nulls if needed
    while (n--) {
        *dest++ = '\0';
    }
    
    return orig_dest;
}

// String concatenation
char* strcat(char* dest, const char* src) {
    char* orig_dest = dest;
    
    // Find end of dest
    while (*dest) dest++;
    
    // Append src
    while ((*dest++ = *src++));
    
    return orig_dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    
    // Find end of dest
    while (*dest) dest++;
    
    // Append up to n characters from src
    while (n-- && (*dest = *src)) {
        dest++;
        src++;
    }
    
    *dest = '\0';
    return orig_dest;
}

// String searching
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

char* strrchr(const char* str, int c) {
    const char* last = NULL;
    
    while (*str) {
        if (*str == c) last = str;
        str++;
    }
    
    if (c == '\0') return (char*)str;
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') return (char*)haystack;
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (*n == '\0') return (char*)haystack;
        haystack++;
    }
    
    return NULL;
}

char* strpbrk(const char* str, const char* accept) {
    while (*str) {
        const char* a = accept;
        while (*a) {
            if (*str == *a) return (char*)str;
            a++;
        }
        str++;
    }
    return NULL;
}

size_t strspn(const char* str, const char* accept) {
    const char* s = str;
    
    while (*s) {
        const char* a = accept;
        while (*a) {
            if (*s == *a) break;
            a++;
        }
        if (*a == '\0') break;
        s++;
    }
    
    return s - str;
}

size_t strcspn(const char* str, const char* reject) {
    const char* s = str;
    
    while (*s) {
        const char* r = reject;
        while (*r) {
            if (*s == *r) return s - str;
            r++;
        }
        s++;
    }
    
    return s - str;
}

// String tokenization (simple implementation)
static char* strtok_state = NULL;

char* strtok(char* str, const char* delim) {
    return strtok_r(str, delim, &strtok_state);
}

char* strtok_r(char* str, const char* delim, char** saveptr) {
    if (str) *saveptr = str;
    if (!*saveptr) return NULL;
    
    // Skip leading delimiters
    *saveptr += strspn(*saveptr, delim);
    if (**saveptr == '\0') return NULL;
    
    // Find end of token
    str = *saveptr;
    *saveptr += strcspn(*saveptr, delim);
    
    if (**saveptr) {
        **saveptr = '\0';
        (*saveptr)++;
    } else {
        *saveptr = NULL;
    }
    
    return str;
}

// Memory functions
void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    while (num--) {
        *d++ = *s++;
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Forward copy
        while (num--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        // Backward copy
        d += num;
        s += num;
        while (num--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    while (num--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

void* memchr(const void* ptr, int value, size_t num) {
    const unsigned char* p = (const unsigned char*)ptr;
    unsigned char c = (unsigned char)value;
    
    while (num--) {
        if (*p == c) return (void*)p;
        p++;
    }
    
    return NULL;
}

// Utility memory functions
void* memzero(void* ptr, size_t size) {
    return memset(ptr, 0, size);
}

bool memeq(const void* ptr1, const void* ptr2, size_t size) {
    return memcmp(ptr1, ptr2, size) == 0;
}

// String to number conversion
int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (isspace(*str)) str++;
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

long atol(const char* str) {
    long result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (isspace(*str)) str++;
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

long long atoll(const char* str) {
    long long result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (isspace(*str)) str++;
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

// Number to string conversion
char* itoa(int value, char* str, int base) {
    if (base < 2 || base > 36) return str;
    
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // Handle negative numbers for base 10
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1 = ptr;
    }
    
    // Convert to string (reversed)
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    // Reverse string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

// String manipulation utilities
void strrev(char* str) {
    if (!str) return;
    
    size_t len = strlen(str);
    char* start = str;
    char* end = str + len - 1;
    
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

char* strtrim(char* str) {
    if (!str) return str;
    
    // Trim leading whitespace
    while (isspace(*str)) str++;
    
    if (*str == '\0') return str;
    
    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    
    end[1] = '\0';
    
    return str;
}

char* strlwr(char* str) {
    char* orig = str;
    while (*str) {
        *str = tolower(*str);
        str++;
    }
    return orig;
}

char* strupr(char* str) {
    char* orig = str;
    while (*str) {
        *str = toupper(*str);
        str++;
    }
    return orig;
}

// Safe string functions
size_t strlcpy(char* dest, const char* src, size_t size) {
    size_t src_len = strlen(src);
    
    if (size > 0) {
        size_t copy_len = (src_len < size - 1) ? src_len : size - 1;
        memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }
    
    return src_len;
}

size_t strlcat(char* dest, const char* src, size_t size) {
    size_t dest_len = strnlen(dest, size);
    size_t src_len = strlen(src);
    
    if (dest_len < size) {
        size_t copy_len = size - dest_len - 1;
        if (src_len < copy_len) copy_len = src_len;
        
        memcpy(dest + dest_len, src, copy_len);
        dest[dest_len + copy_len] = '\0';
    }
    
    return dest_len + src_len;
}

// Kernel utility functions
void hexdump(const void* data, size_t size) {
    // TODO: Complete hexdump implementation when graphics system is available
    // For now, just suppress unused parameter warnings
    (void)data;
    (void)size;
}

char* bin2hex(const void* data, size_t size, char* buffer) {
    const unsigned char* bytes = (const unsigned char*)data;
    char* ptr = buffer;
    
    for (size_t i = 0; i < size; i++) {
        unsigned char byte = bytes[i];
        *ptr++ = "0123456789abcdef"[byte >> 4];
        *ptr++ = "0123456789abcdef"[byte & 0x0F];
    }
    
    *ptr = '\0';
    return buffer;
}

int hex2bin(const char* hex, void* buffer, size_t max_size) {
    unsigned char* bytes = (unsigned char*)buffer;
    size_t hex_len = strlen(hex);
    
    if (hex_len % 2 != 0) return -1; // Invalid hex string
    
    size_t byte_count = hex_len / 2;
    if (byte_count > max_size) return -1; // Buffer too small
    
    for (size_t i = 0; i < byte_count; i++) {
        char high = hex[i * 2];
        char low = hex[i * 2 + 1];
        
        if (!isxdigit(high) || !isxdigit(low)) return -1;
        
        unsigned char high_val = isdigit(high) ? high - '0' : tolower(high) - 'a' + 10;
        unsigned char low_val = isdigit(low) ? low - '0' : tolower(low) - 'a' + 10;
        
        bytes[i] = (high_val << 4) | low_val;
    }
    
    return byte_count;
}

bool is_valid_utf8(const char* str) {
    // Simple UTF-8 validation - can be enhanced
    const unsigned char* s = (const unsigned char*)str;
    
    while (*s) {
        if (*s < 0x80) {
            // ASCII
            s++;
        } else if ((*s >> 5) == 0x06) {
            // 110xxxxx - 2 byte sequence
            if ((s[1] & 0xC0) != 0x80) return false;
            s += 2;
        } else if ((*s >> 4) == 0x0E) {
            // 1110xxxx - 3 byte sequence
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return false;
            s += 3;
        } else if ((*s >> 3) == 0x1E) {
            // 11110xxx - 4 byte sequence
            if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return false;
            s += 4;
        } else {
            return false;
        }
    }
    
    return true;
}

// Basic sprintf implementation for kernel use
int sprintf(char* buffer, const char* format, ...) {
    // Simple implementation - handles %s, %d, %x, %c
    char* dest = buffer;
    const char* src = format;
    
    // This is a placeholder - for full implementation, we'd need va_list support
    // For now, just copy the format string
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    
    return dest - buffer;
}

// Basic snprintf implementation for kernel use  
int snprintf(char* buffer, size_t size, const char* format, ...) {
    if (size == 0) return 0;
    
    // Simple implementation - just copy format for now
    size_t len = strlen(format);
    if (len >= size) len = size - 1;
    
    strncpy(buffer, format, len);
    buffer[len] = '\0';
    
    return len;
}

// String search function already defined above