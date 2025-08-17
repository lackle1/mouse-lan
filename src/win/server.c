#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <stdlib.h>
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#define HORIZONTAL_SHIFT 8

void print_addr_if_lan(PIP_ADAPTER_UNICAST_ADDRESS ua) {
    char ip_str[INET_ADDRSTRLEN];
    DWORD ip_str_len = INET_ADDRSTRLEN;

    WSAAddressToString(ua->Address.lpSockaddr, ua->Address.iSockaddrLength, NULL, ip_str, &ip_str_len);

    const char *lan_start = "192.168.";
    if (!strncmp(ip_str, lan_start, strlen(lan_start)) == 0) {
        return;
    }
    
    // Just returns 0, SERVER_PORT is a little more useful
    // Don't need to worry about casting because we only got IPv4 addresses
    // struct sockaddr_in *addr = (struct sockaddr_in*)ua->Address.lpSockaddr;
    // char port_str[6];
    // sprintf(port_str, "%d", ntohs(addr->sin_port));

    printf("%s:%s\n", ip_str, SERVER_PORT);
}

bool print_ip_addr(SOCKET sock) {
    printf("Listening at:\n");

    DWORD rv, flags, size = 0;
    PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
    PIP_ADAPTER_UNICAST_ADDRESS ua;

    flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    
    // First time is just to get the required buffer length
    rv = GetAdaptersAddresses(AF_INET, flags, NULL, NULL, &size);
    if (rv != ERROR_BUFFER_OVERFLOW) {
        printf("Getting adapters addresses failed...");
        return false;
    }
    adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);

    // Now we actually store the data
    rv = GetAdaptersAddresses(AF_INET, flags, NULL, adapter_addresses, &size);
    if (rv != ERROR_SUCCESS) {
        printf("Getting adapters addresses failed...");
        free(adapter_addresses);
        return false;
    }

    for (aa = adapter_addresses; aa != NULL; aa = aa->Next) {
        
        for (ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next) {
            print_addr_if_lan(ua);
        }
    }

    free(adapter_addresses);

    return true;
}

bool initialise_server(SOCKET *socketptr) {
    struct addrinfo *result = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int i_res;

    i_res = getaddrinfo(NULL, SERVER_PORT, &hints, &result);
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

    if (bind(listen_socket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        printf("Error binding socket: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
        return false;
    }
    print_ip_addr(listen_socket);
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

    *socketptr = client_socket;

    return true;
}

// Generates data packet containing mouse information
void make_data_packet(char *buffer, int buf_len) {

}

bool run_server() {
    
    SOCKET socket;
    if (!initialise_server(&socket)) {
        return false;
    }

    printf("Connected!\n");

    int i_res;
    char buffer[512];
    int buf_len = 512;
    int i_snd_res;

    struct fd_set socket_set;
    FD_ZERO(&socket_set);
    FD_SET(socket, &socket_set);
    struct timeval tv;

    tv.tv_sec = 5;
    tv.tv_usec = 5000;

    // Receive until peer shuts down connection
    do {
        // Receive data (if any)
        i_res = select(0, &socket_set, NULL, NULL, &tv);
        if (i_res > 0) {
            i_res = recv(socket, buffer, buf_len, 0);
            if (i_res > 0) {
                printf("Bytes received: %d\n", i_res);
            }
        }
        else if (i_res == 0) {
            printf("Client disconnected. Closing connection...\n");
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }

        // Send data
        make_data_packet(buffer, buf_len);
        i_snd_res = send(socket, buffer, buf_len, 0);
        if (i_snd_res  == SOCKET_ERROR) {
            printf("Error sending data: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }
        printf("Bytes sent: %d\n", i_snd_res);
        
    } while (i_res > 0 && !check_quit());

    // Don't need shutdown because we are just closing the socket, we don't need a graceful disconnect
    closesocket(socket);
    WSACleanup();

    return true;
}

#endif