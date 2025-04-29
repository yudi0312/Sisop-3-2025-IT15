#ifndef SHM_COMMON_H
#define SHM_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/sem.h>

#define MAX_HUNTERS 50
#define MAX_DUNGEONS 50

struct Hunter {
    char username[50];
    int level;
    int exp;
    int atk;
    int hp;
    int def;
    int banned;
    key_t shm_key;
};

struct Dungeon {
    char name[50];
    int min_level;
    int exp;
    int atk;
    int hp;
    int def;
    unsigned long key;
    key_t shm_key;
};

struct SystemData {
    struct Hunter hunters[MAX_HUNTERS];
    int num_hunters;
    struct Dungeon dungeons[MAX_DUNGEONS];
    int num_dungeons;
    int current_notification_index;
};

key_t get_system_key() {
    return ftok("/tmp", 'S');
}

#endif

#define DUNGEON_COUNT 11

const char* dungeon_names[DUNGEON_COUNT] = {
    "Double Dungeon", "Demon Castle", "Pyramid Dungeon", "Red Gate Dungeon",
    "Hunters Guild Dungeon", "Busan A-Rank Dungeon", "Insects Dungeon",
    "Goblins Dungeon", "D-Rank Dungeon", "Gwanak Mountain Dungeon",
    "Hapjeong Subway Station Dungeon"
};

struct SystemData *systemData;
int shmid = -1;
int semid = -1;

void lock() {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void unlock() {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void cleanup(int signo) {
    if (systemData != (void*) -1) shmdt(systemData);
    if (shmid != -1) shmctl(shmid, IPC_RMID, NULL);
    if (semid != -1) semctl(semid, 0, IPC_RMID);
    printf("\nShared memory and semaphores have been removed. The system is shutting down.\n");
    exit(0);
}

int findHunter(const char *username) {
    for (int i = 0; i < systemData->num_hunters; i++) {
        if (strcmp(systemData->hunters[i].username, username) == 0) return i;
    }
    return -1;
}

int main() {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }
    systemData = (struct SystemData*) shmat(shmid, NULL, 0);
    if (systemData == (void*) -1) { perror("shmat"); exit(1); }

    srand(time(NULL));
    if (systemData->num_hunters == 0 && systemData->num_dungeons == 0) {
        memset(systemData, 0, sizeof(struct SystemData));
    }

    key_t semkey = ftok("/tmp", 'M');
    semid = semget(semkey, 1, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); cleanup(0); }
    semctl(semid, 0, SETVAL, 1);

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    int choice;
    char name[50];

    while (1) {
        printf("\n===== SYSTEM MENU =====\n");
        printf("1. Hunter Info\n2. Dungeon Info\n3. Generate Dungeon\n4. Ban Hunter\n5. Reset Hunter\n6. Exit\nChoice : ");
        scanf("%d", &choice); getchar();

        if (choice == 1) {
            lock();
            printf("\n-- Hunter Info --\n");
            for (int i = 0; i < systemData->num_hunters; i++) {
                struct Hunter *h = &systemData->hunters[i];
                printf("Username: %s | Level: %d | EXP: %d | ATK: %d | HP: %d | DEF: %d | Banned: %s\n",
                    h->username, h->level, h->exp, h->atk, h->hp, h->def,
                    h->banned ? "Yes" : "No");
            }
            if (systemData->num_hunters == 0) printf("(No hunter registered)\n");
            unlock();
        }
        else if (choice == 2) {
            lock();
            printf("\n-- Dungeon Info --\n");
            for (int i = 0; i < systemData->num_dungeons; i++) {
                struct Dungeon *d = &systemData->dungeons[i];
                printf("\nName: %s\nMin Level: %d\nATK: %d\nHP: %d\nDEF: %d\nEXP: %d\nKEY: %lu\n",
                    d->name, d->min_level, d->atk, d->hp, d->def, d->exp, d->key);
            }
            if (systemData->num_dungeons == 0) printf("(No dungeons available)\n");
            unlock();
        }
        else if (choice == 3) {
            lock();
            if (systemData->num_dungeons < MAX_DUNGEONS) {
                struct Dungeon *d = &systemData->dungeons[systemData->num_dungeons];
                int idx = rand() % DUNGEON_COUNT;
                snprintf(d->name, sizeof(d->name), "%s", dungeon_names[idx]);
                d->min_level = (systemData->num_dungeons == 0) ? 1 : (rand() % 5) + 1;
                d->atk = (rand() % 51) + 100;
                d->hp = (rand() % 51) + 50;
                d->def = (rand() % 26) + 25;
                d->exp = (rand() % 151) + 150;
                d->key = (unsigned long) time(NULL) ^ rand();
                systemData->num_dungeons++;
                printf("Dungeon generated: %s (MinLvl %d)\n", d->name, d->min_level);
            } else {
                printf("Dungeon list is full.\n");
            }
            unlock();
        }
        else if (choice == 4) {
            printf("Enter hunter username to ban: ");
            fgets(name, 50, stdin); name[strcspn(name, "\n")] = 0;
            lock();
            int idx = findHunter(name);
            if (idx >= 0) {
                systemData->hunters[idx].banned = 1;
                printf("Hunter '%s' has been banned.\n", name);
            } else {
                printf("Hunter '%s' not found.\n", name);
            }
            unlock();
        }
        else if (choice == 5) {
            printf("Enter hunter username to reset: ");
            fgets(name, 50, stdin); name[strcspn(name, "\n")] = 0;
            lock();
            int idx = findHunter(name);
            if (idx >= 0) {
                struct Hunter *h = &systemData->hunters[idx];
                h->level = 1; h->exp = 0; h->atk = 10; h->hp = 100; h->def = 5; h->banned = 0;
                printf("Stats for hunter '%s' have been reset.\n", name);
            } else {
                printf("Hunter '%s' not found.\n", name);
            }
            unlock();
        }
        else if (choice == 6) {
            printf("Exiting system...\n");
            break;
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    cleanup(0);
    return 0;
}
