/*
 * socket-common.h
 *
 * Simple TCP/IP communication using sockets
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 */

#ifndef _SOCKET_COMMON_H
#define _SOCKET_COMMON_H

/* Compile-time options */
#define TCP_PORT    35018
#define TCP_BACKLOG 5

#define HELLO_THERE "Hello there!"
#define MAX_MSIZE 4096

#define DATA_SIZE       304 // packet size
#define BLOCK_SIZE      16
#define KEY_SIZE	16  /* AES128 */
#define KEY {254,56,64,66,97,111,23,26,78,90,99,33,57,111,54,2}
#define IV {46,77,22,8,6,3,86,1,34,57,68,98,87,45,56,254}
#define CRYPTODEV_NODE "/dev/crypto"

#endif /* _SOCKET_COMMON_H */

