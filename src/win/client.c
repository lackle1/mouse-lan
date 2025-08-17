#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>

void handle_mouse_info(dp_mouse_info *packet) {
    int x = (packet->x * scr_width) / 65535;
    int y = (packet->y * scr_height) / 65535;
    if (!SetCursorPos(x, y)) {
        printf("Error setting cursor position: %d\n", GetLastError());
    }

    printf("Set cursor position to (%d, %d)\n", x, y);
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
    dp snd_packet;
    snd_packet.type = DP_TYPE_MSG;
    memcpy(snd_packet.data, "Dankje", 7); //  Size of string +1 for the null terminator

    // Send an initial bit of data
    i_res = send(socket, (char*)&snd_packet, DP_SIZE_BYTES, 0);
    if (i_res == SOCKET_ERROR) {
        printf("Send failed: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }

    dp packet;
    do {
        memset(&packet, 0, sizeof(packet));
        i_res = recv(socket, (char*)&packet, DP_SIZE_BYTES, 0);
        if (i_res > 0) {
            process_packet(&packet);
        }
        else if (i_res == 0) {
            printf("Server disconnected. Closing connection...\n");
            break;
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }
    } while (i_res > 0 && !check_quit());

    // Shutdown
    i_res = shutdown(socket, SD_BOTH);
    if (i_res == SOCKET_ERROR) {
        printf("Error shutting down: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }
    closesocket(socket);
    WSACleanup();

    printf("Socket successfully shutdown.\n");

    return true;
}

#endif