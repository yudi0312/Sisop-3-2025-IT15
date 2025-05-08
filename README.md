# Praktikum Sisop Modul 3-2025-IT15

Anggota kelompok : 
- Putu Yudi Nandanjaya Wiraguna	5027241080
- Naruna Vicranthyo Putra Gangga	5027241105
- Az Zahrra Tasya Adelia	        5027241087

---

## Soal 1 - 'Rootkids'
Author : Putu Yudi Nandanjaya Wiraguna (5027241080)

### Deskripsi
Author diberikan tugas untuk membuat sistem RPC server-client untuk mengubah text file sehingga bisa dilihat dalam bentuk file jpeg.
- `image_server.c` sebagai program yang berjalan pada latar belakang
- `iamge_client.c` sebagai menu atau mengirim request ke `image_Server.c`
---
### a. download/unzip file yang terdapat pada soal

download file dan unzip file tersebut. selanjutnya, memperlihatkan working directory:

![WhatsApp Image 2025-05-08 at 16 42 37_fe6ed71d](https://github.com/user-attachments/assets/611adfe7-be0c-49aa-b458-8c6bfa666f47)

### b. Pada `image_server.c`, program yang dibuat harus berjalan secara daemon di background dan terhubung dengan `image_client.c` melalui socket RPC.

```
void daemonize() {
    pid_t pid = fork(); // Fork process
    if (pid > 0) exit(EXIT_SUCCESS); // Parent process exit

    setsid(); // Membuat session ID baru
    chdir("/path/server/"); // Mengubah working directory
    close(STDIN_FILENO); // Menutup file descriptor standar
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main() {
    daemonize(); // Memanggil fungsi daemonize
    // Loop utama server
    while (1) { ... }
}

```
- Loop while (1) menjaga server tetap aktif meskipun ada error.

### c. Program image_client.c harus bisa terhubung dengan image_server.c dan bisa mengirimkan perintah untuk:
- Decrypt text file yang dimasukkan dengan cara Reverse Text lalu Decode from Hex, untuk disimpan dalam folder database server dengan nama file berupa timestamp dalam bentuk angka
- Request download dari database server sesuai filename yang dimasukkan:

![WhatsApp Image 2025-05-08 at 16 48 34_50e11bcb](https://github.com/user-attachments/assets/cb0b3ab5-ff1f-4e90-8178-3e607aafc38c)

1. Decrypt 
- Client
```
// Mengirim perintah DECRYPT ke server
snprintf(request, sizeof(request), "DECRYPT %s", text);
send(sock, request, strlen(request), 0);
```

- Server
```
if (strncmp(buffer, "DECRYPT ", 8) == 0) {
    reverse(text_data); // Reverse teks
    hex_to_bytes(text_data, decoded); // Decode hex ke binary
    sprintf(filename, "database/%ld.jpeg", now); // Generate timestamp
    fwrite(decoded, 1, decoded_len, fp); // Simpan sebagai JPEG
}
```

2. Download
- Client
```
// Mengirim perintah DOWNLOAD ke server
snprintf(request, sizeof(request), "DOWNLOAD %s", filename);
send(sock, request, strlen(request), 0);
```

- Server
```
if (strncmp(buffer, "DOWNLOAD ", 9) == 0) {
    // Membaca file dari database dan mengirim ke client
    FILE *fp = fopen(path, "rb");
    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        write(new_socket, buffer, n);
    }
}
```

### d. Menu Interaktif pada Client
disajikan dalam bentuk menu yang memperbolehkan pengguna untuk memasukkan perintah berkali-kali.

![WhatsApp Image 2025-05-08 at 16 43 27_5757f20e](https://github.com/user-attachments/assets/300eae94-fd60-42f7-b4f0-955f965bb23d)

```
void print_menu() {
    printf("\n=== MENU ROOTKIDS ===\n");
    printf("1. Decrypt text file to image\n");
    printf("2. Download image from server\n");
    printf("3. Exit\n");
}

int main() {
    while (1) { // Loop menu
        print_menu();
        scanf("%d", &choice);
        switch (choice) { ... } // Pilih aksi
    }
}
```

### e. Program dianggap berhasil bila pengguna dapat mengirimkan text file dan menerima sebuah file jpeg yang dapat dilihat isinya. Apakah anda akan berhasil menemukan sosok sang legenda “rootkids”?

berikut contoh dari gambar 'input_1.txt':

![image](https://github.com/user-attachments/assets/3f00bbd0-a6b7-4682-8518-e6a8e7bb0eee)


Server:
```
// Menyimpan hasil dekripsi sebagai binary (JPEG)
FILE *fp = fopen(filename, "wb");
fwrite(decoded, 1, decoded_len, fp);
```

Client:
```
// Menyimpan file hasil download sebagai binary
FILE *fp = fopen(filepath, "wb");
fwrite(response, 1, bytes_received, fp);
```
-File disimpan dalam mode binary (wb/rb), sehingga format JPEG tetap valid.

### f. eror handling

- Gagal Koneksi (image_client.c):
```
if (connect(sock, ...) < 0) {
    perror("Connection failed");
}
```

- File Input Tidak Ditemukan (image_client.c):
```
FILE *fp = fopen(fullpath, "r");
if (!fp) printf("Error: Text file not found.\n");
```

- File Tidak Ditemukan di Server (image_server.c):
```
FILE *fp = fopen(path, "rb");
if (!fp) write(new_socket, "ERROR: File not found\n", 23);
```

contoh output untuk eror handling ketika salah input nama file text:

![WhatsApp Image 2025-05-08 at 16 59 17_867dfc59](https://github.com/user-attachments/assets/9972bbd3-c52e-410d-8f16-de2d85268c22)

### g. Logging
Server menyimpan log semua percakapan antara image_server.c dan image_client.c di dalam file server.log dengan format:

![image](https://github.com/user-attachments/assets/07c8e175-9b80-4713-ad00-20400379d903)

berikut hasil output pada server.log:

![WhatsApp Image 2025-05-08 at 16 43 52_8e90f370](https://github.com/user-attachments/assets/4228724d-36bf-4d53-a9bd-7d34942d1910)

```
void write_log(const char *source, const char *action, const char *info) {
    // Format: [Source][Timestamp]: [ACTION] [Info]
    fprintf(log, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
}

// Contoh penggunaan log:
write_log("Client", "DECRYPT", "Text data received");
write_log("Server", "SAVE", "1744399397.jpeg");
```

kendala: 

![Screenshot 2025-05-03 230525](https://github.com/user-attachments/assets/63972790-77cc-4e69-a52d-0ac5e7ca939a)

![Screenshot 2025-05-03 010757](https://github.com/user-attachments/assets/198e6d2e-80a8-44fb-9771-04af0d8948d9)

sempat beberapa kali gagal dalam deckrip dan download file text. dan juga ada eror pada connecting to RPC. 

---

## Soal 2 - RushGo

> Soal ini ada revisi pada bagian `./dispatcher -list`

Author : Az Zahrra Tasya Adelia (5027241087)

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

#### e. Melihat daftar semua pesanan (revisi)

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

![image](https://github.com/user-attachments/assets/7bb747fd-71df-40c7-8509-db922861bac5)

Kendala : Pada bagian `./dispatcher -list`, ketika program `./delivery_agent` dijalankan output dari `./dispatcher -list` hanya pending saja semua. 

---

## Soal 3 - The Lost Dungeon
> Soal ini terdapat revisi pada bagian battle mode.

Author : Naruna Vicranthyo Putra Gangga (5027241105)

### Deskripsi
Author diberikan tugas untuk membuat yang terdiri dari dua komponen utama:
- dungeon.c sebagai server yang menjalankan seluruh logika permainan.
- player.c sebagai client yang berinteraksi dengan pemain melalui koneksi socket TCP/IP.
Pemain dapat terhubung ke server untuk menelusuri dungeon, membeli senjata, mengecek status, serta bertarung melawan musuh.

---

## Entering the Dungeon

File `dungeon.c` berperan sebagai **server** yang menerima koneksi dari **client** (`player.c`) melalui komunikasi socket. Semua perintah dikirim dari `player.c` ke `dungeon.c`, lalu diproses oleh server.

> Server mendukung **multi-client** (lebih dari satu pemain dapat bermain secara bersamaan).
```C
//dungeon.c
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
```
---

## Sightseeing

Saat `player.c` dijalankan, pemain akan terhubung ke server dan ditampilkan menu utama (main menu). Menu ini mencakup pilihan:

1. Show Player Stats  
2. Shop (Buy Weapons)  
3. View Inventory & Equip Weapons  
4. Battle Mode  
5. Exit Game

Menu ditampilkan dalam tampilan terminal interaktif menggunakan warna (ANSI escape codes).
```C
//player.c
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
```

![image](https://github.com/user-attachments/assets/b3c9c25d-3326-4596-ab6b-85732cc6a474)

---

## Status Check

Jika opsi **Show Player Stats** dipilih, maka akan ditampilkan informasi berikut:

- Jumlah gold yang dimiliki
- Senjata yang sedang digunakan
- Base Damage
- Jumlah musuh yang telah dikalahkan
- Jika senjata memiliki **Passive**, akan ditampilkan juga efek tersebut
```C
//dungeon.c
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
```

![image](https://github.com/user-attachments/assets/f32dd46f-f6e8-4ecc-b2a7-b5886ed0c159)

---

## Weapon Shop

Saat opsi **Shop** dipilih, pemain akan melihat daftar senjata yang dapat dibeli. Setiap senjata memiliki:

- Nama
- Harga (dalam gold)
- Damage
- Passive (jika ada)

> Terdapat **minimal 5 senjata**, dan **minimal 2** di antaranya memiliki efek **Passive** unik (contoh: critical chance, insta kill).

Semua logika dan data senjata terdapat di dalam `shop.h`, yang digunakan oleh `dungeon.c` (tanpa membutuhkan `shop.c` lagi).
```C
//dungeon.c
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
```

![image](https://github.com/user-attachments/assets/f9dfbbc7-b778-4a79-be9c-d154fbe0352c)

---

## Handy Inventory

Setelah membeli senjata, pemain dapat memilih **View Inventory & Equip Weapons** untuk:

- Melihat semua senjata yang dimiliki
- Mengganti senjata yang sedang digunakan

Senjata yang sedang digunakan akan mempengaruhi output pada **Show Player Stats**, termasuk damage dan passive.
```C
//dungeon.c
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
```

![image](https://github.com/user-attachments/assets/e6c60c40-7b3f-4d9b-b0f6-84369045cbbd)

---

## Enemy Encounter

Jika pemain memilih **Battle Mode**, maka:

- Musuh akan muncul dengan jumlah darah (HP) acak, misalnya 50–200 HP
- Ditampilkan health-bar visual dan angka HP
- Pemain dapat memasukkan perintah:
  - `attack` → untuk menyerang
  - `exit` → untuk keluar dari Battle Mode

Setelah musuh dikalahkan, pemain akan mendapatkan **reward gold acak**, dan musuh baru akan muncul.
```C
//dungeon.c
void battle(struct Player *p, char *buffer, int clientSock) {
    while (1) {
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
                break;
            }

            send(clientSock, msg, strlen(msg), 0);
        }
    }
}
```

![image](https://github.com/user-attachments/assets/9a8df8e7-49d8-4292-9b5d-cf31c82beaa4)

---

## Other Battle Logic

### Damage Equation

- Damage dasar ditentukan dari base damage senjata
- Variasi damage ditambahkan dengan bilangan acak
- Terdapat kemungkinan **Critical Hit** yang menggandakan damage

### Passive

Jika senjata memiliki efek Passive, efek tersebut akan aktif sesuai logika masing-masing. Contoh:

- Passive: `+30% crit chance` → meningkatkan peluang critical hit
- Passive: `+10% insta kill` → memiliki peluang untuk langsung mengalahkan musuh

Saat passive aktif, akan ditampilkan notifikasi efek tersebut di terminal.
```C
//dungeon.c
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
```
---

## Error Handling

Semua input dari pemain akan divalidasi. Jika input tidak valid (misal: memilih menu yang tidak ada atau salah memasukkan indeks senjata), maka akan ditampilkan pesan kesalahan yang sesuai.

---

## Struktur akhir
![image](https://github.com/user-attachments/assets/d9fa6824-c0ed-4ab8-9cc9-38f28180cdc7)


Kendala: Pada bagian `battle mode`, dimana yang seharusnya muncul enemy baru setelah mengalahkan enemy, melainkan malah balik ke main menu.

---
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
