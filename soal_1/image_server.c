#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 9090
#define BUFFER_SIZE 1048576

void write_log(const char *source, const char *action, const char *info) {
    FILE *log = fopen("server.log", "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", t);
    fprintf(log, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
    fclose(log);
}

void reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char tmp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = tmp;
    }
}

int hex_to_bytes(const char *hex, unsigned char *out) {
    int len = strlen(hex);
    if (len % 2 != 0) return -1;
    for (int i = 0; i < len / 2; i++) {
        if (sscanf(hex + 2 * i, "%2hhx", &out[i]) != 1)
            return -1;
    }
    return len / 2;
}

void daemonize() {
    pid_t pid, sid;
  
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
  
    sid = setsid();
    if (sid < 0) exit(EXIT_FAILURE);

    if ((chdir("/home/yudi0312/Sisop-3-2025-IT15/soal_1/server/")) < 0)
        exit(EXIT_FAILURE);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize();

    mkdir("database", 0777);

    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) continue;

        int valread = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (valread <= 0) {
            close(new_socket);
            continue;
        }
        buffer[valread] = '\0';

        if (strncmp(buffer, "DECRYPT ", 8) == 0) {
            char *text_data = buffer + 8;
            write_log("Client", "DECRYPT", "Text data received");

            reverse(text_data);
            unsigned char decoded[BUFFER_SIZE];
            int decoded_len = hex_to_bytes(text_data, decoded);

            if (decoded_len < 0) {
                write(new_socket, "ERROR: Failed to decode hex\n", 29);
                write_log("Server", "ERROR", "Failed to decode hex");
            } else {
                time_t now = time(NULL);
                char filename[256];
                sprintf(filename, "database/%ld.jpeg", now);

                FILE *fp = fopen(filename, "wb");
                if (fp) {
                    fwrite(decoded, 1, decoded_len, fp);
                    fclose(fp);
                    write_log("Server", "SAVE", strrchr(filename, '/') + 1);
                    write(new_socket, strrchr(filename, '/') + 1, strlen(strrchr(filename, '/') + 1));
                } else {
                    write(new_socket, "ERROR: Cannot save file\n", 25);
                    write_log("Server", "ERROR", "Cannot open file to write");
                }
            }

        } else if (strncmp(buffer, "DOWNLOAD ", 9) == 0) {
            char *filename = buffer + 9;
            char path[256];
            snprintf(path, sizeof(path), "database/%s", filename);

            FILE *fp = fopen(path, "rb");
            if (!fp) {
                write(new_socket, "ERROR: File not found\n", 23);
                write_log("Server", "ERROR", "File not found");
            } else {
                write_log("Client", "DOWNLOAD", filename);
                write_log("Server", "UPLOAD", filename);
                int n;
                while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
                    write(new_socket, buffer, n);
                }
                fclose(fp);
            }

        } else {
            write(new_socket, "ERROR: Invalid command\n", 24);
            write_log("Server", "ERROR", "Invalid command");
        }

        close(new_socket);
    }

    return 0;
}
