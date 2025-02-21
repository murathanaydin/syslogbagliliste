#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESAJ_UZUNLUK 1024  // Maksimum log mesaj uzunlu�u

// Syslog i�in ba�l� liste d���m�
typedef struct LogDugum {
    char zamanDamgasi[20];   // Zaman bilgisi (YYYY-MM-DD HH:MM:SS)
    char seviye[10];         // Log seviyesi (INFO, ERROR, WARNING)
    char mesaj[MAX_MESAJ_UZUNLUK];  // Log mesaj�
    struct LogDugum *sonraki; // Ba�l� listedeki sonraki d���m
} LogDugum;

// Ba�l� listeye yeni bir log ekler
void logEkle(LogDugum **bas, const char *zaman, const char *seviye, const char *mesaj) {
    LogDugum *yeniDugum = (LogDugum *)malloc(sizeof(LogDugum));
    if (!yeniDugum) {
        perror("Bellek tahsis hatas�");
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

// Syslog dosyas�n� okuyarak ba�l� listeye ekler
void syslogOku(LogDugum **bas, const char *dosyaAdi) {
    FILE *dosya = fopen(dosyaAdi, "r");
    if (!dosya) {
        perror("Syslog dosyas� a��lamad�");
        exit(EXIT_FAILURE);
    }

    char satir[MAX_MESAJ_UZUNLUK];
    while (fgets(satir, sizeof(satir), dosya)) {
        char zaman[20] = "Tarih_Bilinmiyor";
        char seviye[10] = "Bilinmiyor";
        char mesaj[MAX_MESAJ_UZUNLUK];

        // Syslog format�: "Feb 20 12:34:56 hostname process[1234]: ERROR: Disk full"
        sscanf(satir, "%19s %9s %1023[^\n]", zaman, seviye, mesaj);
        logEkle(bas, zaman, seviye, mesaj);
    }
    fclose(dosya);
}

// Ba�l� listedeki loglar� ekrana yazd�r�r
void loglariYazdir(LogDugum *bas) {
    while (bas) {
        printf("[%s] (%s) %s\n", bas->zamanDamgasi, bas->seviye, bas->mesaj);
        bas = bas->sonraki;
    }
}

// Ba�l� listeyi temizleyerek belle�i serbest b�rak�r
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

    // Syslog dosyas�n� oku ve ba�l� listeye ekle
    syslogOku(&logListesi, "/var/log/syslog");

    // Kay�tlar� ekrana yazd�r
    loglariYazdir(logListesi);

    // Belle�i temizle
    loglariSil(logListesi);

    return 0;
}

