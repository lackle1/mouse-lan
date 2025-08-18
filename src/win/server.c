#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <stdlib.h>
#include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

int32_t last_pos_x, last_pos_y;    // int32 to avoid overflow

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

// For each axis, returns a value from 0 to 65535 (max size of uint16)
bool get_mouse_pos(uint16_t *ret_x, uint16_t *ret_y) {
    POINT p;
    HWND hwnd = GetForegroundWindow();
    RECT win_rect;
    int32_t delta_x, delta_y, centre_x = scr_width / 2, centre_y = scr_height / 2;     // int32 because integer will overflow otherwise

    if (!GetCursorPos(&p)) {
        printf("\nError getting cursor position: %d\n", GetLastError());
        return false;
    }

    if (!ScreenToClient(hwnd, &p)) {
        printf("\nScreen to client failed. Error: %d\n", GetLastError());
        return false;
    }

    GetWindowRect(hwnd, &win_rect);
    delta_x = p.x + win_rect.left + HORIZONTAL_SHIFT - centre_x;
    delta_y = p.y + win_rect.top - centre_y;
    
    delta_x = (delta_x * MOUSE_POS_MAX) / scr_width;
    delta_y = (delta_y * MOUSE_POS_MAX) / scr_height;
    
    last_pos_x += delta_x;
    last_pos_y += delta_y;

    // Clamp values
    if (last_pos_x < 0) last_pos_x = 0;
    if (last_pos_x >= MOUSE_POS_MAX) last_pos_x = MOUSE_POS_MAX - 1;
    if (last_pos_y < 0) last_pos_y = 0;
    if (last_pos_y >= MOUSE_POS_MAX) last_pos_y = MOUSE_POS_MAX - 1;

    *ret_x = last_pos_x;
    *ret_y = last_pos_y;
    
    if (!SetCursorPos(centre_x, centre_y)) {
        printf("Error centring cursor position: %d\n", GetLastError());
        return false;
    }

    return true;
}

bool create_mouse_data_packet(dp_mouse_info *packet) {
    packet->type = DP_TYPE_MOUSE_INFO;
    
    if (!get_mouse_pos(&packet->x, &packet->y)) {
        return false;
    }
    
    return true;
}

bool run_server(char *parameter) {

    int update_delay = atoi(parameter);
    if (update_delay <= 0 || update_delay > 5000) {
        printf("%s is an invalid parameter: must be an integer between 1 and 5000 inclusive.\n", parameter);
        return false;
    }
    
    SOCKET socket;
    if (!initialise_server(&socket)) {
        return false;
    }

    printf("Connected!\n");

    int i_res;
    int i_snd_res;

    struct fd_set socket_set;
    FD_ZERO(&socket_set);
    FD_SET(socket, &socket_set);
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    dp packet;
    dp_mouse_info mouse_info;
    do {
        // Reset the fd_set (select() modifies it)
        FD_ZERO(&socket_set);
        FD_SET(socket, &socket_set);

        // Receive data (if any)
        i_res = select(0, &socket_set, NULL, NULL, &tv);
        if (i_res > 0) {
            i_res = recv(socket, (char*)&packet, DP_SIZE_BYTES, 0);
            if (i_res > 0) {
                process_packet(&packet);
            }
            else {
                printf("Client disconnected. Closing connection...\n");
                break;
            }
        }
        else if (i_res == 0) {
            // No data to read
        }
        else {
            printf("Error checking socket status: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }

        // Send data
        if (!create_mouse_data_packet(&mouse_info)) {
            printf("Error generating mouse info packet.\n");
            break;
        }
        i_snd_res = send(socket, (char*)&mouse_info, DP_SIZE_BYTES, 0);
        if (i_snd_res  == SOCKET_ERROR) {
            printf("Error sending data: %d\n", WSAGetLastError());
            closesocket(socket);
            WSACleanup();
            return false;
        }

        Sleep(update_delay);
    } while (!check_quit());

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