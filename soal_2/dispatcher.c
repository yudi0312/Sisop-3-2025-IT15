#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>

#define NAME_LEN 64
#define ADDRESS_LEN 128
#define STATUS_LEN 64
#define MAX_ORDERS 100

typedef struct {
    char name[NAME_LEN];
    char address[ADDRESS_LEN];
    char type[10];
    char status[STATUS_LEN];
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int order_count;
} SharedData;

SharedData *shared_data;
int shmid;

void write_log(const char *agent, const Order *order) {
    FILE *logfile = fopen("delivery.log", "a");
    if (!logfile) return;

    int fd = fileno(logfile);
    flock(fd, LOCK_EX);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    fprintf(logfile, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Reguler package delivered to %s in %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            agent, order->name, order->address);

    flock(fd, LOCK_UN);
    fclose(logfile);
}

void deliver_order(const char *target_name, const char *user_agent) {
    for (int i = 0; i < shared_data->order_count; i++) {
        if (strcmp(shared_data->orders[i].name, target_name) == 0) {
            if (strcmp(shared_data->orders[i].type, "Reguler") != 0) {
                printf("Order is not Reguler type.\n");
                return;
            }
            if (strncmp(shared_data->orders[i].status, "Pending", 7) != 0) {
                printf("Order already delivered.\n");
                return;
            }

            snprintf(shared_data->orders[i].status, STATUS_LEN, "Delivered by Agent %s", user_agent);
            write_log(user_agent, &shared_data->orders[i]);
            printf("Delivered successfully.\n");
            return;
        }
    }
    printf("Order not found.\n");
}

void check_status(const char *target_name) {
    for (int i = 0; i < shared_data->order_count; i++) {
        if (strcmp(shared_data->orders[i].name, target_name) == 0) {
            printf("Status for %s: %s\n", target_name, shared_data->orders[i].status);
            return;
        }
    }
    printf("Order not found.\n");
}

void list_orders() {
    for (int i = 0; i < shared_data->order_count; i++) {
        printf("%s - %s\n", shared_data->orders[i].name, shared_data->orders[i].status);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("./dispatcher -deliver [Name]\n");
        printf("./dispatcher -status [Name]\n");
        printf("./dispatcher -list\n");
        return 1;
    }

    key_t key = ftok("delivery_agent.c", 123);
    shmid = shmget(key, sizeof(SharedData), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        char *username = getenv("USER");
        if (!username) username = "UnknownUser";
        deliver_order(argv[2], username);
    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        check_status(argv[2]);
    } else if (strcmp(argv[1], "-list") == 0) {
        list_orders();
    } else {
        printf("Invalid command.\n");
    }

    shmdt(shared_data);
    return 0;
}
