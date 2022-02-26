#ifndef CRYPTOPS_H
#define CRYPTOPS_H




/**
 * @brief encrypts data using AES Algorithm
 * 
 * @param input data to be ecrypted
 * @param output encrypted data
 * @param size size of input/output data
 */
void encryption(unsigned char* input, unsigned char* output,int size);


/**
 * @brief decrypts data using AES Algorithm
 * 
 * @param input data to be decrypted
 * @param output decrypted data
 * @param size size of input/output data
 */
void decryption(unsigned char* input, unsigned char* output,int size);


/**
 * @brief encrypts and writes data
 * 
 * @param fd 
 * @param buf 
 * @param cnt 
 * @return ssize_t 
 */
ssize_t encrypt_insist_write(int fd, void* buf, size_t cnt);


/**
 * @brief reads and decrypts data
 * 
 * @param fd 
 * @param buf 
 * @param cnt 
 * @return ssize_t 
 */
ssize_t decrypt_insist_read(int fd, void *buf, size_t cnt);




#endif