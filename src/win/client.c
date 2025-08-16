#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#define HORIZONTAL_SHIFT 8

void set_cursor_pos(char *buf, int buf_len) {

}

bool initialise_client(char *server_addr, SOCKET *socketptr) {
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int i_res;

    i_res = getaddrinfo(server_addr, SERVER_PORT, &hints, &result);
    if (i_res != 0) {
        printf("getaddrinfo failed: %d\n", i_res);
        WSACleanup();
        return false;
    }

    SOCKET connect_socket = INVALID_SOCKET;
    connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("Error at 'socket()': %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    i_res = connect(connect_socket, result->ai_addr, (int)result->ai_addrlen);
    if (i_res == SOCKET_ERROR) {
        printf("Error connecting to server: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(connect_socket);
        WSACleanup();
        return false;
    }
    freeaddrinfo(result);

    *socketptr = connect_socket;

    return true;
}

bool run_client(char* server_addr) {
    SOCKET socket;
    if(!initialise_client(server_addr, &socket)) {
        return false;
    }

    printf("Connected!\n");

    int i_res;
    char recv_buf[512];
    int recv_buf_len = 512;
    
    int i_snd_res;
    const char *snd_data = "Thank you!";

    // Send an initial bit of data
    i_res = send(socket, snd_data, (int)strlen(snd_data), 0);
    if (i_res == SOCKET_ERROR) {
        printf("Send failed: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }
    printf("Bytes sent: %d\n", i_res);

    do {
        i_res = recv(socket, recv_buf, recv_buf_len, 0);
        if (i_res > 0) {
            set_cursor_pos(recv_buf, recv_buf_len);
        }
        else if (i_res == 0) {
            printf("Server disconnected. Closing connection...\n");
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }
    } while (i_res > 0 && !check_quit());

    // Don't need shutdown because we are just closing the socket, we don't need a graceful disconnect
    closesocket(socket);
    WSACleanup();

    return true;
}

#endif