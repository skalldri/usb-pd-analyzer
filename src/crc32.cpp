#include "crc32.h"

uint32_t reverse(uint32_t input) {
    const int numBits = 32;
    uint32_t output = 0;

    for (int i = 0; i < numBits; i++) {
        // Shift a bit from input down into the zero position, 
        // mask out everything except that bit,
        // then shift back up into the final destination
        output |= (((input >> i) & 0x1) << (numBits - 1 - i));
    }
    
    // cout << "Reverse Input: 0x" << std::hex << input << endl;
    // cout << "Reverse Output: 0x" << std::hex << output << endl;

    return output;
}

uint32_t little_to_big_endian(uint32_t input) {
    uint32_t output = 0;

    uint8_t* inputBytes = (uint8_t*)&input;
    uint8_t* outputBytes = (uint8_t*)&output;

    outputBytes[3] = inputBytes[0];
    outputBytes[2] = inputBytes[1];
    outputBytes[1] = inputBytes[2];
    outputBytes[0] = inputBytes[3];
    
    // cout << "Little Endian Input: 0x" << std::hex << input << endl;
    // cout << "Big Endian Output: 0x" << std::hex << output << endl;

    return output;
}

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len, uint32_t polynomial)
{
    polynomial = reverse(polynomial);

    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
    }
    return ~crc;
}

/*
int main() {
    const char* myString = "123456789";

    uint32_t remainder = crc32(0x00000000, (const uint8_t*)myString, strlen(myString), 0x04C11DB7);

    cout << "Remainder: 0x" << std::hex << remainder << endl;

    uint8_t* buffer = (uint8_t*) malloc(strlen(myString) + sizeof(uint32_t));

    // remainder = little_to_big_endian(remainder);
    // remainder = reverse(remainder);
    // remainder = ~remainder;

    memset(buffer, strlen(myString) + sizeof(uint32_t), 0);

    memcpy(buffer, myString, strlen(myString));
    memcpy(buffer+strlen(myString), &remainder, sizeof(uint32_t));

    cout << "Buffer contents: ";
    for (int i = 0; i < strlen(myString)+sizeof(uint32_t); i++) {
        cout << "0x" << std::hex << (int) buffer[i] << ", ";
    }

    cout << endl;

    uint32_t residual = crc32(0x00000000, (const uint8_t*)buffer, strlen(myString)+sizeof(uint32_t), 0x04C11DB7);
    
    // Bit-reverse the result
    residual = reverse(residual);
    residual = ~residual;

    cout << "Residual: 0x" << std::hex << residual << endl;

    free(buffer);

    return 0;
}
*/