#include "socket-common.h"
#include <crypto/cryptodev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "linkedlist.h"
#include "cryptops.h"
#include "SafeCalls.h"
#include "anutil.h"

/**
 * @brief encrypts data
 * 
 * @param input data to be ecrypted
 * @param output encrypted data
 * @param size size of input/output data
 */
void encryption(unsigned char* input, unsigned char* output,int size){
	
	int fd = open("/dev/crypto", O_RDWR);
    errorcheck(fd,-1,"open(/dev/crypto) {encrypt}");

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

	errorcheck(!ioctl(fd, CIOCGSESSION, &sess),0, "ioctl(CIOCGSESSION) {encrypt}" );

    /*
	 * Encrypt input to output
	 */
	cryp.ses = sess.ses;
	cryp.len = size;
	cryp.src = input;
	cryp.dst = output;
	cryp.iv = iv;
	cryp.op = COP_ENCRYPT;

	errorcheck(!ioctl(fd, CIOCCRYPT, &cryp),0, "ioctl(CIOCCRYPT) {encrypt}" );

    /*Finish crypto session*/
    errorcheck(!ioctl(fd, CIOCFSESSION, &sess.ses),0,"ioctl(CIOCFSESSION) {encrypt}");

    errorcheck(close(fd),-1,"close(/dev/crypto) {encrypt}");

}


ssize_t encrypt_insist_write(int fd, void* buf, size_t cnt){
	unsigned char* bufout = sfmalloc(cnt);
	memset(bufout, 0, cnt);
	encryption(buf,bufout,cnt);
	return insist_write(fd,bufout,cnt);
}