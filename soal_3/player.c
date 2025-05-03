#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <regex.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void strip_ansi(char *str) {
    regex_t regex;
    regcomp(&regex, "\033\\[[0-9;]*m", REG_EXTENDED);
    regmatch_t match;
    char result[BUFFER_SIZE] = "";
    char *cursor = str;

    while (regexec(&regex, cursor, 1, &match, 0) == 0) {
        strncat(result, cursor, match.rm_so);
        cursor += match.rm_eo;
    }
    strcat(result, cursor);
    strcpy(str, result);
    regfree(&regex);
}

void dashboard() {
    printf("\033[2J\033[1;1H");
    printf("\n");
    printf("\033[1;36m");
    printf("████████╗██╗  ██╗███████╗    ██╗      ██████╗ ███████╗████████╗    ██████╗ ██╗   ██╗███╗   ██╗ ██████╗ ███████╗ ██████╗ ███╗   ██╗\n");
    printf("╚══██╔══╝██║  ██║██╔════╝    ██║     ██╔═══██╗██╔════╝╚══██╔══╝    ██╔══██╗██║   ██║████╗  ██║██╔════╝ ██╔════╝██╔═══██╗████╗  ██║\n");
    printf("   ██║   ███████║█████╗      ██║     ██║   ██║███████╗   ██║       ██║  ██║██║   ██║██╔██╗ ██║██║  ███╗█████╗  ██║   ██║██╔██╗ ██║\n");
    printf("   ██║   ██╔══██║██╔══╝      ██║     ██║   ██║╚════██║   ██║       ██║  ██║██║   ██║██║╚██╗██║██║   ██║██╔══╝  ██║   ██║██║╚██╗██║\n");
    printf("   ██║   ██║  ██║███████╗    ███████╗╚██████╔╝███████║   ██║       ██████╔╝╚██████╔╝██║ ╚████║╚██████╔╝███████╗╚██████╔╝██║ ╚████║\n");
    printf("   ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚══════╝ ╚═════╝ ╚══════╝   ╚═╝       ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝  ╚═══╝\n\n");
    printf("\033[0m");
}

void print_menu() {
    dashboard();
    printf("\x1b[32m=== MAIN MENU ===\x1b[0m\n");
    printf("1. Show Player Stats\n");
    printf("2. Shop (Buy Weapons)\n");
    printf("3. View Inventory & Equip Weapons\n");
    printf("4. Battle Mode\n");
    printf("5. Exit Game\n");
    printf("\x1b[33mChoose an option: \x1b[0m");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    char input[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        print_menu();
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        send(sock, input, strlen(input), 0);
        memset(buffer, 0, BUFFER_SIZE);

        if (strcmp(input, "4") == 0) {
            dashboard();
            while (1) {
                read(sock, buffer, BUFFER_SIZE);
                printf("%s\n", buffer);

                if (strstr(buffer, "REWARD") || strstr(buffer, "Exiting Battle Mode")) break;

                printf("Type command (attack/exit): ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;
                send(sock, input, strlen(input), 0);
                memset(buffer, 0, BUFFER_SIZE);
            }
        } else {
            read(sock, buffer, BUFFER_SIZE);
            printf("%s\n", buffer);

            if (strstr(buffer, "Choose weapon number to buy:") ||
                strstr(buffer, "Choose item number to equip")) {

                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;
                send(sock, input, strlen(input), 0);
                memset(buffer, 0, BUFFER_SIZE);
                read(sock, buffer, BUFFER_SIZE);
                printf("%s\n", buffer);
                printf("\nPress Enter to return to menu...");
                getchar();
            } else if (strcmp(input, "5") == 0) {
                break;
            } else {
                printf("\nPress Enter to return to menu...");
                getchar();
            }
        }
    }

    close(sock);
    return 0;
}
