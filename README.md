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


