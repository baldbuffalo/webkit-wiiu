/*
 *  peer_socket.h
 *
 *  WKC socket peer interface.
 *
 *  wkc/wkcsocket.h declares the wkcNet*Peer() Berkeley-socket entry points and
 *  uses the standard BSD socket types (struct sockaddr, socklen_t, struct
 *  addrinfo, fd_set, struct hostent, struct pollfd, struct timeval, ...). This
 *  header maps those types to the target platform's networking headers -- that
 *  mapping is exactly what the peer layer abstracts. On the build/host toolchain
 *  they resolve to the standard POSIX headers; on the Wii U target, wave-browser's
 *  peer layer supplies the equivalents (wut / nsysnet).
 */

#ifndef PEER_SOCKET_H
#define PEER_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>

#endif // PEER_SOCKET_H
