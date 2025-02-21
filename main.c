#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESAJ_UZUNLUK 1024  // Maksimum log mesaj uzunluðu

// Syslog için baðlý liste düðümü
typedef struct LogDugum {
    char zamanDamgasi[20];   // Zaman bilgisi (YYYY-MM-DD HH:MM:SS)
    char seviye[10];         // Log seviyesi (INFO, ERROR, WARNING)
    char mesaj[MAX_MESAJ_UZUNLUK];  // Log mesajý
    struct LogDugum *sonraki; // Baðlý listedeki sonraki düðüm
} LogDugum;

// Baðlý listeye yeni bir log ekler
void logEkle(LogDugum **bas, const char *zaman, const char *seviye, const char *mesaj) {
    LogDugum *yeniDugum = (LogDugum *)malloc(sizeof(LogDugum));
    if (!yeniDugum) {
        perror("Bellek tahsis hatasý");
        exit(EXIT_FAILURE);
    }

    strcpy(yeniDugum->zamanDamgasi, zaman);
    strcpy(yeniDugum->seviye, seviye);
    strcpy(yeniDugum->mesaj, mesaj);
    yeniDugum->sonraki = NULL;

    if (*bas == NULL) {
        *bas = yeniDugum;
    } else {
        LogDugum *gecici = *bas;
        while (gecici->sonraki) {
            gecici = gecici->sonraki;
        }
        gecici->sonraki = yeniDugum;
    }
}

// Syslog dosyasýný okuyarak baðlý listeye ekler
void syslogOku(LogDugum **bas, const char *dosyaAdi) {
    FILE *dosya = fopen(dosyaAdi, "r");
    if (!dosya) {
        perror("Syslog dosyasý açýlamadý");
        exit(EXIT_FAILURE);
    }

    char satir[MAX_MESAJ_UZUNLUK];
    while (fgets(satir, sizeof(satir), dosya)) {
        char zaman[20] = "Tarih_Bilinmiyor";
        char seviye[10] = "Bilinmiyor";
        char mesaj[MAX_MESAJ_UZUNLUK];

        // Syslog formatý: "Feb 20 12:34:56 hostname process[1234]: ERROR: Disk full"
        sscanf(satir, "%19s %9s %1023[^\n]", zaman, seviye, mesaj);
        logEkle(bas, zaman, seviye, mesaj);
    }
    fclose(dosya);
}

// Baðlý listedeki loglarý ekrana yazdýrýr
void loglariYazdir(LogDugum *bas) {
    while (bas) {
        printf("[%s] (%s) %s\n", bas->zamanDamgasi, bas->seviye, bas->mesaj);
        bas = bas->sonraki;
    }
}

// Baðlý listeyi temizleyerek belleði serbest býrakýr
void loglariSil(LogDugum *bas) {
    LogDugum *gecici;
    while (bas) {
        gecici = bas;
        bas = bas->sonraki;
        free(gecici);
    }
}

// Ana fonksiyon
int main() {
    LogDugum *logListesi = NULL;

    // Syslog dosyasýný oku ve baðlý listeye ekle
    syslogOku(&logListesi, "/var/log/syslog");

    // Kayýtlarý ekrana yazdýr
    loglariYazdir(logListesi);

    // Belleði temizle
    loglariSil(logListesi);

    return 0;
}

