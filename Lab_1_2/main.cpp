//
// Created by yrustt on 3/30/18.
//

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <random>
#include <limits>

#define MAX_VECTOR_SIZE 125 * 4
#define MAX_MATRIX_SIZE MAX_VECTOR_SIZE * 4000


uint32_t memory[3 * MAX_MATRIX_SIZE + 4 * MAX_VECTOR_SIZE];
uint32_t *a = memory;
uint32_t *b = memory + MAX_MATRIX_SIZE;
uint32_t *c = b + MAX_MATRIX_SIZE;
uint32_t *v1 = c + MAX_MATRIX_SIZE;
uint32_t *v2 = v1 + MAX_VECTOR_SIZE;
uint32_t *v3 = v2 + MAX_VECTOR_SIZE;
uint32_t *v = v3 + MAX_VECTOR_SIZE;
char buff[1000];

uint32_t size,
         sls_for_T=8,
         sls_for_line,
         line_size;


uint32_t _round(const uint32_t a, const uint32_t b) {
    return a / b + ((a % b) ? 1 : 0);
}


uint32_t _get_code(char s) {
    return (uint32_t) (((s >= '0') && (s <= '9')) ? (s - '0') : (s - 'A' + 10));
}


uint32_t _get_number(uint32_t *a) {
    uint32_t res = 0;
    for (uint32_t i = 0; i < 32; i += 4) {
        res |= (a[i / 4] << (32 - i - 4));
    }
    return res;
}


void _random_vector(uint32_t *ar, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        ar[i] = 0;
        uint32_t x = 0;
        while (!ar[i]) {
            x = rand() & 0xff;
            x |= (rand() & 0xff) << 8;
            x |= (rand() & 0xff) << 16;
            x |= (rand() & 0xff) << 24;
            ar[i] = x;
        }
    }
}


void create_matrix(uint32_t size_) {
    size = size_;
    sls_for_line = _round(size, 4);
    line_size = _round(sls_for_line, sls_for_T);
}


void init_matrix(uint32_t *m, FILE* file) {
    for (uint32_t i = 0; i < size; ++i) {
        fread(buff, sls_for_line + 1, 1, file);

        uint32_t a[sls_for_T];

        for (uint32_t j = 0; j < sls_for_line; j += sls_for_T) {
            for (uint32_t k = 0; k < sls_for_T; ++k) {
                a[k] = (j + k < sls_for_line)? _get_code(buff[j + k]) : 0;
            }
            m[i * line_size + j / sls_for_T] = _get_number(a);
        }
    }
}


void mul(uint32_t *m, uint32_t *v, uint32_t *res, uint32_t const *bits) {
    for (uint32_t i = 0; i < size; i += 32) {
        res[i >> 5] = 0;
        uint32_t tmp = 0;
        for (uint32_t j = 0; j < 32; ++j) {
            if ((i + j) >= size) break;

            uint32_t b = 0;
            for (uint32_t k = 0; k < line_size; ++k) {
                b ^= m[(i + j) * line_size + k] & v[k];
            }

            uint32_t count = 0;
            for (uint32_t k = 0; k < 32; ++k) {
                if (b & bits[k]) {
                    count++;
                }
            }

            if (count % 2) {
                tmp |= bits[31 - j];
            }
        }
        res[i >> 5] = tmp;
    }
}

int32_t check(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *v, uint32_t const *bits) {
    mul(b, v, v1, bits);
    mul(a, v1, v2, bits);
    mul(c, v, v3, bits);

    uint32_t i = 0;
    for (i = 0; i < line_size; ++i) {
        if (v2[i] != v3[i]) break;
    }

    if (i == line_size) {
        return -1;
    } else {
        for (uint32_t j = 0; j < 32; ++j) {
            int32_t i_ = i * 32 + 31 - j;
            if ((i_ < size) && ((v2[i] & bits[j]) != (v3[i] & bits[j]))) {
                return i_ + 1;
            }

        }
    }
    return -1;
}

int32_t check2(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t const *bits, int32_t idx) {
    uint32_t *line_a = a + line_size * (idx - 1);
    uint32_t *line_c = c + line_size * (idx - 1);
    for (uint32_t i = 0; i < line_size; ++i) {
        v1[i] = 0;
    }
    for (uint32_t i = 0; i < size; ++i) {
        uint32_t i_ = i >> 5;
        uint32_t j_ = 31 - (i % 32);

        if (line_a[i_] & bits[j_]) {
            for (uint32_t k = 0; k < line_size; ++k) {
                v1[k] ^= b[i * line_size + k];
            }
        }
    }
    for (uint32_t i = 0; i < size; ++i) {
        uint32_t i_ = i >> 5;
        uint32_t j_ = 31 - (i % 32);
        if ((line_c[i_] & bits[j_]) != (v1[i_] & bits[j_])) {
            return i_ * 32 + 32 - j_;
        }
    }
    return -1;
}


int main() {
    srand(time(NULL));
    FILE *file = fopen("element.in", "r");
    FILE *out = fopen("element.out", "w");
    char YES[] = "Yes\n";
    char NO[] = "No\n";

    unsigned int n;
    uint32_t bits[32];
    for (uint32_t i = 0; i < 32; ++i) {
        bits[i] = 1 << i;
    }

    while (true) {
        fscanf(file, "%u", &n);

        if (n == 0) {
            break;
        }

        getc(file);

        create_matrix(n);

        init_matrix(a, file);
        init_matrix(b, file);
        init_matrix(c, file);

        uint32_t i = 0;
        int32_t idx = -1;
        for (i = 0; i < 22; ++i) {
            _random_vector(v, line_size);
            idx = check(a, b, c, v, bits);
            if (idx != -1) {
                int32_t jdx = check2(a, b, c, bits, idx);
                fprintf(out, "Yes\n%d %d\n", idx, jdx);
                break;
            }
        }
        if (idx == -1) {
            fwrite(NO, sizeof(char), sizeof(NO) - 1, out);
        }
    }

    fclose(out);
    fclose(file);
    return 0;
}