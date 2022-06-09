////////////////////////////////////////////////////////////////////////////////
//
//  File           : fs3_netowork.c
//  Description    : This is the network implementation for the FS3 system.

//
//  Author         : Patrick McDaniel
//  Last Modified  : Thu 16 Sep 2021 03:04:04 PM EDT
//

// Includes
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cmpsc311_log.h>
#include <string.h>
#include <cmpsc311_util.h>

// Project Includes
#include <fs3_network.h>
#include <fs3_driver.h>

//
//  Global data
unsigned char *fs3_network_address = NULL; // Address of FS3 server
unsigned short fs3_network_port = 0;       // Port of FS3 serve
int socket_fd;

//
// Network functions

////////////////////////////////////////////////////////////////////////////////
//
// Function     : network_fs3_syscall
// Description  : Perform a system call over the network
//
// Inputs       : cmd - the command block to send
//                ret - the returned command block
//                buf - the buffer to place received data in
// Outputs      : 0 if successful, -1 if failure

int network_fs3_syscall(FS3CmdBlk cmd, FS3CmdBlk *ret, void *buf)
{
    uint8_t op;
    int16_t sec;
    int32_t trk;
    uint8_t Cret;
    FS3CmdBlk val;

    deconstruct_fs3_cmdblock(cmd, &op, &sec, &trk, &Cret); // deconstruct cmdblk to get op code
    val = htonll64(cmd);                                   // Change cmdblk to network byte order

    if (op == 0)
    {
        char *ip;
        int sPort;

        if (fs3_network_address != NULL)
        {
            ip = (char *)fs3_network_address;
        }
        else
        {
            ip = FS3_DEFAULT_IP;
        }

        if (fs3_network_port != 0)
        {
            sPort = fs3_network_port;
        }
        else
        {
            sPort = FS3_DEFAULT_PORT;
        }

        struct sockaddr_in caddr;
        caddr.sin_family = AF_INET;
        caddr.sin_port = htons(sPort);               // Set port

        if (inet_aton(ip, &caddr.sin_addr) == 0)
        {
            printf("failed on aton");
            return (-1);
        }
        
        socket_fd = socket(PF_INET, SOCK_STREAM, 0); // Create socket

        if (socket_fd == -1)
        {
            printf("Error on socket creation [%s]\n", strerror(errno)); // Sanity checks for errors
            return (-1);
        }
        if (connect(socket_fd, (const struct sockaddr *)&caddr, sizeof(caddr)) == -1) // Connect server to client
        {
            return -1;
        }
        if (write(socket_fd, &val, sizeof(val)) != sizeof(val)) // Write cmdblk over server
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (read(socket_fd, ret, sizeof(ret)) != sizeof(ret)) // Read the returned cmdblk from disk controller
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }
        *ret = ntohll64(*ret); // Change ret to host byte order
    }
    if (op == 1)
    {
        if (write(socket_fd, &val, sizeof(val)) != sizeof(val)) // Sanity checks for errors
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (read(socket_fd, ret, sizeof(ret)) != sizeof(ret))
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }
        *ret = ntohll64(*ret); // Change ret to host byte order
    }
    if (op == 2)
    {
        if (write(socket_fd, &val, sizeof(val)) != sizeof(val)) // Sanity checks for errors
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (read(socket_fd, ret, sizeof(ret)) != sizeof(ret))
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }

        *ret = ntohll64(*ret); // Change ret to host byte order

        if (read(socket_fd, buf, 1024) != 1024) // Read buf read buf received
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }
    }
    if (op == 3)
    {
        if (write(socket_fd, &val, sizeof(val)) != sizeof(val)) // Sanity checks for errors
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (write(socket_fd, buf, 1024) != 1024) // Write/Send buf over server
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (read(socket_fd, ret, sizeof(ret)) != sizeof(ret))
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }
        *ret = ntohll64(*ret); // Change ret to host byte order
    }
    if (op == 4)
    {
        if (write(socket_fd, &val, sizeof(val)) != sizeof(val)) // Sanity checks for errors
        {
            printf("Error writing network data [%s]\n", strerror(errno));
            return -1;
        }
        if (read(socket_fd, ret, sizeof(ret)) != sizeof(ret))
        {
            printf("Error reading network data [%s]\n", strerror(errno));
            return (-1);
        }
        close(socket_fd); // Close socket and set to -1 to avoid use after close in unmount
        socket_fd = -1;
    }

    printf("Receivd a value of [%ld]\n", cmd);

    // Return successfully
    return (0);
}
