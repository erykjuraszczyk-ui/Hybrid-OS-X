# HYBRID OS X — Dokumentacja (PL)

Witamy w polskiej dokumentacji projektu HYBRID OS X.

> **Status projektu: 0.1.0 bootstrap / engineering preview.** Aktualnie projekt obejmuje eksperymentalną ścieżkę startową UEFI x86-64, freestanding C17, diagnostykę COM1, podstawowy ekran GOP i odczyt mapy pamięci UEFI. Nie jest jeszcze kompletnym systemem operacyjnym.

## Dokumenty

| Dokument | Zawartość |
|---|---|
| [Instrukcja budowania](BUILD_GUIDE_PL.md) | Instalacja narzędzi, kompilacja EFI, testy i uruchomienie QEMU |
| [Instrukcja rozwoju](DEVELOPMENT_GUIDE_PL.md) | Architektura modułów, kontrakt UEFI, reguły C17 i roadmap implementacji |
| [Testowanie i diagnostyka](TESTING_GUIDE_PL.md) | Testy host-side, QEMU, mapa pamięci i kontrola jakości |
| [Architektura](ARCHITECTURE.md) | Aktualny kontrakt architektoniczny projektu |
| [Roadmap](ROADMAP.md) | Etapy rozwoju i status implementacji |

## Szybki start

```bash
sudo apt update
sudo apt install -y git build-essential gcc binutils make gnu-efi qemu-system-x86 ovmf

git clone https://github.com/erykjuraszczyk-ui/Hybrid-OS-X.git
cd Hybrid-OS-X

make clean all
make test
make check
make run
```

Oczekiwany obraz EFI:

```text
build/esp/EFI/BOOT/BOOTX64.EFI
```

## Zasady projektu

1. Kod specyficzny dla architektury pozostaje w osobnym katalogu.
2. UEFI Boot Services są dostępne wyłącznie w fazie startowej.
3. Dane handoff i mapy pamięci muszą być walidowane.
4. Funkcje uznaje się za zaimplementowane dopiero po dodaniu kodu i testów.
5. PKI i zarządzanie kluczami pozostają poza jądrem.
6. Nie commitujemy wygenerowanych binariów, firmware, kluczy ani sekretów.

## Język dokumentacji

Polskie instrukcje mają sufiks `_PL`. Dokumenty bazowe projektu pozostają w języku angielskim, aby zachować spójność z istniejącymi plikami i workflow.
