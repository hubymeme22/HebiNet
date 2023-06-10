/*
    MIT License
    Copyright (c) 2023 HueHueberry

    - A simple library for collatz conjecture for encryption algorithm
    - Works on any files, binaries and any ASCII texts, as long as it
    can be read byte by byte.
*/

#include <stdlib.h>
#include <stdio.h>
#include "List.h"

#ifndef COLLENC_H
#define COLLENC_H

char* key = 0;
int keySize = 0;
char shifts = 0;
struct list* colz = NULL;

////////////////////////
//  helper functions  //
////////////////////////
int setKeySeed(char* key, int size);

char ROTR(unsigned char a, unsigned char s);
char ROTL(unsigned char a, unsigned char s);

///////////////////////
//  Encryption part  //
///////////////////////
char singleEncrypt(char a, char k, char collatzFactor, char s);
char singleDecrypt(char e, char k, char collatzFactor, char s);

char* bufferEncrypt(char* buffer, long bufferSize);
char* bufferDecrypt(char* buffer, long bufferSize);

void setKey(char* _key, int size);

////////////////////
//  redefinition  //
////////////////////
// added a function for one-time key setting
void setKey(char* _key, int size) {
    int seed = setKeySeed(_key, size);
    key = _key; keySize = size;

    colz = genCollatzSequence(seed);
    CACHE_CURR->next = colz;

    shifts = (seed % 8);
}

// char a would be the charater that will be shifted
// char s will be the number of shifts that will be done
// the shift will be any number from 0-7 since, the rest
// will only affect the same way
char ROTR(unsigned char a, unsigned char s) {
    s = s % 8;
    return (a >> s) | (a << (8 - s));
}

char ROTL(unsigned char a, unsigned char s) {
    s = s % 8;
    return (a << s) | (a >> (8 - s));
}

// generate the seed from the key
int setKeySeed(char* key, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += key[i] & 0xFF;
    return sum;
}

// the collatz factor will be any number from 0x00 - 0xFF
// this will serve as additional layer for encryption and avalanche effect
// a - character to be encrypted
// k - character key
// s - number of shifts
char singleEncrypt(char a, char k, char collatzFactor, char s) {
    return ROTL(ROTR((a ^ k), s) ^ ((collatzFactor * k) & 0xFF), s);
}

char singleDecrypt(char e, char k, char collatzFactor, char s) {
    return ROTL(ROTR(e, s) ^ ((collatzFactor * k) & 0xFF), s) ^ k;
}

char* bufferEncrypt(char* buffer, long bufferSize) {
    char* bufferCopy = (char*)malloc(sizeof(char) * bufferSize);
    for (long i = 0; i < bufferSize; i++) {
        bufferCopy[i] = singleEncrypt(buffer[i], key[i % keySize], (colz->data & 0xFF), shifts);
        colz = colz->next;
    }

    return bufferCopy;
}

char* bufferDecrypt(char* buffer, long bufferSize) {
    char* bufferCopy = (char*)malloc(sizeof(char) * bufferSize);
    for (long i = 0; i < bufferSize; i++) {
        bufferCopy[i] = singleDecrypt(buffer[i], key[i % keySize], (colz->data & 0xFF), shifts);
        colz = colz->next;
    }

    return bufferCopy;
}

#endif