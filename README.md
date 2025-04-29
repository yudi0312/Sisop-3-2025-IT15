# Praktikum Sisop Modul 1-2025-IT15

Anggota kelompok : 
- Putu Yudi Nandanjaya Wiraguna	5027241080
- Naruna Vicranthyo Putra Gangga	5027241105
- Az Zahrra Tasya Adelia	        5027241087

## Soal 2 - RushGo

Author : Putu Yudi Nandanjaya Wiraguna (5027241080)

### Deskripsi

Author diberikan tugas untuk membuat sistem layanan ekspedisi untuk perusahaan **RushGo** yang terdiri atas dua bagian utama, yaitu : 
- `delivery_agent.c` untuk agen otomatis pengantar Express
- `dispatcher.c` untuk pengiriman dan monitoring pesanan oleh user

Berikutnya author diberikan beberapa tugas, yaitu : 

#### a. Mengunduh file order dan menyimpannya ke shared memory.  

![Image](https://github.com/user-attachments/assets/eec921ea-0fdb-4326-89ff-70d3a432aa4e)

Untuk mengunduh file order, author memakai `wget` dan melakukan rename menggunakan `-O` menjadi `delivery_order.csv`. File `delivery_order.csv` ini berisikan data semua pesanan yang masuk ke sistem RushGo.

```
1. Membuka shared memory
key_t key = ftok("delivery_agent.c", 123);
int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
SharedData *shared_data = (SharedData*)shmat(shmid, NULL, 0);

2. Membaca file CSV
FILE *fp = fopen("delivery_order.csv", "r");
char line[512];
while (fgets(line, sizeof(line), fp)) {
    Order *o = &shared_data->orders[shared_data->order_count++];
    sscanf(line, "%[^,],%[^,],%s", o->name, o->address, o->type);
    strcpy(o->status, "Pending");
}
fclose(fp);
```

Setelah file `delivery_order.csv` telah diunduh, author diberikan tugas untuk membaca data file `delivery_order.csv` dan juga menyimpannya ke dalam shared memory. 

#### b. Pengiriman bertipe express

RushGo ingin agar pesanan expressnya dikirimkan secara otomatis oleh sistem, tanpa disuruh manual oleh user. Dari hal tersebut, disediakan 3 agen otomatis : 
- Agent A
- Agent B
- Agent C

Agen-agen ini akan secara otomatis:
- Mencari order bertipe Express yang belum dikirim.
- Mengambil dan mengirimkannya tanpa intervensi user.


Setelah sukses mengantar, program harus mencatat log di delivery.log dengan format:

`[dd/mm/yyyy hh:mm:ss] [AGENT A/B/C] Express package delivered to [Nama] in [Alamat]`

Program tersebut dapat dilihat pada [delivery_agent.c](https://github.com/yudi0312/Sisop-3-2025-IT15/blob/main/soal_2/delivery_agent.c) , dimana memiliki cara kerja sebagai berikut : 
- Shared Memory: Program membuka shared memory untuk mengakses daftar semua pesanan.
- Membaca Pesanan: Pesanan bertipe Express dibaca dan dipantau status pengirimannya.
- Membuat Thread Agen: Tiga thread dibuat untuk masing-masing agen: AGENT A, AGENT B, dan AGENT C.
- Mencari Pesanan: Setiap agen mencari pesanan Express yang belum dikirim.
- Mengantar Pesanan: Agen menandai pesanan sebagai terkirim dan mencatat namanya.
- Mencatat ke Log: Setelah pengiriman, agen menulis laporan ke file delivery.log.
- Penggunaan Mutex: Mutex digunakan agar antar thread tidak berebut pesanan.
- Loop Berjalan Terus: Agen terus berjalan untuk menangani pesanan baru yang muncul.

Screenshot untuk bagian `delivery.log` pada program `./delivery_agent` : 

![Image](https://github.com/user-attachments/assets/3bbadbae-495a-4854-8e9a-4fd67159cb03)

#### c. Pengiriman bertipe reguler 

Berbeda dengan Express, untuk order bertipe Reguler, pengiriman dilakukan secara manual oleh user.
- User dapat mengirim permintaan untuk mengantar order Reguler dengan memberikan perintah deliver dari dispatcher. Penggunaan:

`./dispatcher -deliver [Nama]`
- Pengiriman dilakukan oleh agent baru yang namanya adalah nama user.
- Setelah sukses mengantar, program harus mencatat log di delivery.log dengan format:

`[dd/mm/yyyy hh:mm:ss] [AGENT <user>] Reguler package delivered to [Nama] in [Alamat]`

Program tersebut dapat dilihat pada [dispatcher.c](https://github.com/yudi0312/Sisop-3-2025-IT15/blob/main/soal_2/dispatcher.c), dimana memiliki cara kerja sebagai berikut : 

1. Validasi Pesanan : Dispatcher akan mengecek apakah nama yang diminta benar-benar ada di daftar pesanan.

2. Cek Status Pesanan : Jika nama ada, tapi pesanan sudah dikirim sebelumnya, dispatcher tidak akan mengirim ulang.

3. Assign Agen (User) : Nama agent pengantar diisi otomatis dengan nama user yang menjalankan perintah -deliver.

4. Tulis Log Setelah Pengiriman : Setelah pesanan dikirim, program langsung membuka file delivery.log, menambahkan catatan baru sesuai dengan format. 

5. Sinkronisasi Shared Memory : Setiap perubahan (status pengiriman dan nama agent) disimpan kembali ke shared memory, supaya data tetap konsisten untuk semua proses yang berjalan.

Screenshot `./dispatcher -deliver [Nama]` : 

![Image](https://github.com/user-attachments/assets/6345f656-8918-46db-9914-ed025acebc14)

![Image](https://github.com/user-attachments/assets/3c3afd7b-0ea6-484b-a4d8-b04126f7a8c9)

#### d. Mengecek status pesanan

Dispatcher juga harus bisa mengecek status setiap pesanan.

Penggunaan:

`./dispatcher -status [Nama]`

Cara kerja untuk bagian mengecek status pesanan, yaitu : 
1. Membuka shared memory tempat semua data pesanan disimpan.
2. Mencari pesanan berdasarkan nama pelanggan yang diinput.
3. Jika ketemu:
- Kalau sudah terkirim → tampilkan nama agent yang mengirimkan.
- Kalau belum dikirim → tampilkan bahwa statusnya Pending.
4. Kalau nama tidak ada → tampilkan pesan "Order not found".

Screenshot `./dispatcher -status [Nama]` : 

![Image](https://github.com/user-attachments/assets/4437880e-0e5c-4d5d-84c4-7f29ee3eafd0)

#### e. Melihat daftar semua pesanan

Untuk memudahkan monitoring, program dispatcher bisa menjalankan perintah list untuk melihat semua order disertai nama dan statusnya.
Penggunaan:
`./dispatcher -list`  

Cara kerja untuk bagian melihat daftar semua pesanan, yaitu 
1. Program `./dispatcher` membuka shared memory tempat data pesanan RushGo disimpan.
2. Program membaca seluruh isi daftar pesanan dari shared memory.
3. Setelah itu akan menampilkan status pesanan :
- Jika sudah terkirim → tampilkan Delivered by [nama agent].
- Jika belum dikirim → tampilkan Pending.
4. Setelah semua data ditampilkan, shared memory akan dilepas.

Screenshot `./dispatcher -list` : 

![Image](https://github.com/user-attachments/assets/234a66ad-580c-4988-b424-782ca46eb59e)

Kendala : Tidak terdapat kendala dalam mengerjakan soal. 

## Soal 4 - Sung Jin Woo

Author : Putu Yudi Nandanjaya Wiraguna (5027241080)

### Deskripsi

Author diberikan oleh Sung Jin Whoo untuk membuat sistem yang bisa melakukan tracking pada seluruh aktivitas dan keadaan seseorang. 

#### a. File `system.c` dan `hunter.c`

Author membuat 2 buah file, yaitu `system.c` dan `hunter.c`, dimana `system.c` merupakan shared memory utama yang mengelola shared memory hunter-hunter dari `hunter.c`. Selain itu, author juga diberikan file `clue.h` untuk membuat pekerjaannya menjadi lebih mudah dan efisien.

```
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
```

Ini merupakan file `clue.h` yang diberikan oleh Sung Jin Woo untuk mempermudah pekerjaan. Sung Jin Woo juga memberikan note : **hunter bisa dijalankan ketika sistem sudah dijalankan.**. 

#### b. Registrasi dan Login

Author diberikan tugas oleh Sung Jin Woo untuk membuat fitur registrasi dan login di program hunter. Setiap hunter akan memiliki key unik dan stats awal (Level=1, EXP=0, ATK=10, HP=100, DEF=5). Data hunter disimpan dalam shared memory tersendiri yang terhubung dengan sistem.

```
//BAGIAN HUNTER MENU
if (!loggedIn) {
    printf("\n===== HUNTER LOGIN =====\n1. Register\n2. Login\n3. Exit\nChoice : ");
    scanf("%d", &choice); getchar();

    if (choice == 1) {
        // Proses registrasi
    }
    else if (choice == 2) {
        // Proses login
    }
    else if (choice == 3) {
        // Keluar dari aplikasi
    }
    else {
        printf("Invalid choice.\n");
    }
}
```

```
//BAGIAN HUNTER SYSTEM
else {
    printf("\n===== HUNTER SYSTEM =====\n===== %s's MENU =====\n", myUsername);
    printf("1. List Dungeon\n2. Raid\n3. Battle\n4. Toggle Notification\n5. Exit\nChoice : ");
    scanf("%d", &choice); getchar();

    if (choice == 1) {
        // Tampilkan dungeon yang tersedia
    }
    else if (choice == 2) {
        // Melakukan raid dungeon
    }
    else if (choice == 3) {
        // Battle antar hunter
    }
    else if (choice == 4) {
        // Toggle notifikasi dungeon
    }
    else if (choice == 5) {
        // Logout dan exit
    }
    else {
        printf("Invalid choice.\n");
    }
}
```
Program ini berada pada sistem `hunter.c`

Screenshot `hunter_menu` :

![Image](https://github.com/user-attachments/assets/63deef77-530e-41bc-891d-655f223c6deb)

Screenshot `hunter_system`: 

![Image](https://github.com/user-attachments/assets/b20c5f37-37c0-4f87-812b-8df9ed289d1f)

#### c. Hunter Info

Author diperintahkan oleh Sung Jin Woo untuk menambahkan fitur di sistem yang dapat menampilkan informasi semua hunter yang terdaftar, termasuk nama hunter, level, exp, atk, hp, def, dan status (banned atau tidak). Ini membuat dia dapat melihat siapa hunter terkuat dan siapa yang mungkin melakukan kecurangan. Berikut program yang menjalankan hunter info : 

```
printf("\n-- Hunter Info --\n");
for (int i = 0; i < systemData->num_hunters; i++) {
    struct Hunter *h = &systemData->hunters[i];
    printf("Username: %s | Level: %d | EXP: %d | ATK: %d | HP: %d | DEF: %d | Banned: %s\n",
            h->username, h->level, h->exp, h->atk, h->hp, h->def,
            h->banned ? "Yes" : "No");
    }
 if (systemData->num_hunters == 0) printf("(No hunter registered)\n");
```
Potongan kode tersebut merupakan bagian dari menu admin (di `system.c`) yang berfungsi untuk menampilkan informasi semua hunter yang telah terdaftar dalam sistem. Saat admin memilih opsi ini, program akan mengunci akses ke shared memory menggunakan `lock()` untuk mencegah kondisi balapan dengan proses lain. Kemudian, program melakukan iterasi melalui array `systemData->hunters` dan mencetak detail setiap hunter, seperti username, level, exp, attack (ATK), health (HP), defense (DEF), serta status apakah hunter tersebut sedang dibanned atau tidak. Setelah semua data ditampilkan, `unlock()` dipanggil untuk melepaskan kunci shared memory. Jika belum ada hunter yang terdaftar (`num_hunters == 0`), maka akan ditampilkan pesan bahwa belum ada hunter yang tersedia.

Screenshot `hunter_info` : 

![Image](https://github.com/user-attachments/assets/89369565-bdd9-48bf-ae79-cb9a6d7fa3d9)

Pada output program `hunter_info` menampilkan statistik dari hunter **Yudi**, dimana hunter **Yudi** merupakan hunter yang baru diregistrasi, sehingga mendapatkan stats awal `Level=1, EXP=0, ATK=10, HP=100, DEF=5`. 

#### d. Generator Dungeon Random

Sung Jin Woo menyadari bahwa para hunter membutuhkan tempat untuk berlatih dan memperoleh pengalaman. Ia memutuskan untuk menyuruh author membuat fitur unik dalam sistem yang dapat menghasilkan dungeon secara random dengan nama, level minimal hunter, dan stat rewards dengan nilai:

🏆Level Minimal : 1 - 5

⚔️ATK : 100 - 150 Poin

❤️HP  : 50 - 100 Poin

🛡️DEF : 25 - 50 Poin

🌟EXP : 150 - 300 Poin

Setiap dungeon akan disimpan dalam shared memory sendiri yang berbeda dan dapat diakses oleh hunter. Berikut merupakan nama dungeon yang author gunakan dan program akan mengambil secara acak : 
- Double Dungeon
- Demon Castle
- Pyramid Dungeon
- Red Gate Dungeon
- Hunters Guild Dungeon
- Busan A-Rank Dungeon
- Insects Dungeon
- Goblins Dungeon
- D-Rank Dungeon
- Gwanak Mountain Dungeon
- Hapjeong Subway Station Dungeon

```
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
```

Potongan program ini berada dalam `system.c` dan digunakan untuk membuat dungeon baru secara acak jika jumlah dungeon belum mencapai batas maksimum (`MAX_DUNGEONS`). Nama dungeon dipilih dari daftar, dan statistiknya (level minimum, ATK, HP, DEF, EXP) dihasilkan secara acak sesuai rentang yang ditentukan. Setiap dungeon juga diberi key unik menggunakan kombinasi waktu dan angka acak. Setelah dungeon dibuat, jumlah dungeon bertambah, dan informasi dungeon ditampilkan. Jika kapasitas sudah penuh, sistem memberi peringatan. 

Screenshot `generate_dungeon` : 

![Image](https://github.com/user-attachments/assets/269da3c9-3054-41e4-871b-8ef0590da86f)

#### e. Dungeon Info

Author diperintahkan oleh Sung Jin Woo untuk  menambahkan fitur yang menampilkan informasi detail semua dungeon. Fitur ini menampilkan daftar lengkap dungeon beserta nama, level minimum, reward (EXP, ATK, HP, DEF), dan key unik untuk masing-masing dungeon. Berikut program untuk `dungeon_info` : 

```
printf("\n-- Dungeon Info --\n");
        for (int i = 0; i < systemData->num_dungeons; i++) {
        struct Dungeon *d = &systemData->dungeons[i];
        printf("\nName: %s\nMin Level: %d\nATK: %d\nHP: %d\nDEF: %d\nEXP: %d\nKEY: %lu\n",
                d->name, d->min_level, d->atk, d->hp, d->def, d->exp, d->key);
        }
        if (systemData->num_dungeons == 0) printf("(No dungeons available)\n");
```

Potongan kode ini terdapat dalam `system.c` dan berfungsi untuk menampilkan informasi detail semua dungeon yang telah dibuat di dalam sistem. Program akan menelusuri array dungeon dalam shared memory dan mencetak informasi seperti nama dungeon, level minimum yang dibutuhkan, nilai ATK, HP, DEF, EXP, serta key unik yang dimiliki setiap dungeon. Jika tidak ada dungeon yang tersedia (`num_dungeons == 0`), maka akan ditampilkan pesan bahwa dungeon belum tersedia. Fungsi ini membantu admin memantau seluruh dungeon aktif yang dapat diakses oleh para hunter.

Screenshot `dungeon_info` : 

![Image](https://github.com/user-attachments/assets/74f48b05-ae21-41c6-b374-965ba293475b)

#### f. Dungeon List pada `hunter.c`

Sung Jin Woo menambahkan fitur yang menampilkan semua dungeon yang tersedia sesuai dengan level hunter. Disini, hunter hanya dapat menampilkan dungeon dengan level minimum yang sesuai dengan level mereka. Berikut merupakan kode program `dungeon_list` : 

```
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
```

Potongan kode ini terdapat pada `hunter.c` dan digunakan oleh hunter untuk melihat daftar dungeon yang tersedia sesuai dengan level mereka. Program akan membandingkan level hunter saat ini dengan level minimum setiap dungeon. Jika level hunter memenuhi syarat, maka informasi dungeon akan ditampilkan. Jika tidak ada dungeon yang bisa diakses berdasarkan level, maka akan muncul pesan bahwa tidak ada dungeon yang tersedia. Fitur ini memastikan hunter hanya dapat melihat dan mengakses dungeon yang sesuai dengan kemampuan mereka.

Screenshot `dungeon_list` : 

![Image](https://github.com/user-attachments/assets/422f891c-0c10-44c7-8df1-cff3c17a69ac)

#### g. Dungeon Raid

Sung Jin Woo melihat beberapa hunter terlalu kuat, sehingga author diberikan tugas untuk membuat fitur menguasai dungeon. Ketika hunter berhasil menaklukan sebuah dungeon, dungeon tersebut akan menghilang dari sistem dan hunter akan mendapatkan stat rewards dari dungeon. Jika exp hunter mencapai 500, mereka akan naik level dan exp kembali ke 0. Berikut kode program untuk `dungeon_raid` : 

```
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
        printf("%d. %s (Level %d+)\n", count + 1,
            systemData->dungeons[i].name,
            systemData->dungeons[i].min_level);
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
```

Potongan kode ini berada pada `hunter.c` dan bertugas untuk menangani proses raid dungeon oleh hunter. Program akan menampilkan semua dungeon yang bisa diakses berdasarkan level hunter saat ini. Hunter kemudian memilih dungeon yang ingin di-raid. Jika pilihan valid, hunter akan mendapatkan EXP dan peningkatan statistik (ATK, HP, DEF) sesuai dengan reward dungeon. Jika EXP mencapai 500, hunter naik level dan EXP di-reset ke 0. Setelah raid selesai, dungeon yang telah diselesaikan akan dihapus dari daftar dungeon yang tersedia.

Screenshot `dungeon_raid` : 

![Image](https://github.com/user-attachments/assets/14d0ca7f-de2d-4805-bab6-9cb692395a84)

#### h. Hunter Battle

Sung Jin Woo memberikan perintah kepada author untuk membuat fitur dimana hunter dapat memilih untuk bertarung dengan hunter lain. Tingkat kekuatan seorang hunter bisa dihitung melalui total stats yang dimiliki hunter tersebut (ATK+HP+DEF). Jika hunter menang, maka hunter tersebut akan mendapatkan semua stats milik lawan dan lawannya akan terhapus dari sistem. Jika kalah, hunter tersebutlah yang akan dihapus dari sistem dan semua statsnya akan diberikan kepada hunter yang dilawannya. Berikut kode program dari `hunter_battle` : 

```
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
            
int myPower = systemData->hunters[myIndex].atk + systemData->hunters[myIndex].hp + systemData->hunters[myIndex].def;
int oppPower = systemData->hunters[opp].atk + systemData->hunters[opp].hp + systemData->hunters[opp].def;

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
```

Potongan kode di atas terdapat pada `hunter.c` dan memiliki tugas untuk menangani fitur pertarungan (PVP) antar pemain dalam sistem berbasis shared memory. Pertama, program memeriksa apakah pemain saat ini dibanned. Jika tidak, akan ditampilkan daftar lawan (kecuali dirinya sendiri) beserta total kekuatannya (gabungan dari atk, hp, dan def). Setelah pemain memilih lawan dengan mengetikkan username, sistem menghitung dan membandingkan total kekuatan kedua pemain. Jika pemain menang, ia mengambil seluruh statistik lawan, dan lawan dihapus dari sistem. Jika kalah, lawan yang mengambil statistiknya dan pemain keluar dari permainan. Jika seri, tidak ada perubahan status. Kode juga menangani sinkronisasi data dengan `lock()` dan `unlock()` agar data tetap konsisten saat diakses oleh beberapa proses.

Screenshot `hunter_battle` : 

![image](https://github.com/user-attachments/assets/f1dffdec-3084-44da-af03-2f1d8572238a)

Pada gambar diatas, hunter Yudi memenangkan battle dengan hunter Nandanjaya, dikarenakan hunter Yudi memiliki power yang lebih banyak (ATK: 151 | HP: 190 | DEF: 38 | Power Total: 379) sedangkan hunter Nandanjaya memiliki power lebih sedikit (ATK: 159 | HP: 166 | DEF: 35 | Power Total: 360). 

#### i. Ban Hunter

Sung Jin Woo melihat beberapa hunter melakukan kecurangan di dalam sistem. Author diberikan tugas untuk menambahkan fitur di sistem yang membuat dia dapat melarang hunter tertentu untuk melakukan raid atau battle. Karena masa percobaan tak bisa berlangsung selamanya, Sung Jin Woo pun juga menambahkan konfigurasi agar fiturnya dapat memperbolehkan hunter itu melakukan raid atau battle lagi. Berikut merupakan kode program dari `ban_hunter` : 

```
printf("Enter hunter username to ban: ");
fgets(name, 50, stdin); name[strcspn(name, "\n")] = 0;
int idx = findHunter(name);
if (idx >= 0) {
    systemData->hunters[idx].banned = 1;
    printf("Hunter '%s' has been banned.\n", name);
} else {
    printf("Hunter '%s' not found.\n", name);
}
```

Potongan kode ini terdapat pada `system.c` digunakan untuk **membanned hunter** berdasarkan username. Pertama, program meminta input nama hunter dari pengguna (`fgets`). Lalu, nama tersebut dicari dalam daftar hunter menggunakan fungsi `findHunter`. Jika hunter ditemukan (`idx >= 0`), maka nilai atribut `banned` pada hunter tersebut diatur menjadi `1`, menandakan bahwa akun dibanned. Setelah itu, program mencetak konfirmasi bahwa hunter telah dibanned. Jika tidak ditemukan, maka ditampilkan pesan bahwa hunter tidak ditemukan. 

Screenshot `ban_hunter` : 

![image](https://github.com/user-attachments/assets/916a6194-ded0-43c3-a240-6d7e2f8b658d)

![image](https://github.com/user-attachments/assets/32c2a247-01eb-4d7d-8625-46378fd6563d)

Disini terbukti bahwa hunter Yudi tidak dapat mengakses Raid, dikarenakan di ban oleh admin. 

![image](https://github.com/user-attachments/assets/7ba3e27d-bdcc-4938-92c6-b258cf9c497b)

![image](https://github.com/user-attachments/assets/a7a5f606-344c-4144-8c80-842e967bffb6)

Setelah melakukan reset, hunter Yudi akhirnya dapat mencoba fitur raid lagi. 

#### j. Reset Hunter

Sung Jin Woo memberikan kesempatan kedua bagi hunter yang ingin bertobat dan memulai dari awal, Sung Jin Woo juga menambahkan fitur di sistem yang membuat dia bisa mengembalikan stats hunter tertentu ke nilai awal. Berikut merupakan kode program `reset_hunter` : 

```
printf("Enter hunter username to reset: ");
fgets(name, 50, stdin); name[strcspn(name, "\n")] = 0;
int idx = findHunter(name);
if (idx >= 0) {
    struct Hunter *h = &systemData->hunters[idx];
    h->level = 1; h->exp = 0; h->atk = 10; h->hp = 100; h->def = 5; h->banned = 0;
    printf("Stats for hunter '%s' have been reset.\n", name);
} else {
    printf("Hunter '%s' not found.\n", name);
}
```

Potongan kode ini terdapat di `system.c` digunakan untuk **mereset status dan statistik hunter** berdasarkan username yang dimasukkan. Program pertama-tama meminta input nama hunter (`fgets`), lalu mencari hunter tersebut dengan `findHunter`. Jika ditemukan (`idx >= 0`), maka atribut hunter di-reset ke nilai awal: level 1, exp 0, atk 10, hp 100, def 5, dan status banned di-nonaktifkan (`banned = 0`). Ini memberikan kesempatan kedua bagi hunter. Jika tidak ditemukan, akan ditampilkan pesan bahwa hunter tidak ada.

Screenshot `reset_hunter` : 

![Screenshot 2025-04-30 012138](https://github.com/user-attachments/assets/450654a0-dbc6-4e91-be38-c5aeebfe9173)

![image](https://github.com/user-attachments/assets/28e01fd0-c0c3-4124-a3c6-1882819a1dd3)

#### j. Notification

Sung Jin Woo merasa bosan dengan sistemnya, Sung Jin Woo menambahkan fitur notifikasi dungeon di setiap hunter. Saat diaktifkan, akan muncul informasi tentang dungeon yang tersedia untuk hunter tersebut dan akan terus berganti setiap 3 detik. Berikut merupakan kode program `notification` : 

```
void* notification(void *arg) {
    while (runNotif) {
        sleep(3); // tunggu 3 detik sebelum menampilkan ulang
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
```

Fungsi `notification()` terdapat pada `hunter.c` dan menampilkan daftar dungeon yang bisa diakses hunter berdasarkan level-nya, dan dijalankan setiap 3 detik saat notifikasi aktif. Fitur ini dijalankan di thread terpisah dan hanya aktif jika hunter memilih opsi **Toggle Notification** di menu.

Screenshot `notification` : 

![image](https://github.com/user-attachments/assets/8fcdaae1-3c33-4dd0-a7fc-0c2decf554db)

Ini merupakan notifikasi dimana pada awalnya tidak terdapat dungeon random, dan setelah itu author mengenerate dungeon secara random dan muncul pada notifikasi. 

#### l. Delete Shared Memory

Untuk menambah keamanan sistem agar data hunter tidak bocor, Sung Jin Woo melakukan konfigurasi agar setiap kali sistem dimatikan, maka semua shared memory yang sedang berjalan juga akan ikut terhapus. 

```
void cleanup(int signo) {
    if (systemData != (void*) -1) shmdt(systemData);                   // Melepas pointer dari shared memory
    if (shmid != -1) shmctl(shmid, IPC_RMID, NULL);                    // Menghapus shared memory dari sistem
    if (semid != -1) semctl(semid, 0, IPC_RMID);                       // Menghapus semaphore
    printf("\nShared memory and semaphores have been removed. The system is shutting down.\n");
    exit(0);
}
```

Pada program `system.c`, penghapusan shared memory dan semaphore dilakukan melalui fungsi `cleanup()`, yang akan dipanggil secara otomatis saat program dihentikan, seperti saat menerima sinyal `SIGINT` (misalnya dari `Ctrl+C`). Fungsi ini pertama-tama melepaskan shared memory dari proses menggunakan `shmdt()`, memastikan bahwa proses tidak lagi terhubung dengan memori bersama. Setelah itu, `shmctl()` dengan perintah `IPC_RMID` digunakan untuk menghapus segmen shared memory secara permanen dari sistem. Terakhir, semaphore yang digunakan untuk mengatur sinkronisasi antar proses juga dihapus menggunakan `semctl()` dengan perintah `IPC_RMID`. Dengan mekanisme ini, resource yang digunakan oleh sistem dapat dibersihkan dengan baik setelah program selesai dijalankan.

Screenshot `delete_shared_memory` : 

![image](https://github.com/user-attachments/assets/61b63a23-60ae-4368-9eba-01ad9485283f)

Kendala : Tidak terdapat kendala pada saat mengerjakan soal ini. 
