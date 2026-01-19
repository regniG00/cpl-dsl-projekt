
# DSL Manual: Aufgabenerstellungsgrammatik

This DSL (Domain-Specific Language) allows you to define educational tasks of various types such as Cloze, Matching, Sorting, Marking, Correction, Choice, and True/False.

Each task begins with a header in the form:

```text
<Titel>(<TaskType>): <Optional Description>
```

The body of the task depends on the task type and ends with a `;`.

---

## Supported Task Types

| Type Name         | DSL Keyword    | Description |
|------------------|----------------|-------------|
| True/False        | `RoF`          | Richtig/Falsch mit Begründung |
| Sorting           | `Umordnung`    | Sortieraufgaben |
| Matching          | `Zuordnung`    | Zuordnungsaufgaben |
| Marking           | `Markierung`   | Wörter markieren oder hervorheben |
| Cloze Text        | `Lückentext`   | Lückentextaufgaben |
| Correction Text   | `Textkorrektur`| Rechtschreibfehler korrigieren |
| Multiple/Single Choice | `Auswahl` | Auswahlaufgaben (mit Punkten) |

---

## General Syntax Rules

- **Each task ends with a `;`**
- **Use NEWLINE (`\n`) between lines**
- **Sentences must end with `.`, `?`, or `!`**
- **Points**: Specify with `(number)` after the word or sentence.
- **Words**: Alphanumeric, including letters with German umlauts.

---

## Task Type Examples

### 1. True/False (`RoF`)

```text
Aufgabe XYZ(RoF): 
    Berlin ist die Haupstadt von Deutschland. -Richtig
    Paris ist die Haupstadt von England. -Falsch -> Paris ist die Haupstadt von Frankreich.;
```

- `-Richtig` or `-Falsch`
- If `-Falsch`, must be followed by `-> <Reason>`

---

### 2. Sorting (`Umordnung`)

```text
Sortieren(Umordnung):
    Sortiere die Zahlen aufsteigend nach ihrer größe.(2) -1 -2 -3
    Sortiere die Zahlen aufsteigend nach ihrer größe. -1 -2 -3 -4 -5 -6 -7 -8;
```

- Prefix each sortable item with `-`
- Optional points: `(2)` means full points only if correct order

---

### 3. Matching (`Zuordnung`)

```text
Aufgabe 4(Zuordnung):
    Ordne (Hauptstadt) zu (Land)!(3) -Paris/Frankreich -Berlin/Deutschland -Amsterdam/Niederlande
    Ordne (Hauptstadt) zu (Land)! -Paris/Frankreich -Berlin/Deutschland -Amsterdam/Niederlande;
```

- Pair items with `/`
- Each line can optionally define full points `(3)`

---

### 4. Marking (`Markierung`)

```text
Aufgabe 4(Markierung): Markiere alle Autoren.
    (Stephen King)[1] ist ein Buchautor.;

Aufgabe 5(Markierung): Markiere alle Rechtschreibfehler.
    Die Haupstadt von Deutschland ist (Baerlin)[Berlin,1].
    Ein Vogel kann (vliegen)[fliegen,2].;
```

- Marked words: `(incorrect_text)[correct_text,points]` or `(word)[points]`

---

### 5. Correction Text (`Textkorrektur`)

```text
Aufgabe 6(Textkorrektur): Korrigiere die Rechtschreibfehler im Text.
    Die Haupstadt von Deutschland ist (Baerlin)[Berlin,1].
    Ein Vogel kann (vliegen)[fliegen,2].;
```

- Same format as `Markierung`, but user is expected to correct the words manually.

---

### 6. Cloze Task (`Lückentext`)

```text
Aufgabe Cloze(Lückentext):Fuelle die Luecken aus.
    Niklas ist(ein,1) toller Mensch.;
```

- Cloze words: `(correct_word,points)`

---

### 7. Multiple/Single Choice (`Auswahl`)

```text
MultipleChoice (Auswahl):
    Wie viele Bits sind in einem Byte. -8 Bit(2) -4 Bit -1 Bit -16 Bit
    Welche Städte sind in Deutschland? -Berlin(1) -Minden(1) -Paris -London 
    Welche Städte sind in Deutschland? -Berlin(1) -Minden(1) -Paris(-1) -London(-1);
```

- Correct answers: `-Answer(points)`
- Incorrect answers: optionally `-Answer(-points)`

---

## Building Blocks

### Sentences

- Must end with `.`, `!` or `?`
- Example:  
  ```text
  Das ist ein Satz.
  ```

### Words

- Alphanumeric including umlauts (`äöüß`)
- Separated by whitespace

### Connections

Allowed separators in `endless_words`: `,` `:` `-` `;`

---

## Errors & Notes

- Every **sentence must end** with a punctuation mark (`.`, `!`, or `?`)
- **True/False** explanations **must be full sentences** ending with punctuation.
- Always **close tasks with a `;`**

---

## Tips for Writing Tasks

- Make sure all sentences are syntactically correct.
- Use consistent spacing and punctuation.
- Do not leave out required punctuation or points if expected.
- Each task block must begin with a `(<TaskType>)` label.
