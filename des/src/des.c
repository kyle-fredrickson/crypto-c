#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "des.h"

const uint64_t mask32 = (1LL << 32) - 1LL;
const uint64_t mask48 = (1LL << 48) - 1LL;

const size_t PERMUTED_CHOICE_1[56] = {
    56, 48, 40, 32, 24, 16,  8,
     0, 57, 49, 41, 33, 25, 17,
     9,  1, 58, 50, 42, 34, 26,
    18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,
     6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,
    20, 12,  4, 27, 19, 11,  3
};

const size_t PERMUTED_CHOICE_2[48] = {
    13, 16, 10, 23,  0,  4,
     2, 27, 14,  5, 20,  9,
    22, 18, 11,  3, 25,  7,
    15,  6, 26, 19, 12,  1,
    40, 51, 30, 36, 46, 54,
    29, 39, 50, 44, 32, 47,
    43, 48, 38, 55, 33, 52,
    45, 41, 49, 35, 28, 31
};

const size_t LEFT_SHIFTS[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

const size_t INITIAL_PERMUTATION[64] = {
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7,
    56, 48, 40, 32, 24, 16,  8,  0,
    58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6
};

const size_t INVERSE_INITIAL_PERMUTATION[64] = {
    39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30,
    37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28,
    35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26,
    33,  1, 41,  9, 49, 17, 57, 25,
    32,  0, 40,  8, 48, 16, 56, 24
};

const size_t EXPANSION_TABLE[48] = {
    31,  0,  1,  2,  3,  4,
     3,  4,  5,  6,  7,  8,
     7,  8,  9, 10, 11, 12,
    11, 12, 13, 14, 15, 16,
    15, 16, 17, 18, 19, 20,
    19, 20, 21, 22, 23, 24,
    23, 24, 25, 26, 27, 28,
    27, 28, 29, 30, 31,  0
};

const size_t SBOXES[8][64] = {
    // S1
    {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
      0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
      4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
     15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13},

    // S2
    {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
      3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
      0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
     13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9},

    // S3
    {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
     13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
     13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
      1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12},

    // S4
    { 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
     13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
     10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
      3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14},

    // S5
    { 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
     14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
      4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
     11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3},

    // S6
    {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
     10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
      9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
      4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13},

    // S7
    { 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
     13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
      1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
      6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12},

    // S8
    {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
      1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
      7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
      2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11},
};

const size_t CIPHER_PERMUTATION[32] = {
    15,  6, 19, 20, 28, 11,
    27, 16,  0, 14, 22, 25,
     4, 17, 30,  9,  1,  7,
    23, 13, 31, 26,  2,  8,
    18, 12, 29,  5, 21, 10,
     3, 24
};

void print_64bits(uint64_t n)
{
    printf("0b");
    for(size_t i = 0; i < 64; i++)
    {
        if ((i % 4) == 0 && i != 0)
            printf(" ");

        printf("%lld", (n & (1LL << (63 - i))) >> (63 - i));
    }
    printf("\n");
}

uint64_t rotl(uint64_t block, size_t block_length, uint8_t rotation)
{
    uint64_t rotated_block = ((block << rotation) | (block >> (block_length - rotation))) & ((1LL << block_length) - 1);
    return rotated_block;
}

uint64_t* get_blocks(uint64_t input, size_t input_length, uint8_t block_size)
{
    size_t num_blocks = input_length / block_size;
    uint64_t* blocks = malloc(num_blocks * sizeof(uint64_t));

    uint64_t mask = (1LL << block_size) - 1LL;

    for (size_t i = 0; i < num_blocks; i++)
    {
        size_t shift = (num_blocks - 1 - i) * block_size;
        blocks[i] = (input  & (mask << shift)) >> shift;
    }

    return blocks;
}

uint64_t combine_blocks(uint64_t* blocks, size_t num_blocks, uint8_t block_length)
{
    uint64_t block = 0LL;
    for (size_t i = 0; i < num_blocks; i++)
    {
        block = (block << block_length) | blocks[i];
    }

    return block;
}

uint64_t add_parity_bit(uint64_t block)
{
    return 0;
}

uint64_t generate_key()
{
    return 0;
}

uint64_t permute(uint64_t block, const size_t* permutation, uint8_t block_length, uint8_t permutation_length)
{
    uint64_t permuted_block = 0LL;

    for (size_t i = 0; i < permutation_length; i++)
    {
        size_t bit = (block & (1LL << (block_length - permutation[i] - 1))) >> (block_length - permutation[i] - 1);
        permuted_block = (permuted_block << 1LL) | bit;
    }


    return permuted_block;
}

uint64_t initial_permutation(uint64_t block)
{
    return permute(block, INITIAL_PERMUTATION, 64, 64);
}

uint64_t final_permutation(uint64_t block)
{
    return permute(block, INVERSE_INITIAL_PERMUTATION, 64, 64);
}

uint64_t expand(uint64_t block)
{
    return permute(block, EXPANSION_TABLE, 32, 48);
}

uint64_t cipher_permutation(uint64_t block)
{
    return permute(block, CIPHER_PERMUTATION, 32, 32);
}

uint64_t sbox(uint64_t block, size_t box)
{
    size_t row = (block & 0x20) >> 4 | (block & 0x1);
    size_t col = (block & 0x1E) >> 1;

    return SBOXES[box][16 * row + col];
}

uint64_t cipher_function(uint64_t block, uint64_t key)
{
    uint64_t expanded_block = expand(block) ^ key;
    uint64_t* blocks = get_blocks(expanded_block, 48, 6);

    uint64_t sbox_result = 0LL;
    for (size_t i = 0; i < 8; i++)
    {
        sbox_result = ((sbox_result << 4) | sbox(blocks[i], i));
    }

    uint64_t output = cipher_permutation(sbox_result);

    return output;
}

uint64_t* double_round(uint64_t left0, uint64_t right0, uint64_t key0, uint64_t key1)
{
    uint64_t right1 = left0 ^ cipher_function(right0, key0);
    uint64_t right2 = right0 ^ cipher_function(right1, key1);

    uint64_t* left_right = malloc(2 * sizeof(uint64_t));
    left_right[0] = right1;
    left_right[1] = right2;

    return left_right;
}

uint64_t cipher(uint64_t block, uint64_t* keys)
{
    block = initial_permutation(block);
    uint64_t* left_right = get_blocks(block, 64, 32);

    for (size_t i = 0; i < 14; i+=2)
        left_right = double_round(left_right[0], left_right[1], keys[i], keys[i + 1]);

    uint64_t final_left = left_right[0] ^ cipher_function(left_right[1], keys[14]);
    uint64_t final_right = left_right[1] ^ cipher_function(final_left, keys[15]);

    left_right[0] = final_right;
    left_right[1] = final_left;

    uint64_t output = combine_blocks(left_right, 2, 32);

    free(left_right);
    free(keys);

    return final_permutation(output);
}

uint64_t* key_schedule(uint64_t key)
{
    uint64_t* keys = malloc(16 * sizeof(uint64_t));
    key = permute(key, PERMUTED_CHOICE_1, 64, 56);
    uint64_t* cd = get_blocks(key, 56, 28);
    uint64_t c = cd[0];
    uint64_t d = cd[1];
    free(cd);

    for (size_t i = 0; i < 16; i++)
    {
        c = rotl(c, 28, LEFT_SHIFTS[i]);
        d = rotl(d, 28, LEFT_SHIFTS[i]);
        uint64_t rot_cd[2] = {c, d};
        keys[i] = permute(combine_blocks(rot_cd, 2, 28), PERMUTED_CHOICE_2, 56, 48);
    }

    return keys;
}

uint64_t encrypt(uint64_t block, uint64_t key)
{
    return cipher(block, key_schedule(key));
}

uint64_t* reverse(uint64_t* arr, size_t length)
{
    uint64_t* arr_reverse = malloc(length * sizeof(uint64_t));

    size_t i = 0;
    size_t j = length - 1;

    while (i <= j)
    {
        arr_reverse[i] = arr[j];
        if (i != j)
            arr_reverse[j] = arr[i];

        i++; j--;
    }

    return arr_reverse;
}

uint64_t decrypt(uint64_t block, uint64_t key)
{
    uint64_t* keys = key_schedule(key);
    uint64_t* keys_reversed = reverse(keys, 16);
    free(keys);

    return cipher(block, keys_reversed);
}

int main(int argc, char *argv[])
{
    // Verify args
    if (argc != 4)
    {
        printf("des: [e | d] [64b hex key] [64b hex block]\n");
        exit(0);
    }

    char* choice = argv[1];
    char* strkey = argv[2];
    char* strblock = argv[3];


    // Verify length
    if (strlen(strkey) != 16 || strlen(strblock) != 16)
    {
        printf("Error: incorrect key or block length.\n");
        exit(0);
    }

    char* rkey;
    char* rblock;
    // Verify hex
    uint64_t key = (uint64_t) strtoul(strkey, &rkey, 16);
    uint64_t block = (uint64_t) strtoul(strblock, &rblock, 16);

    if (strcmp(choice, "e") == 0)
    {
        uint64_t encrypted = encrypt(block, key);
        printf("%016lx\n", encrypted);
    }
    else if (strcmp(choice, "d") == 0)
    {
        uint64_t decrypted = decrypt (block, key);
        printf("%016lx\n", decrypted);
    }
    else
    {
        printf("Error: bad choice. Enter [e | d].\n");
        exit(0);
    }

    return 0;
}
