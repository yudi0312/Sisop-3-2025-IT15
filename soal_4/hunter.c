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

struct SystemData *systemData;
int shmid = -1;
int semid = -1;
char myUsername[50];
int myIndex = -1;
int loggedIn = 0;
int runNotif = 0;
pthread_t notifThread;

void lock() {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void unlock() {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int findHunter(const char *username) {
    for (int i = 0; i < systemData->num_hunters; i++) {
        if (strcmp(systemData->hunters[i].username, username) == 0) return i;
    }
    return -1;
}

void* notification(void *arg) {
    while (runNotif) {
        sleep(3);
        lock();
        if (myIndex >= 0 && myIndex < systemData->num_hunters) {
            int myLevel = systemData->hunters[myIndex].level;
            printf("\n[NOTIF] Dungeon minimum level %d:\n", myLevel);
            int found = 0;
            for (int i = 0; i < systemData->num_dungeons; i++) {
                struct Dungeon *d = &systemData->dungeons[i];
                if (d->min_level <= myLevel) {
                    printf(" - %s (MinLvl:%d, ATK:%d, HP:%d, DEF:%d, EXP:%d)\n",
                        d->name, d->min_level, d->atk, d->hp, d->def, d->exp);
                    found++;
                }
            }
            if (!found) printf(" [NOTIF] There are no accessible dungeons.\n");
            printf("> "); fflush(stdout);
        }
        unlock();
    }
    return NULL;
}

int main() {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }
    systemData = (struct SystemData*) shmat(shmid, NULL, 0);
    if (systemData == (void*) -1) { perror("shmat"); exit(1); }

    key_t semkey = ftok("/tmp", 'M');
    semid = semget(semkey, 1, 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    int choice;
    char username[50];

    while (1) {
        if (!loggedIn) {
            printf("\n===== HUNTER LOGIN =====\n1. Register\n2. Login\n3. Exit\nChoice : ");
            scanf("%d", &choice); getchar();

            if (choice == 1) {
                printf("Username: ");
                fgets(username, 50, stdin);
                username[strcspn(username, "\n")] = 0;
                lock();
                if (systemData->num_hunters < MAX_HUNTERS && findHunter(username) == -1) {
                    struct Hunter *h = &systemData->hunters[systemData->num_hunters++];
                    strcpy(h->username, username);
                    h->level = 1; h->exp = 0; h->atk = 10; h->hp = 100; h->def = 5  ;
                    h->banned = 0;
                    printf("Hunter '%s' successfully registered.\n", username);
                } else {
                    printf("Registration failed. Username already exists or capacity is full.\n");
                }
                unlock();
            }
            else if (choice == 2) {
                printf("Username: ");
                fgets(username, 50, stdin);
                username[strcspn(username, "\n")] = 0;
                lock();
                int idx = findHunter(username);
                if (idx >= 0) {
                    loggedIn = 1;
                    myIndex = idx;
                    strcpy(myUsername, username);
                    printf("Login successful. Welcome, %s!\n", username);
                
                    if (systemData->hunters[idx].banned) {
                        printf("⚠️  WARNING: Your account is currently banned. You can log in but cannot raid or battle.\n");
                    }
                } else {
                    printf("Login failed. Username not found.\n");
                }
                
                unlock();
            }
            else if (choice == 3) {
                printf("Exit the application.\n");
                break;
            }
            else {
                printf("Invalid choice.\n");
            }
        }

        else {
            printf("\n===== HUNTER SYSTEM =====\n===== %s's MENU =====\n1. List Dungeon\n2. Raid\n3. Battle\n4. Toggle Notification\n5. Exit\nChoice : ", myUsername);
            scanf("%d", &choice); getchar();

            if (choice == 1) {
                lock();
                int myLevel = systemData->hunters[myIndex].level;
                printf("\nAvailable Dungeons (Level %d):\n", myLevel);
                int found = 0;
                for (int i = 0; i < systemData->num_dungeons; i++) {
                    struct Dungeon *d = &systemData->dungeons[i];
                    if (d->min_level <= myLevel) {
                        printf(" - %s (Level: %d+ )\n",
                            d->name, d->min_level);
                        found++;
                    }
                }
                if (!found) printf(" There are no accessible dungeons.\n");
                unlock();
            }
            else if (choice == 2) {
                if (systemData->hunters[myIndex].banned) {
                    printf("❌ You are banned and cannot raid.\n");
                    continue;
                }
                lock();
                int myLevel = systemData->hunters[myIndex].level;
                printf("\n===== RAIDABLE DUNGEONS =====\n");
                int list[MAX_DUNGEONS], count = 0;
                for (int i = 0; i < systemData->num_dungeons; i++) {
                    if (systemData->dungeons[i].min_level <= myLevel) {
                        printf("%d. %s (EXP:%d)\n", count + 1,
                            systemData->dungeons[i].name,
                            systemData->dungeons[i].exp);
                        list[count++] = i;
                    }
                }
                if (count == 0) {
                    printf("There are no dungeons to raid.\n");
                    unlock(); continue;
                }

                int pilih;
                printf("Pilih nomor dungeon: ");
                scanf("%d", &pilih); getchar();
                if (pilih < 1 || pilih > count) {
                    printf("Pilihan tidak valid.\n");
                } else {
                    struct Dungeon d = systemData->dungeons[list[pilih - 1]];
                    systemData->hunters[myIndex].exp += d.exp;
                    systemData->hunters[myIndex].atk += d.atk;
                    systemData->hunters[myIndex].hp += d.hp;
                    systemData->hunters[myIndex].def += d.def;
                    printf("Raid success! Gained : \nEXP: %d \nATK: %d \nHP: %d \nDEF: %d\n",
                        d.exp, d.atk, d.hp, d.def);
                    if (systemData->hunters[myIndex].exp >= 500) {
                        systemData->hunters[myIndex].level++;
                        systemData->hunters[myIndex].exp = 0;
                        printf("Level up! Now level : %d\n", systemData->hunters[myIndex].level);
                    }
                    for (int j = list[pilih - 1]; j < systemData->num_dungeons - 1; j++) {
                        systemData->dungeons[j] = systemData->dungeons[j + 1];
                    }
                    systemData->num_dungeons--;
                }
                unlock();
            }
            else if (choice == 3) {
                if (systemData->hunters[myIndex].banned) {
                    printf("❌ You are banned and cannot battle.\n");
                    continue;
                }
                lock();
                printf("\n=== PVP LIST ===\n");
                for (int i = 0; i < systemData->num_hunters; i++) {
                    if (i != myIndex) {
                        struct Hunter *h = &systemData->hunters[i];
                        int power = h->atk + h->hp + h->def;
                        printf("%s - Total Power: %d\n", h->username, power);
                    }
                }
            
                printf("Target: ");
                fgets(username, 50, stdin);
                username[strcspn(username, "\n")] = 0;
                int opp = findHunter(username);
            
                if (opp < 0 || opp == myIndex) {
                    printf("Hunter is invalid.\n");
                    unlock(); continue;
                }
            
                printf("You chose to battle %s\n", username);
            
                int myPower = systemData->hunters[myIndex].atk + systemData->hunters[myIndex].hp +
                              systemData->hunters[myIndex].def;
                int oppPower = systemData->hunters[opp].atk + systemData->hunters[opp].hp +
                               systemData->hunters[opp].def;
            
                printf("Your Power: %d\n", myPower);
                printf("Opponent's Power: %d\n", oppPower);
            
                if (myPower > oppPower) {
                    printf("Battle won! You acquired %s's stats\n", username);
                    systemData->hunters[myIndex].exp += systemData->hunters[opp].exp;
                    systemData->hunters[myIndex].atk += systemData->hunters[opp].atk;
                    systemData->hunters[myIndex].hp += systemData->hunters[opp].hp;
                    systemData->hunters[myIndex].def += systemData->hunters[opp].def;
                    for (int i = opp; i < systemData->num_hunters - 1; i++) {
                        systemData->hunters[i] = systemData->hunters[i + 1];
                    }
                    systemData->num_hunters--;
                    if (opp < myIndex) myIndex--;
                } else if (myPower < oppPower) {
                    printf("You lost! Opponent %s takes your stats.\n", username);
                    systemData->hunters[opp].exp += systemData->hunters[myIndex].exp;
                    systemData->hunters[opp].atk += systemData->hunters[myIndex].atk;
                    systemData->hunters[opp].hp += systemData->hunters[myIndex].hp;
                    systemData->hunters[opp].def += systemData->hunters[myIndex].def;
                    for (int i = myIndex; i < systemData->num_hunters - 1; i++) {
                        systemData->hunters[i] = systemData->hunters[i + 1];
                    }
                    systemData->num_hunters--;
                    unlock();
                    runNotif = 0;
                    pthread_cancel(notifThread);
                    shmdt(systemData);
                    exit(0);
                } else {
                    printf("⚔️  It's a draw! No one wins or loses.\n");
                }
                unlock();
            }
            else if (choice == 4) {
                if (runNotif) {
                    runNotif = 0;
                    pthread_cancel(notifThread);
                    printf("🔕 Notifications turned off.\n");
                } else {
                    runNotif = 1;
                    pthread_create(&notifThread, NULL, notification, NULL);
                    printf("🔔 Notifications turned on.\n");
                }
            }
            else if (choice == 5) {
                printf("Logout and exit...\n");
                runNotif = 0;
                pthread_cancel(notifThread);
                shmdt(systemData);
                exit(0);
            }
            else {
                printf("Invalid choice.\n");
            }
        }
    }

    shmdt(systemData);
    return 0;
}
