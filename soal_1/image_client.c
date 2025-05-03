#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>

#define PORT 9090
#define BUFFER_SIZE 1048576

void print_menu() {
    printf("\n=== MENU ROOTKIDS ===\n");
    printf("1. Decrypt text file to image\n");
    printf("2. Download image from server\n");
    printf("3. Exit\n");
    printf("Choice: ");
}

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    return sock;
}

void decrypt_text() {
    char filename[256];
    printf("Enter text filename (in secrets/): ");
    scanf("%s", filename);

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "client/secrets/%s", filename);

    FILE *fp = fopen(fullpath, "r");
    if (!fp) {
        printf("Error: Text file not found.\n");
        return;
    }

    char text[BUFFER_SIZE];
    fread(text, 1, BUFFER_SIZE, fp);
    fclose(fp);
    text[strcspn(text, "\r\n")] = 0;

    int sock = connect_to_server();
    if (sock < 0) return;

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DECRYPT %s", text);

    send(sock, request, strlen(request), 0);
    char response[512] = {0};
    int len = read(sock, response, sizeof(response));
    close(sock);

    if (strncmp(response, "ERROR", 5) == 0) {
        printf("Server Error: %s\n", response);
    } else {
        printf("Image saved on server as: %s\n", response);
    }
}

void download_image() {
    char filename[256];
    printf("Enter image filename to download (e.g., 1744399411.jpeg): ");
    scanf("%s", filename);

    int sock = connect_to_server();
    if (sock < 0) return;

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DOWNLOAD %s", filename);
    send(sock, request, strlen(request), 0);

    char response[BUFFER_SIZE];
    int bytes_received = read(sock, response, sizeof(response));
    if (bytes_received <= 0) {
        printf("Failed to receive data from server.\n");
        close(sock);
        return;
    }

    if (strncmp(response, "ERROR", 5) == 0) {
        printf("Server Error: %s\n", response);
        close(sock);
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "client/%s", filename);
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        printf("Failed to save file.\n");
        close(sock);
        return;
    }

    fwrite(response, 1, bytes_received, fp);

    while ((bytes_received = read(sock, response, sizeof(response))) > 0) {
        fwrite(response, 1, bytes_received, fp);
    }

    fclose(fp);
    close(sock);

    printf("Image downloaded and saved to %s\n", filepath);
}

int main() {
    int choice;

    while (1) {
        print_menu();
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                decrypt_text();
                break;
            case 2:
                download_image();
                break;
            case 3:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}
