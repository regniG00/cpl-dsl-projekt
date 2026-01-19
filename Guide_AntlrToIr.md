# 🛠 DSL → Parser → IR → JSON (ANTLR4 + CMake)

Dieser Guide dokumentiert die aktuelle Pipeline unseres Projekts:

> **DSL-Text (.txt) → ANTLR Parser → Parse Tree → IR (Visitor) → JSON-Ausgabe**

Die Implementierung ist plattformunabhängig (Windows / Linux / macOS) und nutzt:

* ANTLR 4.13.2 (C++ Target)
* CMake + C++17
* `antlr4-runtime` (z. B. via vcpkg)

Alle Schritte und Entscheidungen basieren auf der gemeinsamen Arbeit aus der Session.

---

## 📁 Projektstruktur

```
CPL/DSL_Projekt/Aufgabenerstellungsgrammatik/
│
├── Aufgabenerstellungsgrammatik/
│   ├── grammar/
│   │   ├── Aufgabenerstellungsgrammatik.g4   ← DSL Grammatik
│   │   ├── [ANTLR generierte C++ Dateien]     (ignored)
│   │
│   ├── src/
│   │   ├── IR.h                              ← IR-Struktur + JSON Export (pretty)
│   │   ├── IRBuilder.h / .cpp                ← ParseTree → IR Visitor
│   │   ├── main.cpp                          ← TXT → JSON CLI Tool
│   │
│   ├── usage/
│   │   ├── input/example0.txt
│   │   └── output/example0.json
│   │
│   ├── CMakeLists.txt
│   └── build/                                ← CMake Build Ordner (ignored)
```

---

## 🧾 1️⃣ ANTLR Parser generieren (C++ Target)

```powershell
cd grammar

java "-Dfile.encoding=UTF-8" -jar "C:/Users/Malte/tools/antlr/antlr-4.13.2-complete.jar" -Dlanguage=Cpp -visitor "Aufgabenerstellungsgrammatik.g4"


cd ..

```

Erzeugt u. a.:

```
Lexer / Parser (.cpp/.h)
BaseVisitor / Visitor
.tokens / .interp
```

Die Dateien werden **nicht versioniert**.

---

## 🛠 2️⃣ Build mit CMake

```powershell
Remove-Item -Recurse -Force .\build
New-Item -ItemType Directory -Force -Path .\build | Out-Null
cd build
cmake ..
cmake --build . --config Release
cd ..
.\build\Release\aufgaben_dsl.exe usage\input\example0.txt usage\output\example0.json

```

Das erzeugt u. a.:

```
Release/aufgaben_dsl.exe
```

---

## 🚀 3️⃣ CLI Werkzeug: TXT → JSON

Wir haben den Programmfluss bewusst vereinfacht:

✔ kein Interaktiv‑Modus
✔ kein stdin
✔ **nur Dateimodus**

```
aufgaben_dsl.exe <eingabe.txt> <ausgabe.json>
```

Beispiel (aus Projektwurzel):

```powershell
.\build\Release\aufgaben_dsl.exe usage\input\example0.txt usage\output\example0.json
```

Fehler werden auf **stderr** ausgegeben:

* fehlende Datei
* leere Eingabe
* Syntaxfehler beim Parsen

Die JSON‑Datei wird dabei **nicht überschrieben**.

---

## 🧩 4️⃣ Parse Tree → IR (IRBuilder)

Der Visitor behält Whitespaces über eine Hilfsfunktion

```cpp
textWithSpaces(ctx)
```

indem der Original‑Substring aus dem Eingabetext rekonstruiert wird.

Abgebildete Task‑Typen:

* TRUE/FALSE („RoF“)
* Sorting („Umordnung“)
* Matching („Zuordnung“)
* Marking („Markierung“)

Output wird in `ProgramIR` gesammelt und dann serialisiert.

---

## ⚙ Abhängigkeiten

| Komponente     | Version   | Installation                       |
| -------------- | --------- | ---------------------------------- |
| ANTLR          | 4.13.2    | manuell `.jar`                     |
| CMake          | ≥ 3.15    | `winget install cmake`             |
| Compiler       | C++17     | MSVC / clang / gcc                 |
| antlr4-runtime | via vcpkg | `vcpkg install antlr4:x64-windows` |

---

## 🔁 Entwicklungsworkflow

1️⃣ Grammar in `.g4` ändern

2️⃣ Parser neu generieren

```powershell
java -jar antlr-4.13.2-complete.jar -Dlanguage=Cpp -visitor …
```

3️⃣ CMake Rebuild

```powershell
cmake --build . --config Release
```

4️⃣ DSL testen

```powershell
aufgaben_dsl.exe input.txt output.json
```

5️⃣ JSON‑IR prüfen

---
