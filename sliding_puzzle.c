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
    printf("\n  ---------\n");
    for (int i = 0; i < BOYUT; i++) {
        printf("  |");
        for (int j = 0; j < BOYUT; j++) {
            if (tahta[i][j] == 0)
                printf(" _ ");  // bos hucreyi alt cizgi ile gosterilir
            else
                printf(" %d ", tahta[i][j]);
        }
        printf("|\n");
    }
    printf("  ---------\n");
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


//Cozulebilirlik Kontolu::
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
                aramaSirasi[sira_sonu].ebeveyn = simdikiIdx; /* kimin cocugu oldugunu kaydet */
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

int main()
{
    //Tahtayi hedef durumda baslatir ve sonra karistirir
    for (int i = 0; i < BOYUT; i++)
        for (int j = 0; j < BOYUT; j++)
            tahta[i][j] = hedefDurum[i * BOYUT + j];
    karistir();


    int dizi1B[HUCRE_SAYISI];
    ikiDenBire(tahta, dizi1B);
    if (!cozulebilirMi(dizi1B)) {
        printf("  [HATA] Tahta cozulemez! Program durduruluyor.\n");
        return 1;
    }

    hamle_sayisi    = 0;
    oyun_baslangici = time(NULL);

    printf("  === 3x3 Sliding Puzzle ===\n");
    printf("  Hedef: 1 2 3 | 4 5 6 | 7 8 _\n");
    printf("  W/S/A/D: hareket H:ipucu  Q: cikis\n");

    char komut;

    while (1) {
        tahtayiYazdir();
        printf("  Hamle: %d  |  Sure: %d sn\n",
               hamle_sayisi, (int)(time(NULL) - oyun_baslangici));

        if (kazandiMi()) {
            printf("\n  Tebrikler! %d hamlede cozdunuz!\n\n", hamle_sayisi);
            break;
        }

        printf("  Komut: ");
        scanf(" %c", &komut);

        if (komut == 'H' || komut == 'h') {
            char ipucu = bfsIpucu();
            printf("  Ipucu: %c\n", ipucu);
            continue;
        }           

        if (komut == 'Q' || komut == 'q') {
            printf("  Cikis yapiliyor.\n");
            break;
        }

        if (hamleYap(komut, 0))
            hamle_sayisi++;
    }

    return 0;
}
