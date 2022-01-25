#include "socket-common.h"
#include <crypto/cryptodev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "linkedlist.h"
#include "cryptops.h"


/**
 * @brief decrypts data
 * 
 * @param input data to be decrypted
 * @param output decrypted data
 * @param size size of input/output data
 */
void decryption(unsigned char* input, unsigned char* output,int size){
    fd = open("/dev/crypto", O_RDWR);
    errorcheck(fd,-1,"open(/dev/crypto)");

    unsigned char key[] = KEY;
    unsigned char iv[] = IV;

    struct session_op sess;
    struct crypt_op cryp;

    memset(&sess, 0, sizeof(sess));
	memset(&cryp, 0, sizeof(cryp));

    /*
	 * Get crypto session for AES128
	 */
	sess.cipher = CRYPTO_AES_CBC;
	sess.keylen = KEY_SIZE;
	sess.key = key;

	errorcheck(!ioctl(fd, CIOCGSESSION, &sess),0, "ioctl(CIOCGSESSION)" );


    /*
	 * Decrypt input to output
	 */
	
	cryp.ses = sess.ses;
	cryp.len = size;
	cryp.src = input;
	cryp.dst = output;
	cryp.iv = iv;
	cryp.op = COP_DECRYPT;

	errorcheck(!ioctl(fd, CIOCCRYPT, &cryp),0, "ioctl(CIOCCRYPT)" );

    /*Finish crypto session*/
    errorcheck(!ioctl(fd, CIOCFSESSION, &sess.ses),0,"ioctl(CIOCFSESSION)");

    errorcheck(close(fd),-1,"close(/dev/crypto)");

}