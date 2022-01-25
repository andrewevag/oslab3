#ifndef CRYPTOPS_H
#define CRYPTOPS_H




/**
 * @brief encrypts data
 * 
 * @param input data to be ecrypted
 * @param output encrypted data
 * @param size size of input/output data
 */
void encryption(unsigned char* input, unsigned char* output,int size);


/**
 * @brief decrypts data
 * 
 * @param input data to be decrypted
 * @param output decrypted data
 * @param size size of input/output data
 */
void decryption(unsigned char* input, unsigned char* output,int size);





#endif