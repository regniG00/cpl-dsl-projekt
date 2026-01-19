# DSL Projekt:
-   C++
-   Projekt nicht im Zusammenhang mit Dungeon
-   Projektthema DSL und IR: [Inhalt](https://github.com/Compiler-CampusMinden/CPL-Vorlesung-Master-W25/discussions/11)

## Meeting 1 (25.11.2025):
- Interessanter Use Case, was passiert bei Duplikationen?

Input Sprache: Deutsch

Wichtig -> Punktevergabe (Wie viele Punkte bringt richtige und falsche Antwort, Gesamtpunktanzahl -> Notenschlüssel bilden (Angabe des Breakpoints e.g. 50% = 4.0)

Aufgabe 1(Einfachauswahl): Wie heißt die Hauptstadt von Frankreich? -Paris(1) -Berlin(0) -Hamburg(0) -Minden(0)

### Aufgabe/Idee 1 - Deklaration des Aufgabentyps vielleicht redundant, durch Aufgabenpunkte ersetzbar? :

```
Aufgabe 1(Aufgabenpunkte): Wie heißt die Hauptstadt von Frankreich? -Paris(Teilantwortpunke) -Berlin(Teilantwortpunke) -Hamburg(Teilantwortpunke) -Minden(Teilantwortpunke);
Aufgabe 1(2): Wie heißt die Hauptstadt von Frankreich? -Paris(2) -Berlin(0) -Hamburg(0) -Minden(0) 2/-1;
```

(Könnte im Dungeon trotzdem machbar sein, aber nicht unser Scope)
Teilpunktzahl erreicht -> Tür öffnet
Keine Punkte -> Tür öffnet sich gar nicht

### Aufgabe/Idee 2,3 - Aufteilung von Richtig oder Falsch, Erklärtext bei falscher Antwort? Aber vielleicht doch redundant zur Single Choice:

```
Aufgabe 2(RoF): Die Vatikanstadt ist ein Land! -Richtig;
Aufgabe 3(RoF): Melbourne ist die Hauptstadt Australiens! -Falsch ->Die Hauptstadt ist Canberra.;
```

### Aufgabe 4 - Gibt keine Notwendigkeit für offenen Fragetypen da, Zuordnungsaufgabe eindeutig, daher nur Angabe der zu ordnenden Begriffe?
- Ohne Darstellungsreihenfolge
- (Hauptstädte), (Land) Platzhalter für andere Zuordnungsthemen? Tier - Spezies

```
Aufgabe 4(Zuordnungsaufgaben): Ordne (Hauptstadt) zu (Land)! -Paris/Frankreich -Berlin/Deutschland -Amsterdam/Niederlande;
```

### Aufgabe 5


```
Aufgabe 5(Umordnungsaufgabe): Sortiere Zahl nach Größe? -1 -3 -5 -8;
```

### Aufgabe 6

- Unterscheidung in Typen? Nur "Markieren"(1) oder "Markieren und Korrigieren"(2)
- ROBIN BITTE ERGÄNZEN

```
Aufgabe 6(Markierungsaufgabe): Markiere den Vornamen/Rechtschreibfehler! "Lorem ipsum "Jesus"(1) "Belin"\[Berlin\](2) Christus.";
```

### Aufgabe 7
- Lösung anzeigen, wenn Lösungsbrackett vorhanden sind, ansonsten nicht

```
Aufgabe 7(Lückentext): Fülle den Lückentext! (redundant, vielleicht hinzufügen der möglichen Wörter? (ATP-Synthese, Mülleimer, Energielieferanten, Mitochondrien): "Die \[Mitochondrien\] sind die Kraftwerke unserer Zellen und sind somit \[Energielieferanten\].";
```
<img width="999" height="537" alt="image" src="https://github.com/user-attachments/assets/1decedba-bc85-49cd-bd8c-8e287ceb3eeb" />

### Aufgabe 8

Nur für Dungeon entfällt. Alle haben sich geeinigt! Alle!


Wie sieht die allgemeine Aufgabenstruktur aus.
SUBJECT TO CHANGE:

AUFGABE NUMERAL(AUFGABENTYP): AUFGABENFRAGE ANTWORTEN;

## Syntax 

### Einzelne Frage pro Aufgabe
```
AUFGABENNAME(AUFGABENTYP): Die Vatikanstadt ist ein Land! -Richtig;
```

### Mehrere Fragen pro Aufgabe

```
AUFGABENNAME(AUFGABENTYP): Die Vatikanstadt ist ein Land! -Richtig\n
  Die Vatikanstadt ist ein Land! -Richtig\n
  Die Vatikanstadt ist ein Land! -Richtig\n
  Die Vatikanstadt ist ein Land! -Richtig;

AUFGABENNAME(AUFGABENTYP):\n
  Die Vatikanstadt ist ein Land! -Richtig\n
  Die Vatikanstadt ist ein Land! -Richtig\n
  Die Vatikanstadt ist ein Land! -Richtig;
```
## Meeting 2 (27.11.2025):

Implementierung erster Grammatiken in ANTLR

## Meeting 3 (30.11.2025):

Erweiterung der Grammatiken und Vorbereitung auf das Edmonton Meeting.
Festlegung der Rahmenbedingung der Präsentation und Erstellung eines vorläufigen Prototypen.

## Meeting 4 (20.12.2025)

Arbeit an der Grammatik

## Meeting 5 (04.01.2026)

- Fertigstellung der Grammatik
- Wichtiger Punkt: Es wird derzeit nicht beachtet, dass auch Klammern ( und ) so im Text vorkommen können.
- Großes Problem des Encodings -> lokaler Fix:
```
$env:JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF-8"
antlr4-parse .\DSL_Projekt\grammar\Aufgabenerstellungsgrammatik.g4 prog `
.\DSL_Projekt\grammar\testinput.txt -gui
```

### Todo:
- Punktesystem angelehnt an ILIAS, manuelle Punktevergabe per Aufgabe
- Single und Multiple Choice Questions hinzufügen
- Fix problem with multiple tasks

## Meeting 6 (13.01.2026)

- Arbeit an IR
- Festlegung der Klassen im Diagramm für IR
- Hinzufügen von Single und Multiple Choice Questions
- Fixed problem with multiple tasks

### Todo:
Add point system to grammar

## Meeting 7 (18.01.2026)

Problem: Wie balanced man Aufgabenpunkte in Relation zur Gesamtklausur. Einige Aufgaben 10 Punkte und andere nur 2 Punkte, zu starke Gewichtung innerhalb der Klausur.

- Added point system to grammar
- Implement IR, use inheritance and variants for task types
- Fix problem with words

## Meeting 8 (19.01.2026)

- Fertigstellung Präsentation
- Repo Pflege
- Ausbesserungen der Grammatik
- Fertigstellung des DSL Manuals
- Fertigstellung der Pipeline
- Finale Besprechung und Vorbereitung
- Eröffnung einer neuen sauberen Repo
