#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define LEFTROTATE(x, n) ((x >> (32 - n)) | (x << n))
#define LITTLEENDIAN(x) (((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24))

struct encoding {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
};

void md5(struct encoding *hash, FILE *stream, uint8_t buf[64]) {
    uint32_t s[64] = {
        7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
        5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
        4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
        6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
    };

    uint32_t K[64] = { 
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 
    };

    uint32_t A = hash->A;
    uint32_t B = hash->B;
    uint32_t C = hash->C;
    uint32_t D = hash->D;

    for (int i = 0; i < 64; i++) {
        uint32_t F, g;

        if (i < 16) {
            F = (B & C) | ((~B) & D);
            g = i;
        } else if (i < 32) {
            F = (D & B) | ((~D) & C);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            F = B ^ C ^ D;
            g = (3 * i + 5) % 16;
        } else if (i < 64) {
            F = C ^ (B | (~D));
            g = (7 * i) % 16;
        }

        uint32_t temp = D;
        D = C;
        C = B;
        uint32_t val = 0;
        for (int k = 0; k < 4; k++) {
            val += buf[4 * g + k] << (k * 8);
        }

        F = F + A + K[i] + val;
        B = B + LEFTROTATE(F, s[i]);
        A = temp;
    }

    hash->A += A;
    hash->B += B;
    hash->C += C;
    hash->D += D;
}

int main(int argc, char *argv[]) {
    FILE *stream;
    uint8_t buf[64];

    struct encoding hash = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};

    if (argc == 1) {
        stream = stdin;
    } else{
        stream = fopen(argv[1], "r");
    }

    uint64_t message_length = 0;
    char byte;
    int flag = true;
    int overflow = false;

    while (flag) {
        for (int i = 0; i < 64; i++) {
            byte = fgetc(stream);

            if (byte == EOF) {
                if (flag) {
                    flag = false;
                    buf[i] = 0x80;
                    if (i >= 56) {
                        overflow = true;
                    }
                } else {
                    if (overflow) {
                        buf[i] = 0;
                    } else {
                        if (i >= 56) {
                            buf[i] = ((message_length) & ((uint64_t)0xFF << (((i - 56) * 8)))) >> ((i - 56) * 8);
                        } else {
                            buf[i] = 0;
                        }
                    }
                }
            } else {
                buf[i] = byte;
                message_length += 8;
            }
        }

        md5(&hash, stream, buf);
    }

    if (overflow) {
        for (int i = 0; i < 56; i++) {
            buf[i] = 0;
        }

        for (int i = 56; i < 64; i++) {
            buf[i] = message_length & (0xFF << (63 - i));
        }

        md5(&hash, stream, buf);
    }

    printf("%x%x%x%x\n", LITTLEENDIAN(hash.A), LITTLEENDIAN(hash.B), LITTLEENDIAN(hash.C), LITTLEENDIAN(hash.D));

    return 0;
}