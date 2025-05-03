#include <stdio.h>
#include <string.h>

#ifndef SHOP_H
#define SHOP_H
#define MAX_WEAPONS 6
#define MAX_INVENTORY 10
struct Weapon {
    char name[64];
    int price;
    int damage;
    char passive[128];
};

struct Weapon shopWeapons[MAX_WEAPONS] = {
    {"Crowbar", 50, 10, ""},
    {"Meowmere", 150, 25, ""},
    {"Buster sword", 200, 35, ""},
    {"High-Frequency blade", 99, 20, "+10% memikat hati perempuan"},
    {"Moonlight greatsword", 300, 50, ""},
    {"Kusabimaru", 400, 55, "+999% ghosting"}
};

void showShop(char *buffer) {
    strcpy(buffer, "=== Weapon Shop ===\n");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        char line[256];
        sprintf(line, "\x1b[32m[%d] %s - \x1b[33m%d gold - \x1b[31m%d dmg\x1b[34m%s%s\x1b[0m\n",
                i + 1,
                shopWeapons[i].name,
                shopWeapons[i].price,
                shopWeapons[i].damage,
                strlen(shopWeapons[i].passive) > 0 ? " - Passive: " : "",
                shopWeapons[i].passive);
        strcat(buffer, line);
    }
    strcat(buffer, "\n\x1b[33mChoose weapon number to buy: \x1b[0m");
}

int buyWeapon(int index, int *gold, struct Weapon *inventory, int *inventoryCount) {
    if (index < 0 || index >= MAX_WEAPONS) return 0;
    if (*inventoryCount >= MAX_INVENTORY) return -2;
    struct Weapon weapon = shopWeapons[index];
    if (*gold < weapon.price) return -1;

    inventory[*inventoryCount] = weapon;
    (*inventoryCount)++;
    *gold -= weapon.price;
    return 1;
}


#endif
