#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "shop.h"

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_INVENTORY 10
#define BUFFER_SIZE 1024

struct Player {
    int socket;
    int gold;
    int baseDamage;
    struct Weapon equipped;
    struct Weapon inventory[MAX_INVENTORY];
    int inventoryCount;
    int enemyDefeated;
};

void showStats(struct Player *p, char *buffer) {
    int totalDamage = p->baseDamage + p->equipped.damage;
    sprintf(buffer,
        "\n\x1b[32m== Player Stats ==\x1b[0m\n"
        "\x1b[33mGold: \x1b[0m%d\n"
        "\x1b[31mBase Damage: \x1b[0m%d\n"
        "\x1b[31mWeapon Damage: \x1b[0m%d\n"
        "\x1b[36mTotal Damage: \x1b[0m%d\n"
        "\x1b[34mEquipped Weapon: \x1b[0m%s\n"
        "\x1b[35mPassive Skill: \x1b[0m%s\n"
        "\x1b[35mEnemies Defeated: \x1b[0m%d\n",
        p->gold,
        p->baseDamage,
        p->equipped.damage,
        totalDamage,
        p->equipped.name,
        p->equipped.passive,
        p->enemyDefeated
    );
}

void showInventory(struct Player *p, char *buffer) {
    strcpy(buffer, "\n== Inventory ==\n");
    for (int i = 0; i < p->inventoryCount; i++) {
        char line[256];
        sprintf(line, "%d. %s (%d dmg, %s)%s\n",
                i + 1,
                p->inventory[i].name,
                p->inventory[i].damage,
                p->inventory[i].passive,
                strcmp(p->inventory[i].name, p->equipped.name) == 0 ? " [Equipped]" : "");
        strcat(buffer, line);
    }
    strcat(buffer, "Choose item number to equip or 0 to cancel: ");
}

void getEnemyStatusBar(char *buffer, int currentHP, int maxHP) {
    int barWidth = 20;
    int filledBars = (currentHP * barWidth) / maxHP;
    int emptyBars = barWidth - filledBars;

    char bar[128] = "";
    strcat(bar, "[\x1b[43m");
    for (int i = 0; i < filledBars; i++) strcat(bar, " ");
    strcat(bar, "\x1b[0m");
    for (int i = 0; i < emptyBars; i++) strcat(bar, " ");
    strcat(bar, "]");

    sprintf(buffer,
        "\n\x1b[34m=== ENEMY STATUS ===\x1b[0m\n"
        "Enemy health: %s %d/%d HP\n",
        bar, currentHP, maxHP);
}

void shop(struct Player *p, char *buffer, int clientSock) {
    char shopDisplay[BUFFER_SIZE];
    showShop(shopDisplay);
    send(clientSock, shopDisplay, strlen(shopDisplay), 0);
    int valread = read(clientSock, buffer, BUFFER_SIZE);
    buffer[valread] = '\0';
    buffer[strcspn(buffer, "\r\n")] = 0;

    int choice = atoi(buffer);
    if (choice < 1 || choice > MAX_WEAPONS) {
        send(clientSock, "Invalid choice.\n", 16, 0);
        return;
    }

    int result = buyWeapon(choice - 1, &p->gold, p->inventory, &p->inventoryCount);
    if (result == 1) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Purchased %s!\n", shopWeapons[choice - 1].name);
        send(clientSock, msg, strlen(msg), 0);
    } else if (result == -1) {
        send(clientSock, "Not enough gold.\n", 17, 0);
    } else if (result == -2) {
        send(clientSock, "Inventory is full.\n", 20, 0);
    } else {
        send(clientSock, "Invalid weapon.\n", 17, 0);
    }
}

void battle(struct Player *p, char *buffer, int clientSock) {
    int enemyHP = (rand() % 151) + 50;
    int maxHP = enemyHP;

    char intro[256];
    sprintf(intro,
        "\n\x1b[31m=== BATTLE STARTED ===\x1b[0m\n"
        "Enemy appeared with:\n"
        "\x1b[42m[                     ]\x1b[0m %d/ %d HP\n"
        "Type '\x1b[32mattack\x1b[0m' to attack or '\x1b[31mexit\x1b[0m' to leave battle.\n",
        enemyHP, enemyHP);
    send(clientSock, intro, strlen(intro), 0);

    while (enemyHP > 0) {
        int valread = read(clientSock, buffer, 1024);
        if (valread <= 0) return;
        buffer[valread] = 0;
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            send(clientSock, "Exiting Battle Mode.\n", 22, 0);
            return;
        } else if (strcmp(buffer, "attack") != 0) {
            send(clientSock, "Invalid command. Type 'attack' or 'exit'.\n", 42, 0);
            continue;
        }

        int instakill = 0, crit = 0, doubleGold = 0, charm = 0, ghost = 0;
        char msg[1024] = "";

        if (strstr(p->equipped.passive, "insta-kill") && rand() % 100 < 10) instakill = 1;
        if (strstr(p->equipped.passive, "crit") && rand() % 100 < 30) crit = 1;
        if (strstr(p->equipped.passive, "double gold") && rand() % 100 < 20) doubleGold = 1;
        if (strstr(p->equipped.passive, "memikat hati") && rand() % 100 < 25) charm = 1;
        if (strstr(p->equipped.passive, "ghosting") && rand() % 100 < 25) ghost = 1;

        int base = p->baseDamage + p->equipped.damage;
        int bonus = rand() % 6;
        int dmg = base + bonus;

        if (instakill) {
            enemyHP = 0;
            snprintf(msg, sizeof(msg),
                "\x1b[35m=== INSTANT KILL! ===\x1b[0m\n"
                "Your \x1b[36m%s\x1b[0m obliterated the enemy instantly!\n",
                p->equipped.name);
        } else {
            if (crit) {
                dmg *= 2;
                snprintf(msg, sizeof(msg),
                    "\x1b[33m=== CRITICAL HIT! ===\x1b[0m\n"
                    "You dealt \x1b[31m%d damage\x1b[0m!\n", dmg);
            } else {
                snprintf(msg, sizeof(msg),
                    "You dealt \x1b[31m%d damage\x1b[0m!\n", dmg);
            }
            enemyHP -= dmg;
            if (enemyHP < 0) enemyHP = 0;
        }

        if (charm)
            strcat(msg, "\x1b[35m[Passive Activated: You charmed the enemy, reducing their will to fight!]\x1b[0m\n");
        if (ghost)
            strcat(msg, "\x1b[35m[Passive Activated: You ghosted the enemy, it hesitated!]\x1b[0m\n");

        char healthStatus[128];
        getEnemyStatusBar(healthStatus, enemyHP, maxHP);
        strcat(msg, healthStatus);

        if (enemyHP <= 0) {
            int reward = (rand() % 51) + 50;
            if (doubleGold) reward *= 2;
            p->gold += reward;
            p->enemyDefeated++;

            char rewardMsg[128];
            sprintf(rewardMsg,
                "\n\x1b[32m=== REWARD ===\x1b[0m\n"
                "You earned \x1b[33m%d gold\x1b[0m!\n", reward);
            strcat(msg, rewardMsg);

            send(clientSock, msg, strlen(msg), 0);
            return;
        }

        send(clientSock, msg, strlen(msg), 0);
    }
}

void *handleClient(void *arg) {
    int clientSock = *(int *)arg;
    free(arg);

    struct Player player;
    player.socket = clientSock;
    player.gold = 100;
    player.baseDamage = 10;
    player.inventoryCount = 0;
    player.enemyDefeated = 0;
    strcpy(player.equipped.name, "Fists");
    player.equipped.damage = 0;
    strcpy(player.equipped.passive, "-");

    char buffer[1024];

    while (1) {
        int valread = read(clientSock, buffer, 1024);
        if (valread <= 0) break;
        buffer[valread] = 0;
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strcmp(buffer, "1") == 0) {
            char response[512];
            showStats(&player, response);
            send(clientSock, response, strlen(response), 0);
        } else if (strcmp(buffer, "2") == 0) {
            shop(&player, buffer, clientSock);
        } else if (strcmp(buffer, "3") == 0) {
            char response[1024];
            showInventory(&player, response);
            send(clientSock, response, strlen(response), 0);
            int val = read(clientSock, buffer, 1024);
            buffer[val] = 0;
            buffer[strcspn(buffer, "\r\n")] = 0;
            int choice = atoi(buffer);
            if (choice > 0 && choice <= player.inventoryCount) {
                player.equipped = player.inventory[choice - 1];
                send(clientSock, "Weapon equipped!\n", 17, 0);
            } else {
                send(clientSock, "Cancelled.\n", 11, 0);
            }
        } else if (strcmp(buffer, "4") == 0) {
            battle(&player, buffer, clientSock);
        } else if (strcmp(buffer, "5") == 0) {
            send(clientSock, "Goodbye!\n", 9, 0);
            break;
        } else {
            send(clientSock, "Invalid option.\n", 16, 0);
        }
    }

    close(clientSock);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    printf("Dungeon server running on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        pthread_t tid;
        pthread_create(&tid, NULL, handleClient, pclient);
    }

    return 0;
}
