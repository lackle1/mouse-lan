#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT "27015"
#define HORIZONTAL_SHIFT 8

bool create_socket() {
    WSADATA wsa_data;
    int i_res = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (i_res != 0) {
        printf("WSA initialisation failed. RESULT: %d\n", i_res);
        return false;
    }

    struct addrinfo *result = NULL, *ptr = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    i_res = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (i_res != 0) {
        printf("getaddrinfo failed: %d\n", i_res);
        WSACleanup();
        return false;
    }

    SOCKET listen_socket = INVALID_SOCKET;
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        printf("Error at 'socket()': %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    if (bind(listen_socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
        printf("Error binding socket: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }
    freeaddrinfo(result);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Error listening on socket: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }

    SOCKET client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        printf("accept() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }
    closesocket(listen_socket);

    char recv_buf[512];
    int i_snd_res;
    int recv_buf_len = 512;

    // Receive until peer shuts down connection
    do {
        i_res = recv(client_socket, recv_buf, recv_buf_len, 0);
        if (i_res > 0) {
            printf("Bytes received: %d\n", i_res);

            // Send buffer back to sender
            i_snd_res = send(client_socket, recv_buf, recv_buf_len, 0);
            if (i_snd_res  == SOCKET_ERROR) {
                printf("Error sending data: %d\n", WSAGetLastError());
                closesocket(client_socket);
                WSACleanup();
                return false;
            }
            printf("Bytes send: %d\n", i_snd_res);
        }
        else if (i_res == 0) {
            printf("Closing connection...\n");
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return false;
        }
    } while (i_res > 0);

    i_res = shutdown(client_socket, SD_BOTH);
    if (i_res == SOCKET_ERROR) {
        printf("Error shutting down: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return false;
    }

    closesocket(client_socket);
    WSACleanup();

    return true;
}

int run_server(int scr_width, int scr_height) {
    POINT p;
    HWND hwnd = GetForegroundWindow();
    RECT win_rect;
    int x, y;
    float normalised_x, normalised_y;

    printf("Tracking mouse position. CTRL+C to exit.\n");
    while(true) {
        if (!GetCursorPos(&p)) {
            printf("\nError getting cursor position: %d\n", GetLastError());
            continue;
        }

        if (!ScreenToClient(hwnd, &p)) {
            printf("\nScreen to client failed. Error: %d\n", GetLastError());
            continue;
        }

        GetWindowRect(hwnd, &win_rect);
        x = p.x + win_rect.left + HORIZONTAL_SHIFT;
        y = p.y + win_rect.top;
        
        normalised_x = (float)x / scr_width;
        normalised_y = (float)y / scr_height;
        
        printf("\rMouse Position: %f, %f       ", normalised_x, normalised_y); // + a bunch of spaces to get rid of any numbers leftover from before

        Sleep(10);
    }
}

int run(enum Role role) {
    int scr_width = GetSystemMetrics(SM_CXSCREEN);
    int scr_height = GetSystemMetrics(SM_CYSCREEN);

    run_server(scr_width, scr_height);

    return 0;
}

#endif