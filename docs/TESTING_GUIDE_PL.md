# HYBRID OS X — Testowanie i diagnostyka

## 1. Cel

Testowanie HYBRID OS X ma wykrywać błędy w kodzie niskopoziomowym, kontrakcie startowym i obsłudze pamięci zanim zmiany trafią do kolejnych etapów jądra.

Status testów należy opisywać zgodnie z tym, co rzeczywiście uruchomiono. Samo udane linkowanie nie jest dowodem poprawnego startu systemu.

## 2. Poziomy testowania

### Testy host-side

Działają jako zwykłe programy na komputerze deweloperskim. Aktualny projekt wykorzystuje je do kontroli parsera mapy pamięci UEFI i granic deskryptorów.

Uruchom:

```bash
make test
```

### Kontrola artefaktu

```bash
make check
```

Sprawdza właściwości wygenerowanej aplikacji EFI i format PE/COFF.

### Test uruchomieniowy w emulatorze

```bash
make run
```

QEMU + OVMF umożliwia powtarzalne sprawdzanie ścieżki startowej bez zapisywania obrazu na fizycznym dysku.

## 3. Zalecana sekwencja walidacji

Po każdej zmianie kodu:

```bash
make clean
make all
make test
make check
make run
```

Dodatkowo:

```bash
git diff --check
git status --short
```

Jeżeli którykolwiek etap nie przechodzi, nie oznaczaj zmiany jako gotowej.

## 4. Testy mapy pamięci

Mapa pamięci UEFI jest danymi zewnętrznymi i może mieć deskryptory o rozmiarze większym niż rozmiar znanej wersji struktury.

Weryfikuj co najmniej:

- pustą mapę,
- pojedynczy deskryptor,
- wiele deskryptorów,
- nieoczekiwany rozmiar deskryptora,
- niepoprawny rozmiar bufora,
- obcięty ostatni deskryptor,
- niepoprawny lub zerowy stride,
- przepełnienie adresu końcowego,
- niepoprawną wersję lub rozmiar danych handoff.

Nie odczytuj kolejnych deskryptorów przez założenie, że struktury leżą bezpośrednio jedna za drugą według rozmiaru lokalnej struktury C. Używaj stride przekazanego przez firmware i sprawdzaj granice bufora przed każdym odczytem.

## 5. Testowanie ExitBootServices

Sprawdź scenariusze:

1. Prawidłowe pobranie mapy pamięci i udane przekazanie sterowania.
2. Zmiana mapy pamięci przed wywołaniem ExitBootServices.
3. Ograniczona liczba ponowień.
4. Błąd firmware i bezpieczne zakończenie ścieżki startowej.
5. Brak wywołań Boot Services po udanym ExitBootServices.

Po zakończeniu usług firmware jądro nie może korzystać z ich funkcji ani zakładać, że zasoby firmware pozostają dostępne.

## 6. Diagnostyka QEMU

Podstawowym kanałem diagnostycznym aktualnej wersji jest COM1. Analizuj komunikaty terminala QEMU i dodawaj stabilne znaczniki etapów startu.

Przydatne znaczniki diagnostyczne powinny odpowiadać rzeczywistym zdarzeniom, np.:

- wejście aplikacji UEFI,
- wykrycie GOP,
- przygotowanie mapy pamięci,
- udane ExitBootServices,
- wejście do kernel_main,
- zakończenie inicjalizacji bieżącego etapu.

Nie dodawaj komunikatu o sukcesie przed faktycznym zakończeniem operacji.

## 7. Testy awarii i odporności

Dla każdego nowego modułu zaplanuj:

- błędne dane wejściowe,
- wartości graniczne,
- brak zasobu,
- błąd urządzenia,
- przerwanie inicjalizacji,
- powtórne wywołanie,
- zachowanie po błędzie.

Obsługa awarii powinna być deterministyczna. W ścieżce panic nie polegaj na alokacji sterty ani na usługach firmware.

## 8. CI i przegląd zmian

Każda zmiana powinna przejść automatyczną kompilację i testy dostępne w repozytorium. Jeśli workflow CI nie obejmuje konkretnego testu, uruchom go lokalnie i opisz wynik w PR.

Checklist PR:

- [ ] Zmiana ma jasno określony zakres.
- [ ] Kod jest zgodny z C17 i zasadami freestanding.
- [ ] Sprawdzone są granice buforów i przepełnienia.
- [ ] Dodano lub zaktualizowano testy.
- [ ] `make all` przechodzi.
- [ ] `make test` przechodzi.
- [ ] `make check` przechodzi.
- [ ] QEMU/OVMF został uruchomiony, jeśli zmiana dotyczy ścieżki startowej.
- [ ] Dokumentacja została zaktualizowana.
- [ ] W commicie nie ma build artifacts, sekretów ani firmware.

## 9. Raportowanie błędów

Zgłoszenie powinno zawierać:

- wersję projektu i identyfikator commita,
- system hosta i wersję narzędzi,
- dokładne polecenie,
- pełny komunikat błędu,
- oczekiwany rezultat,
- rzeczywisty rezultat,
- informację, czy błąd występuje również na czystym buildzie.

Nie umieszczaj w zgłoszeniu kluczy prywatnych, tokenów ani danych osobowych.

## 10. Kryterium ukończenia

Zmiana jest gotowa do integracji, gdy odpowiednie testy przechodzą, wynik jest odtwarzalny, a dokumentacja nie sugeruje funkcji, których kod jeszcze nie realizuje.
