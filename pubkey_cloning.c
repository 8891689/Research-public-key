/* Apache License, Version 2.0
   Copyright [2025] [8891689]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   Author: 8891689 (https://github.com/8891689)
*/
/* pubkey_cloning.c 
 * gcc pubkey_cloning.c random.c bitrange.c sha256.c ripemd160.c base58.c -o p -pthread -lsecp256k1 -lgmp -Wall -Wextra -O3
 * gcc -static -DSECP256K1_STATIC pubkey_cloning.c random.c bitrange.c sha256.c ripemd160.c base58.c -o p.exe -pthread -march=native -lsecp256k1 -lgmp -Wall -Wextra -O3
 * p.exe -m a -n 10 -b 8 -v 0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
 * ./p 0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798 -m a -n 10 -b 8 -v
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <gmp.h>
#include <secp256k1.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#define getpid GetCurrentProcessId
#else
#include <unistd.h>
#endif

#include "random.h"
#include "bitrange.h"
#include "sha256.h"
#include "ripemd160.h"
#include "base58.h"

#define SHA256_DIGEST_SIZE 32
#define HASH160_SIZE 20

const char* SECP256K1_N_HEX = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141";

typedef enum { 
    MODE_PUBKEY, 
    MODE_HASH160, 
    MODE_ADDRESS 
} OutputMode;

typedef struct {
    int thread_id;
    long long start_count;
    long long end_count;
    secp256k1_context *ctx;
    secp256k1_pubkey pubkey_orig;
    mpz_t min_scalar;
    mpz_t max_scalar;
    mpz_t n;
    bool random_mode;
    bool verbose;
    OutputMode output_mode;
    FILE *output_fp;
    pthread_mutex_t *output_mutex;
    gmp_randstate_t randstate; 
} ThreadData;

bool hex_to_bytes(const char *hex, unsigned char *bytes, size_t hex_len, size_t *bytes_len) {
    if (hex_len % 2 != 0) return false;
    *bytes_len = hex_len / 2;
    if (*bytes_len == 0 && hex_len == 0) return true;
    for (size_t i = 0; i < *bytes_len; ++i) {
        if (sscanf(hex + 2 * i, "%2hhx", &bytes[i]) != 1) return false;
    }
    return true;
}

void print_bytes_hex(FILE *fp, const unsigned char *bytes, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        fprintf(fp, "%02x", bytes[i]);
    }
}

bool mpz_to_scalar32(mpz_t scalar_mpz, mpz_t n, unsigned char* scalar_bytes) {
    mpz_t temp_scalar;
    mpz_init(temp_scalar);
    mpz_mod(temp_scalar, scalar_mpz, n);
    memset(scalar_bytes, 0, 32);
    size_t needed_bytes;
    unsigned char* exported_bytes = mpz_export(NULL, &needed_bytes, 1, 1, 1, 0, temp_scalar);
    if (needed_bytes > 32) {
        fprintf(stderr, "Internal Error: Exported scalar (%zu bytes) is larger than 32 bytes.\n", needed_bytes);
        free(exported_bytes);
        mpz_clear(temp_scalar);
        return false;
    }
    memcpy(scalar_bytes + (32 - needed_bytes), exported_bytes, needed_bytes);
    free(exported_bytes);
    mpz_clear(temp_scalar);
    return true;
}

bool generate_random_scalar_in_range(mpz_t result, gmp_randstate_t state, mpz_t min, mpz_t max) {
    if (mpz_cmp(min, max) > 0) return false;
    
    mpz_t range_size;
    mpz_init(range_size);
    mpz_sub(range_size, max, min);
    mpz_add_ui(range_size, range_size, 1);
    
    mpz_urandomm(result, state, range_size);
    mpz_add(result, result, min);

    mpz_clear(range_size);
    return true;
}

void hash160(const unsigned char *data, size_t len, unsigned char *out_h160) {
    unsigned char sha256_hash[SHA256_DIGEST_SIZE];
    sha256(data, len, sha256_hash);
    ripemd160(sha256_hash, SHA256_DIGEST_SIZE, out_h160);
}

void pubkey_to_address(const unsigned char *pubkey, size_t pubkey_len, char **address_str) {
    unsigned char h160[HASH160_SIZE];
    hash160(pubkey, pubkey_len, h160);

    unsigned char payload[1 + HASH160_SIZE];
    payload[0] = 0x00; // P2PKH Mainnet version byte
    memcpy(payload + 1, h160, HASH160_SIZE);

    *address_str = base58_encode_check(payload, sizeof(payload));
}

// --- 程序主邏輯 ---
void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <public key hex> [options]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -m <mode>   Output mode: p (pubkey, default), h (hash160), a (address).\n");
    fprintf(stderr, "  -t <num>    Number of threads (default: 1).\n");
    fprintf(stderr, "  -n <count>  Total number of operations (default: 1, must be > 0).\n");
    fprintf(stderr, "  -o <file>   Write output to the specified file (default is to the console).\n");
    fprintf(stderr, "  -R          Generate a random scalar. If not specified, enters incremental mode.\n");
    fprintf(stderr, "  -b <bits>   Specifies a bit range for the scalar, e.g., -b 32 means [2^31, 2^32-1].\n");
    fprintf(stderr, "  -r <A:B>    Specifies a hexadecimal range for the scalar, e.g., -r 100:200.\n");
    fprintf(stderr, "  -v          Verbose: prints the scalar value (in hex) for each operation.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s 02... -n 1000 -t 4 -m a -R   # Generate 1000 random addresses using 4 threads.\n", prog_name);
    fprintf(stderr, "  %s 02... -n 100 -b 64 -v        # Incrementally generate 100 pubkeys from bit 64.\n", prog_name);
}

void *worker_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    mpz_t current_scalar_mpz, neg_current_scalar_mpz;
    mpz_inits(current_scalar_mpz, neg_current_scalar_mpz, NULL);

    unsigned char scalar_bytes[32];
    unsigned char neg_scalar_bytes[32];
    
    if (!data->random_mode) {
        mpz_add_ui(current_scalar_mpz, data->min_scalar, data->start_count);
    }

    for (long long i = data->start_count; i < data->end_count; ++i) {
        if (data->random_mode) {
            // 使用傳入的 randstate 生成隨機數
            if (!generate_random_scalar_in_range(current_scalar_mpz, data->randstate, data->min_scalar, data->max_scalar)) {
                fprintf(stderr, "Thread %d: Error generating random scalar.\n", data->thread_id);
                continue;
            }
        }
        
        if (!mpz_to_scalar32(current_scalar_mpz, data->n, scalar_bytes)) continue;
        
        mpz_neg(neg_current_scalar_mpz, current_scalar_mpz);
        if (!mpz_to_scalar32(neg_current_scalar_mpz, data->n, neg_scalar_bytes)) continue;

        // Process addition
        secp256k1_pubkey pubkey_plus = data->pubkey_orig;
        if (secp256k1_ec_pubkey_tweak_add(data->ctx, &pubkey_plus, scalar_bytes)) {
            unsigned char serialized_pubkey[33];
            size_t len = sizeof(serialized_pubkey);
            secp256k1_ec_pubkey_serialize(data->ctx, serialized_pubkey, &len, &pubkey_plus, SECP256K1_EC_COMPRESSED);

            pthread_mutex_lock(data->output_mutex);
            switch(data->output_mode) {
                case MODE_PUBKEY:
                    print_bytes_hex(data->output_fp, serialized_pubkey, len);
                    break;
                case MODE_HASH160: {
                    unsigned char h160[HASH160_SIZE];
                    hash160(serialized_pubkey, len, h160);
                    print_bytes_hex(data->output_fp, h160, HASH160_SIZE);
                    break;
                }
                case MODE_ADDRESS: {
                    char *addr_str = NULL;
                    pubkey_to_address(serialized_pubkey, len, &addr_str);
                    if(addr_str) { fprintf(data->output_fp, "%s", addr_str); free(addr_str); }
                    break;
                }
            }
            if (data->verbose) gmp_fprintf(data->output_fp, " = + 0x%Zx", current_scalar_mpz);
            fprintf(data->output_fp, "\n");
            pthread_mutex_unlock(data->output_mutex);
        }

        // Process subtraction
        secp256k1_pubkey pubkey_minus = data->pubkey_orig;
        if (secp256k1_ec_pubkey_tweak_add(data->ctx, &pubkey_minus, neg_scalar_bytes)) {
            unsigned char serialized_pubkey[33];
            size_t len = sizeof(serialized_pubkey);
            secp256k1_ec_pubkey_serialize(data->ctx, serialized_pubkey, &len, &pubkey_minus, SECP256K1_EC_COMPRESSED);

            pthread_mutex_lock(data->output_mutex);
            switch(data->output_mode) {
                case MODE_PUBKEY:
                    print_bytes_hex(data->output_fp, serialized_pubkey, len);
                    break;
                case MODE_HASH160: {
                    unsigned char h160[HASH160_SIZE];
                    hash160(serialized_pubkey, len, h160);
                    print_bytes_hex(data->output_fp, h160, HASH160_SIZE);
                    break;
                }
                case MODE_ADDRESS: {
                    char *addr_str = NULL;
                    pubkey_to_address(serialized_pubkey, len, &addr_str);
                    if(addr_str) { fprintf(data->output_fp, "%s", addr_str); free(addr_str); }
                    break;
                }
            }
            if (data->verbose) gmp_fprintf(data->output_fp, " = - 0x%Zx", current_scalar_mpz);
            fprintf(data->output_fp, "\n");
            pthread_mutex_unlock(data->output_mutex);
        }

        if (!data->random_mode) {
            mpz_add_ui(current_scalar_mpz, current_scalar_mpz, 1);
        }
    }

    mpz_clears(current_scalar_mpz, neg_current_scalar_mpz, NULL);
    return NULL;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    long long count = 1;
    int num_threads = 1;
    bool verbose = false;
    bool random_mode = false;
    const char *bitrange_param = NULL;
    const char *range_param = NULL;
    const char *output_filename = NULL;
    OutputMode output_mode = MODE_PUBKEY;

    mpz_t min_scalar, max_scalar, n;
    mpz_inits(min_scalar, max_scalar, n, NULL);
    mpz_set_str(n, SECP256K1_N_HEX, 16);

    int opt;
    while ((opt = getopt(argc, argv, "m:t:n:vRb:r:o:")) != -1) {
        switch (opt) {
            case 'm':
                if (strcmp(optarg, "p") == 0) output_mode = MODE_PUBKEY;
                else if (strcmp(optarg, "h") == 0) output_mode = MODE_HASH160;
                else if (strcmp(optarg, "a") == 0) output_mode = MODE_ADDRESS;
                else { fprintf(stderr, "Error: Invalid mode '%s'. Use p, h, or a.\n", optarg); return 1; }
                break;
            case 't':
                num_threads = atoi(optarg);
                if (num_threads <= 0) { fprintf(stderr, "Error: Number of threads must be > 0.\n"); return 1; }
                break;
            case 'n': count = atoll(optarg); if(count <= 0) { fprintf(stderr, "Error: -n count must be > 0.\n"); return 1; } break;
            case 'v': verbose = true; break;
            case 'R': random_mode = true; break;
            case 'b': bitrange_param = optarg; break;
            case 'r': range_param = optarg; break;
            case 'o': output_filename = optarg; break;
            default: print_usage(argv[0]); return 1;
        }
    }

    if (bitrange_param && range_param) {
        fprintf(stderr, "Error: Cannot specify both -b and -r.\n"); return 1;
    }
    if (random_mode) {
        if (bitrange_param) set_bitrange(bitrange_param, min_scalar, max_scalar);
        else if (range_param) set_range(range_param, min_scalar, max_scalar);
        else { mpz_set_ui(min_scalar, 1); mpz_sub_ui(max_scalar, n, 1); }
    } else {
        if (!bitrange_param && !range_param) {
            mpz_set_ui(min_scalar, 1);
            mpz_add_ui(max_scalar, min_scalar, count - 1);
        } else {
             if (bitrange_param) set_bitrange(bitrange_param, min_scalar, max_scalar);
             else if (range_param) set_range(range_param, min_scalar, max_scalar);
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Public key hex string is missing.\n"); return 1;
    }
    const char *pubkey_hex = argv[optind];
    size_t pubkey_hex_len = strlen(pubkey_hex);
    unsigned char pubkey_bytes[65];
    size_t pubkey_bytes_len;
    if (!hex_to_bytes(pubkey_hex, pubkey_bytes, pubkey_hex_len, &pubkey_bytes_len) || (pubkey_bytes_len != 33 && pubkey_bytes_len != 65)) {
        fprintf(stderr, "Error: Invalid public key hex string or length.\n"); return 1;
    }

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    secp256k1_pubkey pubkey_orig;
    if (!secp256k1_ec_pubkey_parse(ctx, &pubkey_orig, pubkey_bytes, pubkey_bytes_len)) {
        fprintf(stderr, "Error: Failed to parse public key.\n");
        secp256k1_context_destroy(ctx);
        return 1;
    }
    
    FILE *output_fp = stdout;
    if (output_filename) {
        output_fp = fopen(output_filename, "w");
        if (!output_fp) {
            fprintf(stderr, "Error: Could not open output file '%s'.\n", output_filename);
            secp256k1_context_destroy(ctx);
            return 1;
        }
    }

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = malloc(num_threads * sizeof(ThreadData));
    pthread_mutex_t output_mutex;
    pthread_mutex_init(&output_mutex, NULL);
    
    long long count_per_thread = count / num_threads;
    long long remainder = count % num_threads;
    long long current_start = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start_count = current_start;
        long long work_for_this_thread = count_per_thread + (i < remainder ? 1 : 0);
        thread_data[i].end_count = current_start + work_for_this_thread;
        current_start = thread_data[i].end_count;

        thread_data[i].ctx = ctx;
        thread_data[i].pubkey_orig = pubkey_orig;
        mpz_init_set(thread_data[i].min_scalar, min_scalar);
        mpz_init_set(thread_data[i].max_scalar, max_scalar);
        mpz_init_set(thread_data[i].n, n);
        thread_data[i].random_mode = random_mode;
        thread_data[i].verbose = verbose;
        thread_data[i].output_mode = output_mode;
        thread_data[i].output_fp = output_fp;
        thread_data[i].output_mutex = &output_mutex;

        // 初始化並為每個執行緒的隨機狀態播種
        gmp_randinit_default(thread_data[i].randstate);
        unsigned long seed = time(NULL) ^ (unsigned long)getpid() ^ (unsigned long)(i + 1);
        gmp_randseed_ui(thread_data[i].randstate, seed);

        pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        // 清理執行緒數據和隨機狀態
        mpz_clears(thread_data[i].min_scalar, thread_data[i].max_scalar, thread_data[i].n, NULL);
        gmp_randclear(thread_data[i].randstate);
    }
    
    if (verbose) {
        unsigned char serialized_pubkey_orig[33];
        size_t len = sizeof(serialized_pubkey_orig);
        secp256k1_ec_pubkey_serialize(ctx, serialized_pubkey_orig, &len, &pubkey_orig, SECP256K1_EC_COMPRESSED);
        
        pthread_mutex_lock(&output_mutex);
        switch(output_mode) {
             case MODE_PUBKEY:
                 print_bytes_hex(output_fp, serialized_pubkey_orig, len);
                 break;
             case MODE_HASH160: {
                 unsigned char h160[HASH160_SIZE];
                 hash160(serialized_pubkey_orig, len, h160);
                 print_bytes_hex(output_fp, h160, HASH160_SIZE);
                 break;
             }
             case MODE_ADDRESS: {
                 char *addr_str = NULL;
                 pubkey_to_address(serialized_pubkey_orig, len, &addr_str);
                 if(addr_str) { fprintf(output_fp, "%s", addr_str); free(addr_str); }
                 break;
             }
        }
        fprintf(output_fp, " = original\n");
        pthread_mutex_unlock(&output_mutex);
    }

    pthread_mutex_destroy(&output_mutex);
    free(threads);
    free(thread_data);
    secp256k1_context_destroy(ctx);
    mpz_clears(min_scalar, max_scalar, n, NULL);
    if (output_fp != stdout) {
        fclose(output_fp);
    }

    return 0;
}
