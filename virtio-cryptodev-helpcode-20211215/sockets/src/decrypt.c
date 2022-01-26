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
 * @brief decrypts data
 * 
 * @param input data to be decrypted
 * @param output decrypted data
 * @param size size of input/output data
 */
void decryption(unsigned char* input, unsigned char* output,int size){
    int fd = open("/dev/crypto", O_RDWR);
    errorcheck(fd,-1,"open(/dev/crypto) {decrypt}");

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

	errorcheck(!ioctl(fd, CIOCGSESSION, &sess),0, "ioctl(CIOCGSESSION) {decrypt}" );


    /*
	 * Decrypt input to output
	 */
	
	cryp.ses = sess.ses;
	cryp.len = size;
	cryp.src = input;
	cryp.dst = output;
	cryp.iv = iv;
	cryp.op = COP_DECRYPT;

	errorcheck(!ioctl(fd, CIOCCRYPT, &cryp),0, "ioctl(CIOCCRYPT) {decrypt}" );

    /*Finish crypto session*/
    errorcheck(!ioctl(fd, CIOCFSESSION, &sess.ses),0,"ioctl(CIOCFSESSION) {decrypt}");

    errorcheck(close(fd),-1,"close(/dev/crypto)");

}

ssize_t decrypt_insist_read(int fd, void *buf, size_t cnt){
	unsigned char* bufin = sfmalloc(cnt);
	int r = insist_read(fd,bufin,cnt);
	if(r < 0){
		return r;
	}
	decryption(bufin, buf, cnt);
	return r;
}
