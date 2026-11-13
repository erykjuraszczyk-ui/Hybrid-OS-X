# HYBRID OS X — Instrukcja rozwoju systemu

## 1. Zasady architektoniczne

HYBRID OS X rozpoczyna rozwój jako **modularne jądro monolityczne**. Podsystemy są rozdzielone na moduły, nagłówki i jawne interfejsy, ale w tej fazie działają w jednym uprzywilejowanym kontekście.

Docelowa wieloplatformowość nie oznacza mieszania kodu architektur. Kod specyficzny dla procesora musi znajdować się w `kernel/arch/<architektura>/`. Wspólne moduły nie mogą zależeć bezpośrednio od portów I/O ani rejestrów x86.

## 2. Struktura projektu

Aktualny układ opisany przez README:

```text
boot/uefi/             wejście UEFI i przekazanie sterowania
include/hybrid/        wersjonowany kontrakt danych startowych
kernel/arch/x86_64/    prymitywy CPU x86-64
kernel/console/        podstawowa obsługa GOP
kernel/core/           wejście jądra i panic
kernel/drivers/serial/ diagnostyka COM1
kernel/mm/             odczyt mapy pamięci UEFI
docs/                  architektura, roadmap i specyfikacje
scripts/               skrypty pomocnicze
```

Nie twórz katalogów lub modułów tylko po to, aby wyglądały na gotowe. Nowy komponent powinien mieć określoną odpowiedzialność, interfejs, właściciela zasobów i test.

## 3. Kontrakt startowy UEFI

`boot/uefi/entry.c` jest jedynym komponentem, który może wywoływać UEFI Boot Services.

Faza startowa:

1. Firmware uruchamia aplikację EFI.
2. Kod startowy wykrywa dostępne GOP i potrzebne informacje firmware.
3. Pobiera ograniczony snapshot mapy pamięci.
4. Przygotowuje wersjonowaną strukturę `hybrid_boot_info_t`.
5. Wywołuje `ExitBootServices`, z ograniczoną obsługą ponowienia, jeśli mapa pamięci uległa zmianie.
6. Przekazuje sterowanie do `kernel_main`.

Po udanym `ExitBootServices` nie wolno wywoływać usług Boot Services. Każdy moduł jądra musi korzystać z przekazanego kontraktu, a nie z przypadkowych wskaźników do struktur firmware.

Przed użyciem danych startowych jądro musi sprawdzić wersję, rozmiar struktury, wskaźniki, długości i zakresy pamięci.

## 4. Reguły implementacji w C17

- Używaj standardu C17 i jawnego trybu freestanding.
- Ogranicz rozszerzenia kompilatora do uzasadnionych przypadków.
- Nie zakładaj, że struktury firmware są spakowane bez przerw.
- Iteruj po mapie pamięci z użyciem przekazanego rozmiaru deskryptora.
- Sprawdzaj przepełnienia przy obliczaniu adresów i rozmiarów.
- Nie zakładaj dostępności sterty, wyjątków językowych ani biblioteki standardowej.
- Ścieżka panic nie może wymagać alokacji pamięci.
- Każdy publiczny interfejs modułu powinien mieć opis kontraktu i warunków błędu.

Przykład bezpiecznej kontroli dodawania rozmiaru do adresu:

```c
#include <stdint.h>
#include <stdbool.h>

static bool range_end_u64(uint64_t start, uint64_t size, uint64_t *end)
{
    if (end == 0 || size > UINT64_MAX - start) {
        return false;
    }

    *end = start + size;
    return true;
}
```

To przykład pomocniczy, nie gotowa implementacja alokatora ani mapowania pamięci.

## 5. Granice modułów

| Moduł | Odpowiedzialność |
|---|---|
| boot/uefi | Firmware discovery, mapa pamięci, handoff |
| arch/x86_64 | Instrukcje i rejestry specyficzne dla x86-64 |
| console | Abstrakcja wyjścia tekstowego/graficznego |
| serial | Diagnostyka portu szeregowego |
| core | Główna inicjalizacja, panic |
| mm | Zarządzanie pamięcią — rozwijane etapami |

Nowy sterownik powinien otrzymać ograniczony interfejs. Nie udostępniaj całemu jądru surowych rejestrów urządzenia, jeśli wystarczy abstrakcja sterownika.

## 6. Kolejność rozwoju

### M1 — Stabilne wejście CPU (0.2.x)

- dedykowany punkt wejścia asemblerowy,
- własny stos jądra,
- GDT i IDT,
- procedury wyjątków,
- jawny układ pamięci i linker script,
- testy startu w QEMU.

### M2 — Pamięć i wykonanie (0.3.x)

- alokator ramek fizycznych,
- własne tablice stron,
- API pamięci wirtualnej,
- alokator sterty z kontrolą błędów,
- abstrakcja timera i kontrolera przerwań,
- wykrywanie SMP i uruchamianie CPU pomocniczych.

### M3 — Usługi jądra (0.4.x)

- wątki i przełączanie kontekstu,
- planista,
- synchronizacja i zasady blokad,
- IPC i uchwyty oparte na możliwościach,
- cykl życia obiektów jądra i sterowników.

### M4 — Pamięć masowa i userspace (0.5.x)

- enumeracja PCI,
- warstwa urządzeń blokowych,
- parser GPT,
- sterownik FAT w trybie tylko do odczytu,
- VFS i pierwszy system plików z zapisem,
- loader ELF64, ABI syscall i proces init.

### M5 — Platforma bezpieczeństwa (0.6.x i później)

- projekt Secure Boot i measured boot,
- separacja uprawnień i capabilities,
- dziennik audytowy,
- usługa PKI poza jądrem,
- prywatność, retencja i usuwanie danych,
- SDK, podpisywanie aktualizacji i zarządzanie wydaniami,
- telemetria operacyjna i procedury incydentowe.

## 7. Bezpieczeństwo i zaufanie

PKI i zarządzanie kluczami kryptograficznymi nie powinny być implementowane wewnątrz jądra. Jądro udostępnia minimalne mechanizmy ochronne; usługi tożsamości i zaufania działają za osobnymi, kontrolowanymi granicami.

Klucze prywatne, sekrety i dane użytkowników nie mogą trafiać do repozytorium. Operacje kryptograficzne wymagają przeglądu, testów i dokumentacji cyklu życia kluczy.

## 8. Workflow programisty

```bash
git pull --ff-only
git switch -c feature/nazwa-zmiany
make clean all
make test
make check
make run
git diff --check
git status --short
```

Po zmianach:

1. Opisz cel i zakres zmiany.
2. Zaktualizuj dokumentację interfejsu lub architektury.
3. Dodaj testy dla błędów i wartości brzegowych.
4. Sprawdź kompilację i testy.
5. Przejrzyj diff pod kątem przypadkowych plików.
6. Utwórz commit z krótkim, rzeczowym opisem.

Przykład:

```bash
git add kernel include docs
git commit -m "kernel: add bounded subsystem interface"
```

Nie dodawaj do commita katalogu `build/`, firmware, kluczy, plików sekretów ani niezweryfikowanych binariów.

## 9. Kryteria akceptacji modułu

Moduł można oznaczyć jako zaimplementowany dopiero, gdy:

- istnieje kod źródłowy,
- interfejs i odpowiedzialność są opisane,
- błędy i zakresy wejścia są kontrolowane,
- testy pokrywają ścieżkę poprawną i krytyczne przypadki brzegowe,
- kompilacja i testy całego projektu przechodzą,
- dokumentacja odpowiada rzeczywistemu zachowaniu.

Powiązane dokumenty: [ARCHITECTURE.md](ARCHITECTURE.md), [ROADMAP.md](ROADMAP.md), [BUILD_GUIDE_PL.md](BUILD_GUIDE_PL.md), [TESTING_GUIDE_PL.md](TESTING_GUIDE_PL.md).
