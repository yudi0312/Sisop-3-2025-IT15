#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>

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

Order *orders;
int shmid;

void write_log(const char *agent, const Order *order) {
    FILE *logfile = fopen("delivery.log", "a");
    if (!logfile) return;

    int fd = fileno(logfile);
    flock(fd, LOCK_EX);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(logfile, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            agent, order->name, order->address);

    flock(fd, LOCK_UN);
    fclose(logfile);
}

void* agent_thread(void *arg) {
    char *agent_name = (char*)arg;
    while (1) {
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].type, "Express") == 0 &&
                !orders[i].is_delivered &&
                strlen(orders[i].name) > 0) {

                orders[i].is_delivered = 1;
                strncpy(orders[i].agent_name, agent_name, MAX_NAME);
                write_log(agent_name, &orders[i]);
                sleep(1);
            }
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    shmid = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    orders = (Order*)shmat(shmid, NULL, 0);
    if (orders == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    pthread_t tid[3];
    pthread_create(&tid[0], NULL, agent_thread, "AGENT A");
    pthread_create(&tid[1], NULL, agent_thread, "AGENT B");
    pthread_create(&tid[2], NULL, agent_thread, "AGENT C");

    for (int i = 0; i < 3; i++) {
        pthread_join(tid[i], NULL);
    }

    shmdt(orders);
    return 0;
}
