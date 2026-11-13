# HYBRID OS X — Instrukcja budowania

**Wersja dokumentacji:** 1.0  
**Stan projektu:** bootstrap 0.1.0 / engineering preview  
**Architektura:** x86-64, UEFI  
**Język:** ISO C17 (freestanding)

## 1. Cel dokumentu

Ten dokument opisuje przygotowanie środowiska, pobranie kodu, kompilację obrazu EFI, testy host-side oraz uruchomienie HYBRID OS X w emulatorze QEMU z firmware OVMF.

Instrukcja dotyczy aktualnego stanu repozytorium. Nie zakłada istnienia jeszcze schedulera, pamięci wirtualnej, systemu plików, sieci ani przestrzeni użytkownika.

## 2. Wymagania

Zalecane środowisko: Ubuntu 24.04 lub nowsze, x86-64.

Wymagane pakiety:

- Git
- GCC i podstawowe narzędzia kompilacyjne
- GNU binutils
- GNU-EFI
- GNU Make
- QEMU dla x86
- OVMF

Instalacja:

```bash
sudo apt update
sudo apt install -y git build-essential gcc binutils make gnu-efi qemu-system-x86 ovmf
```

Sprawdź dostępność narzędzi:

```bash
git --version
gcc --version
make --version
qemu-system-x86_64 --version
```

Jeśli dystrybucja nie udostępnia pakietu `gnu-efi` lub OVMF pod tą nazwą, sprawdź nazwę pakietu w repozytoriach swojej wersji Ubuntu. Nie pobieraj przypadkowych binariów EFI z nieznanych źródeł.

## 3. Pobranie projektu

```bash
git clone https://github.com/erykjuraszczyk-ui/Hybrid-OS-X.git
cd Hybrid-OS-X
```

Sprawdź gałąź i stan repozytorium:

```bash
git status
git branch --show-current
```

Przed rozpoczęciem pracy pobierz najnowsze zmiany:

```bash
git pull --ff-only
```

## 4. Budowanie systemu

W katalogu głównym repozytorium wykonaj:

```bash
make clean
make all
```

Oczekiwany artefakt EFI:

```text
build/esp/EFI/BOOT/BOOTX64.EFI
```

Jest to aplikacja UEFI w formacie PE32+. W aktualnym etapie kod startowy i początkowy kod jądra są połączone w jednym obrazie EFI. Nie jest to jeszcze osobny loader ELF ani niezależny plik kernela.

## 5. Testy i kontrola artefaktu

Uruchom testy:

```bash
make test
make check
```

- `make test` uruchamia testy host-side parsera mapy pamięci UEFI i kontroli granic.
- `make check` sprawdza wygenerowaną aplikację EFI oraz jej format PE/COFF.

Jeśli polecenie zakończy się błędem, zachowaj pełny log terminala. Nie oznaczaj kompilacji jako poprawnej tylko dlatego, że plik EFI istnieje.

## 6. Uruchomienie w QEMU

```bash
make run
```

QEMU uruchamia maszynę q35 z firmware OVMF. Diagnostyka jądra jest kierowana przez port szeregowy COM1 do terminala.

Jeśli Makefile nie odnajduje firmware OVMF, sprawdź lokalizację plików:

```bash
find /usr/share -iname 'OVMF_CODE*.fd' -o -iname 'OVMF_VARS*.fd'
```

Następnie przekaż odpowiednie ścieżki:

```bash
make run OVMF_CODE=/pełna/ścieżka/OVMF_CODE.fd OVMF_VARS=/pełna/ścieżka/OVMF_VARS.fd
```

Nazwy i lokalizacje plików OVMF zależą od dystrybucji. Użyj pary plików zgodnej z konfiguracją pakietu.

## 7. Dostępne cele Makefile

| Polecenie | Przeznaczenie |
|---|---|
| `make all` | Buduje BOOTX64.EFI i przygotowuje strukturę EFI |
| `make test` | Uruchamia testy host-side |
| `make check` | Weryfikuje artefakt EFI |
| `make run` | Uruchamia QEMU/OVMF |
| `make clean` | Usuwa wygenerowane pliki kompilacji |

## 8. Diagnostyka problemów

### Brak kompilatora lub Make

```bash
sudo apt install -y build-essential gcc binutils make
```

### Brak nagłówków GNU-EFI

```bash
sudo apt install -y gnu-efi
```

Po instalacji ponów `make clean all`. Jeśli błąd pozostaje, sprawdź ścieżki nagłówków i bibliotek GNU-EFI używane przez Makefile.

### QEMU nie uruchamia obrazu

Sprawdź:

1. Czy istnieje `build/esp/EFI/BOOT/BOOTX64.EFI`.
2. Czy używany jest firmware OVMF dla x86-64.
3. Czy parametry `OVMF_CODE` i `OVMF_VARS` wskazują istniejące pliki.
4. Czy `make check` przechodzi poprawnie.

### Brak komunikatów na ekranie

W pierwszym etapie COM1 jest podstawowym kanałem diagnostycznym. Sprawdź wyjście terminala QEMU. Nie zakładaj, że brak tekstu w oknie graficznym oznacza brak uruchomienia kodu.

## 9. Czego obecny build jeszcze nie zapewnia

Wersja 0.1.0 nie implementuje:

- pełnego zarządzania pamięcią i własnych tablic stron,
- IDT/GDT i kompletnej obsługi wyjątków,
- planisty procesów i wątków,
- IPC,
- systemu plików i VFS,
- sterowników USB, sieci i pełnej obsługi PCI,
- kont użytkowników i powłoki,
- kompatybilności aplikacji Windows/Linux,
- kompletnego Secure Boot, PKI ani szyfrowania dysku.

Te funkcje wymagają osobnych etapów, specyfikacji, kodu i testów.

## 10. Czyste środowisko kompilacji

Po zakończeniu pracy:

```bash
make clean
git status --short
```

Nie commituj katalogu `build/`, plików firmware, kluczy prywatnych, sekretów ani lokalnych plików konfiguracyjnych.

## 11. Kryterium ukończenia

Etap budowania 0.1.0 można uznać za poprawnie wykonany, gdy:

- [ ] zależności są zainstalowane,
- [ ] repozytorium zostało pobrane,
- [ ] `make clean all` kończy się powodzeniem,
- [ ] plik BOOTX64.EFI istnieje,
- [ ] `make test` przechodzi,
- [ ] `make check` przechodzi,
- [ ] `make run` uruchamia obraz w QEMU/OVMF,
- [ ] logi diagnostyczne zostały sprawdzone.

---

Powiązane dokumenty: [ARCHITECTURE.md](ARCHITECTURE.md), [ROADMAP.md](ROADMAP.md), [DEVELOPMENT_GUIDE_PL.md](DEVELOPMENT_GUIDE_PL.md), [TESTING_GUIDE_PL.md](TESTING_GUIDE_PL.md).
