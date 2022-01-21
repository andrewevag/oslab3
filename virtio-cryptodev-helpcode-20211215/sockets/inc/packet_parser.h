#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include "packet.h"

/**
 * @brief Parses a packet from an open stream (here stream is a file descriptor)
 * 
 * @param fd the file descriptor
 * @return a pointer to a packet if parsed correctly else a NULL pointer.
 */
packet* packet_parse(int fd);











#endif