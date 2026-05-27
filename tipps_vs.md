# Einrichtung & Debugging von **TiGL** in Visual Studio

## Vorbereitungen

### Developer Command Prompt über das Kontextmenü starten

Um den **VS Developer Command Prompt** direkt per Rechtsklick in einem beliebigen Ordner zu öffnen:

1. **Registry öffnen:**
   `regedit`

2. Folgenden Schlüssel einrichten:

   ```
   Computer\HKEY_CLASSES_ROOT\Directory\Background\shell\VSDevConsole\command
   ```

3. **Standardwert setzen auf:**

   ```
   cmd.exe /k "cd /d %V && call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64"
   ```

Danach lässt sich per **Rechtsklick → VSDevConsole** im Explorer
ein Developer Prompt direkt im aktuellen Verzeichnis öffnen.

## Pixi installieren

Pixi dient dem Erstellen und Verwalten der TiGL-Build-Umgebung (siehe [TiGL Docs](https://github.com/DLR-SC/tigl/wiki/Building-TiGL-from-Source)).

```bash
winget install --id=prefix-dev.pixi -e
```

## Python-/Build-Umgebung für TiGL konfigurieren

Einmalig im Projektverzeichnis:

```bash
pixi run -e default configure
```

Dadurch wird die Build-Umgebung eingerichtet, die CMake-Konfiguration initialisiert
und die Abhängigkeiten (z. B. OpenCASCADE, tixi3, etc.) geladen.

## Entwickeln & Debuggen mit Visual Studio

### Visual Studio starten

1. **Developer Command Prompt öffnen** (siehe oben für Option vir Rechtsklick im Projektverzeichnis)
2. Im Projektverzeichnis ausführen:

   ```bash
   run_vs.bat
   ```

   ➜ Das Skript führt CMake aus und startet Visual Studio automatisch mit der generierten Solution.

### Visual Studio konfigurieren

* Im **Projektmappen-Explorer** wird die Datei `TIGL.sln` angezeigt.
  Darin befinden sich alle Build-Ziele (Projekte).

* Rechtsklick auf
  **`TiGL-unittests` → Als Startprojekt festlegen**

* Zum Einrichten von Testfiltern: In den **Eigenschaften** des Projekts →
  **Debugging → Befehlsargumente:**

  ```
  --gtest_filter=Systems.*
  ```


### Debugging-Hinweise

* Der eigentliche Quellcode befindet sich in:

  ```
  build-vs/src/tigl3_objects
  ```

  → Hier können **Haltepunkte** gesetzt werden.
  Auch wenn diese Dateien Teil einer statischen Library sind,
  werden sie beim Start von `TiGL-unittests.exe` korrekt geladen.
