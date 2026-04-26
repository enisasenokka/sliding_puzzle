// W / S / A / D : bos hucreyi hareket ettir
// Q : cikis
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOYUT 3
#define HUCRE_SAYISI 9  //9 hucre
#define MAX_DURUM 362880  //9! = tum farkli dizilis sayisi 
#define TABLO_BOYUTU 400007  //Hash tablosu hizli calismasi icin asal ve buyuk secildi.


//BFS'te her tahta durumu bir dugum olarak tutulur.
typedef struct {
    int dizi[HUCRE_SAYISI]; //tahtanin 1 boyutlu hali
    int ebeveyn; //bu dugumu kuyruga ekleyen dugumun indeksi.-1 baslangic durumudur.
    char adim;
} AramaDugumu;

int tahta[BOYUT][BOYUT];
char son_ipucu = ' ';
int hamle_sayisi;
time_t oyun_baslangici;
int hedefDurum[HUCRE_SAYISI] = {1, 2, 3, 4, 5, 6, 7, 8, 0};


AramaDugumu aramaSirasi[MAX_DURUM]; //BFS kuyrugu => tum dugumler burda olur
int sira_basi, sira_sonu;  //kuyrugun basi ve sonu 

long long ziyaret_tablosu[TABLO_BOYUTU]; 
int slot_dolu[TABLO_BOYUTU]; // o slot doluysa 1, bossa 0 


void ikiDenBire(int kaynak[BOYUT][BOYUT], int hedef_dizi[HUCRE_SAYISI]) //Oyun tahtasi 2D dizi olarak tutulur
{
    for (int i = 0; i < BOYUT; i++)
        for (int j = 0; j < BOYUT; j++)
            hedef_dizi[i * BOYUT + j] = kaynak[i][j];
}

void birdenIkiye(int kaynak[HUCRE_SAYISI], int hedef_dizi[BOYUT][BOYUT])
{
    for (int i = 0; i < BOYUT; i++)
        for (int j = 0; j < BOYUT; j++)
            hedef_dizi[i][j] = kaynak[i * BOYUT + j]; //Donusum formulu: 1D indeks = satir * 3 + sutun
}

void tahtayiYazdir()
{
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    printf("\n  \033[1;35m========= PUZZLE =========\033[0m\n\n");

    for (int i = 0; i < BOYUT; i++) {
        printf("  "); 

    
        // 1. Satır: Sayı ve Kutunun üst yarısı
        
        for (int j = 0; j < BOYUT; j++) {
            if (tahta[i][j] == 0) {
                // Boş hücre: Daha küçük gri bir yuva
                printf("\033[47m    \033[0m "); 
            } else {
                // Pembe blok: Sayı içinde
                printf("\033[1;37;45m  %d \033[0m ", tahta[i][j]);
            }
        }
        printf("\n  "); // Alt satıra geç

        // 2. Satır: Kutuların alt yarısı (boşluk bırakarak yüksekliği tamamlar)
        for (int j = 0; j < BOYUT; j++) {
            if (tahta[i][j] == 0) {
                printf("     "); // Boş hücrenin altı tamamen boş
            } else {
                printf("\033[45m    \033[0m "); // Pembe bloğun alt dolgusu
            }
        }
        printf("\n\n"); // Karolar arası dikey boşluk
    }
    printf("  \033[1;35m==========================\033[0m\n");


    if (son_ipucu != ' ' && son_ipucu != '?') {
        printf("\n  \033[1;33m[IPUCU]: Siradaki hamle -> %c\033[0m\n", son_ipucu);
    } else if (son_ipucu == '?') {
        printf("\n  \033[1;31m[IPUCU]: Cozum bulunamadi!\033[0m\n");
    }
}

void bosHucreyiBul(int arama_tahtasi[BOYUT][BOYUT], int *satir, int *sutun)
{
    for (int i = 0; i < BOYUT; i++)
        for (int j = 0; j < BOYUT; j++)
            if (arama_tahtasi[i][j] == 0) {
                *satir = i;
                *sutun = j;
                return;
            }
}

int hamleYap(char yon, int sessiz_mod)
{
    int satir, sutun, yeni_satir, yeni_sutun;
    bosHucreyiBul(tahta, &satir, &sutun);

    //yeni konum hesaplanir:
    yeni_satir = satir;
    yeni_sutun = sutun;

    if      (yon == 'W' || yon == 'w') yeni_satir = satir - 1;
    else if (yon == 'S' || yon == 's') yeni_satir = satir + 1;
    else if (yon == 'A' || yon == 'a') yeni_sutun = sutun - 1;
    else if (yon == 'D' || yon == 'd') yeni_sutun = sutun + 1;
    else {
        if (!sessiz_mod) printf("  [!] Gecersiz tus! W/A/S/D kullanin.\n");
        return 0;
    }

    //Sinir kontolu:
    if (yeni_satir < 0 || yeni_satir >= BOYUT ||
        yeni_sutun < 0 || yeni_sutun >= BOYUT) {
        if (!sessiz_mod) printf("  [!] Bu yonde tas yok!\n");
        return 0;
    }

    //Bos hucre ile komsu degisikligi
    int gecici           = tahta[satir][sutun];
    tahta[satir][sutun]  = tahta[yeni_satir][yeni_sutun];
    tahta[yeni_satir][yeni_sutun] = gecici;

    return 1;
}

int kazandiMi()
{
    int dizi[HUCRE_SAYISI];
    ikiDenBire(tahta, dizi);
    for (int i = 0; i < HUCRE_SAYISI; i++)
        if (dizi[i] != hedefDurum[i]) return 0;
    return 1;
}


//Cozulebilirlik kontolu:
//Sol taraftaki sayi, sag taraftaki sayidan buyukse buna "inversiyon" denir. inversiyon sayisi cifste cozulebilir
int cozulebilirMi(int dizi[HUCRE_SAYISI])
{
    int inversiyon_sayisi = 0;
    for (int i = 0; i < HUCRE_SAYISI - 1; i++) {
        if (dizi[i] == 0) continue; //bos hucre atlanir
        for (int j = i + 1; j < HUCRE_SAYISI; j++) {
            if (dizi[j] == 0) continue;
            if (dizi[i] > dizi[j])
                inversiyon_sayisi++;
        }
    }

    return (inversiyon_sayisi % 2 == 0); 
}

//HASH TABLOSU FONKSIYONLARI:bir durumun daha once ziyaret edilip edilmedigini kontrol eder.
long long durumKodunuHesapla(int dizi[HUCRE_SAYISI])
{
    long long kod = 0;
    for (int i = 0; i < HUCRE_SAYISI; i++)
        kod = kod * 10 + dizi[i]; // her adimda bir haneyi sola kaydirir ve ekler 
    return kod;
}

//kodu tablo boyutuna boler kalanini slot olarak kullanir:
int slotBul(long long deger)
{
    int slot = (int)(deger % TABLO_BOYUTU);
    if (slot < 0) slot += TABLO_BOYUTU;

    while (slot_dolu[slot] && ziyaret_tablosu[slot] != deger) //slot doluysa bir sonrakine bakilir
        slot = (slot + 1) % TABLO_BOYUTU;

    return slot;
}

int ziyaretEdildiMi(long long deger)
{
    int slot = slotBul(deger);
    return slot_dolu[slot] && ziyaret_tablosu[slot] == deger;
}

void ziyaretOlarakIsaretle(long long deger)
{
    int slot = slotBul(deger);
    slot_dolu[slot]        = 1;
    ziyaret_tablosu[slot]  = deger;
}
char bfsIpucu()
{
    char yonSecenekleri[4] = {'W', 'S', 'A', 'D'};

    //veriler temizlenir:
    memset(slot_dolu, 0, sizeof(slot_dolu)); 
    sira_basi = 0;
    sira_sonu = 0;

    ikiDenBire(tahta, aramaSirasi[0].dizi);
    aramaSirasi[0].ebeveyn = -1;
    aramaSirasi[0].adim    = 0;
    ziyaretOlarakIsaretle(durumKodunuHesapla(aramaSirasi[0].dizi));
    sira_sonu = 1;

    //sira bos olana kadar devam eder:
    while (sira_basi < sira_sonu) {
        int simdikiIdx  = sira_basi;
        AramaDugumu  simdiki = aramaSirasi[sira_basi++];

        int hedefMi = 1;
        for (int i = 0; i < HUCRE_SAYISI; i++)
            if (simdiki.dizi[i] != hedefDurum[i]) { hedefMi = 0; break; }

        if (hedefMi) {
            int simdikiKonum = simdikiIdx;
            while (aramaSirasi[simdikiKonum].ebeveyn != -1 &&
                   aramaSirasi[aramaSirasi[simdikiKonum].ebeveyn].ebeveyn != -1)
                simdikiKonum = aramaSirasi[simdikiKonum].ebeveyn;

            //Baslangic zaten hedefe esitse:
            if (aramaSirasi[simdikiKonum].ebeveyn == -1) return '?'; 

            return aramaSirasi[simdikiKonum].adim;
        }

        //Komsulari uretir
        int mevcutTahta[BOYUT][BOYUT];
        birdenIkiye(simdiki.dizi, mevcutTahta);

        for (int y = 0; y < 4; y++) {

            // Mevcut tahtanin bir kopyasini alinir
            int denemeTahta[BOYUT][BOYUT];
            memcpy(denemeTahta, mevcutTahta, sizeof(mevcutTahta));

            int satir, sutun, yeni_satir, yeni_sutun;
            bosHucreyiBul(denemeTahta, &satir, &sutun);

            yeni_satir = satir;
            yeni_sutun = sutun;
            if      (yonSecenekleri[y] == 'W') yeni_satir--;
            else if (yonSecenekleri[y] == 'S') yeni_satir++;
            else if (yonSecenekleri[y] == 'A') yeni_sutun--;
            else                               yeni_sutun++;

            // Sinir kontrolu: gecersiz hamle ise bu yonu atla 
            if (yeni_satir < 0 || yeni_satir >= BOYUT ||
                yeni_sutun < 0 || yeni_sutun >= BOYUT) continue;

            //Hamleyi gerceklestirir
            denemeTahta[satir][sutun] = denemeTahta[yeni_satir][yeni_sutun];
            denemeTahta[yeni_satir][yeni_sutun] = 0;

            // Yeni durumun kodunu hesapla, daha once gorulduyse ekleme 
            int yeniDizi[HUCRE_SAYISI];
            ikiDenBire(denemeTahta, yeniDizi);
            long long durumNo = durumKodunuHesapla(yeniDizi);

            if (!ziyaretEdildiMi(durumNo) && sira_sonu < MAX_DURUM) {
                ziyaretOlarakIsaretle(durumNo);

                memcpy(aramaSirasi[sira_sonu].dizi, yeniDizi, sizeof(yeniDizi));
                aramaSirasi[sira_sonu].ebeveyn = simdikiIdx; 
                aramaSirasi[sira_sonu].adim    = yonSecenekleri[y];
                sira_sonu++;
            }
        }

    }

    return '?';

}

void karistir()
{
    char yonler[4] = {'W', 'S', 'A', 'D'};
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 200; i++)
        hamleYap(yonler[rand() % 4], 1); 
}

int skorHesapla() {
    int gecen_sure = (int)(time(NULL) - oyun_baslangici);
    // Her saniye 1 puan,her hamle 5 puan düşürür. Başlangıç 1000.
    int puan = 1000 - (gecen_sure * 1) - (hamle_sayisi * 5);
    return (puan < 0) ? 0 : puan;
}

int main()
{
    // Zorluk secimi
    int zorluk;
    printf("\n  Zorluk Secin (1: Kolay, 2: Orta, 3: Zor): ");
    scanf("%d", &zorluk);

    int karistirma_miktari = (zorluk == 1) ? 20 : (zorluk == 2) ? 100 : 250;

    // Tahtayi hedef durumda başlat
    for (int i = 0; i < BOYUT; i++)
        for (int j = 0; j < BOYUT; j++)
            tahta[i][j] = hedefDurum[i * BOYUT + j];

    // Secilen zorluga gore karistir
    srand((unsigned int)time(NULL));
    char yonler[4] = {'W', 'S', 'A', 'D'};
    for (int i = 0; i < karistirma_miktari; i++) {
        hamleYap(yonler[rand() % 4], 1); // 1: sessiz mod (hata mesaji basmaz)
    }

    // Degiskenleri sifirla
    hamle_sayisi = 0;
    oyun_baslangici = time(NULL);

    printf("\n  === 3x3 Sliding Puzzle ===\n");
    printf("  W/S/A/D: hareket | H: ipucu | Q: cikis\n");

    char komut;

    // Ana oyun dongusu
    while (1) {
        // 1. Ekranı ve Tahtayı Yazdır
        tahtayiYazdir();
    
        // 2. Skor ve Bilgileri Yazdır
        int mevcut_puan = skorHesapla();
        printf("\n  Hamle: %d  |  Sure: %d sn  |  SKOR: %d\n",
               hamle_sayisi, (int)(time(NULL) - oyun_baslangici), mevcut_puan);

        // 3. Kazanma Kontrolü
        if (kazandiMi()) {
            printf("\n  ************************************\n");
            printf("  TEBRIKLER! Final Skoru: %d\n", mevcut_puan);
            printf("  ************************************\n\n");
            break;
        }

        // 4. Komut Al
        printf("  Komut: ");
        scanf(" %c", &komut);

        // 5. Komutları İşle
        if (komut == 'Q' || komut == 'q') break;

        if (komut == 'H' || komut == 'h') {
            son_ipucu = bfsIpucu(); // İpucunu belleğe al
            continue; // Döngünün başına dön (tahtayı tekrar çizince ipucu görünecek)
        }

        // Eğer geçerli bir W/A/S/D hamlesi yapılırsa:
        if (hamleYap(komut, 0)) {
            hamle_sayisi++;
            son_ipucu = ' '; // Hareket edince eski ipucunu temizle
        }
    }

    return 0;
}
