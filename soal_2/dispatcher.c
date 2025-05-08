#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#define MAX_ORDERS 100
#define MAX_NAME 100
#define MAX_ADDRESS 200
#define SHM_KEY 0x12345

typedef struct {
    char name[MAX_NAME];
    char address[MAX_ADDRESS];
    char type[10]; 
    int is_delivered;
    char agent_name[MAX_NAME];
} Order;

void load_orders(Order *orders) {
    FILE *file = fopen("delivery_order.csv", "r");
    if (!file) {
        perror("Gagal membuka file delivery_order.csv");
        exit(1);
    }

    char line[512];
    int index = 0;
    fgets(line, sizeof(line), file); // skip header if needed

    while (fgets(line, sizeof(line), file) && index < MAX_ORDERS) {
        char *name = strtok(line, ",");
        char *address = strtok(NULL, ",");
        char *type = strtok(NULL, ",\n");

        if (name && address && type) {
            strncpy(orders[index].name, name, MAX_NAME);
            strncpy(orders[index].address, address, MAX_ADDRESS);
            strncpy(orders[index].type, type, 10);
            orders[index].is_delivered = 0;
            strcpy(orders[index].agent_name, "-");
            index++;
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    int shmid = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget error");
        exit(1);
    }

    Order *orders = shmat(shmid, NULL, 0);
    if ((void *)orders == (void *)-1) {
        perror("shmat error");
        exit(1);
    }

    if (orders[0].name[0] == '\0') {
        load_orders(orders);
    }

    if (argc == 2 && strcmp(argv[1], "-list") == 0) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strlen(orders[i].name) == 0) continue;

            if (orders[i].is_delivered) {
                printf("%s - Delivered by %s\n", orders[i].name, orders[i].agent_name);
            } else {
                printf("%s - Pending\n", orders[i].name);
            }
        }
    } else if (argc == 3 && strcmp(argv[1], "-status") == 0) {
        char *target = argv[2];
        int found = 0;
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].name, target) == 0) {
                found = 1;
                if (orders[i].is_delivered) {
                    printf("Status for %s: Delivered by %s\n", target, orders[i].agent_name);
                } else {
                    printf("Status for %s: Pending\n", target);
                }
                break;
            }
        }
        if (!found) {
            printf("Pesanan dengan nama %s tidak ditemukan.\n", target);
        }
    } else if (argc == 3 && strcmp(argv[1], "-deliver") == 0) {
        char *target = argv[2];
        int found = 0;

        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].name, target) == 0) {
                if (strcmp(orders[i].type, "Reguler") == 0 && !orders[i].is_delivered) {
                    found = 1;

                    orders[i].is_delivered = 1;
                    strcpy(orders[i].agent_name, getenv("USER") ? getenv("USER") : "Unknown");

                    FILE *log = fopen("delivery.log", "a");
                    if (log) {
                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        fprintf(log, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] Reguler package delivered to %s in %s\n",
                                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                                t->tm_hour, t->tm_min, t->tm_sec,
                                orders[i].agent_name,
                                orders[i].name, orders[i].address);
                        fclose(log);
                    }

                    printf("Pesanan %s berhasil dikirim oleh %s\n", orders[i].name, orders[i].agent_name);
                } else {
                    printf("Pesanan tidak ditemukan atau bukan Reguler.\n");
                }
                break;
            }
        }
        if (!found) {
            printf("Pesanan %s tidak ditemukan.\n", target);
        }
    } else {
        printf("Penggunaan:\n");
        printf("./dispatcher -list\n");
        printf("./dispatcher -status [Nama]\n");
        printf("./dispatcher -deliver [Nama]\n");
    }

    shmdt(orders);
    return 0;
}
