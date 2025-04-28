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
        for (int i = 0; i < shared_data->order_count; i++) {
            if (strcmp(shared_data->orders[i].type, "Express") == 0 &&
                strcmp(shared_data->orders[i].status, "Pending") == 0) {

                snprintf(shared_data->orders[i].status, STATUS_LEN, "Delivered by %s", agent_name);
                write_log(agent_name, &shared_data->orders[i]);
                sleep(1);
            }
        }
        sleep(1);
    }
    return NULL;
}

void load_csv(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open CSV");
        exit(1);
    }
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        Order *o = &shared_data->orders[shared_data->order_count++];
        sscanf(line, "%[^,],%[^,],%s", o->name, o->address, o->type);
        strcpy(o->status, "Pending");
    }
    fclose(fp);
}

int main() {
    key_t key = ftok("delivery_agent.c", 123);
    shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    shared_data = (SharedData*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    if (shared_data->order_count == 0) {
        load_csv("delivery_order.csv");  
    }

    pthread_t tid[3];
    pthread_create(&tid[0], NULL, agent_thread, "AGENT A");
    pthread_create(&tid[1], NULL, agent_thread, "AGENT B");
    pthread_create(&tid[2], NULL, agent_thread, "AGENT C");

    for (int i = 0; i < 3; i++) {
        pthread_join(tid[i], NULL);
    }

    shmdt(shared_data);
    return 0;
}
