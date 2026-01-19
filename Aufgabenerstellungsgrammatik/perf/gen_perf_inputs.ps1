# perf\gen_perf_inputs.ps1
$ErrorActionPreference = "Stop"

$inDir  = Join-Path $PSScriptRoot "input"
$outDir = Join-Path $PSScriptRoot "output"

New-Item -ItemType Directory -Force -Path $inDir  | Out-Null
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

# Basis-DSL (leicht parametrisiert über {{ID}})
$base = @"
Aufgabe {{ID}}(Lückentext): Fülle die Lücken aus.
    CPL ist ein (tolles,2) Modul.
    Ein Vogel kann (fliegen,1).;
Aufgabe {{ID}}(Zuordnung):
    Ordne (Hauptstadt) zu (Land)!(3) -Paris/Frankreich -Berlin/Deutschland -Amsterdam/Niederlande
    Ordne (Hauptstadt) zu (Land)! -Paris/Frankreich -Berlin/Deutschland -Amsterdam/Niederlande;
Testaufgabe {{ID}}(RoF): 
    Berlin ist die Haupstadt von Deutschland. -Richtig
    Paris ist die Haupstadt von England. -Falsch -> Paris ist die Haupstadt von Frankreich.
    Diese Aufgabe ist super schwierig und Richtig. -Richtig;
Sortieren {{ID}}(Umordnung):
    Sortiere die Zahlen aufsteigend nach ihrer größe.(2) -1 -2 -3
    Sortiere die Zahlen aufsteigend nach ihrer größe. -1 -2 -3 -4 -5 -6 -7 -8;
Aufgabe {{ID}}(Markierung): Markiere alle Autoren.
    (Stephen King)[1] ist ein Buchautor.;
Aufgabe {{ID}}(Markierung): Markiere alle Rechtschreibfehler.
    Die Haupstadt von Deutschland ist (Baerlin)[Berlin,1].
    Ein Vogel kann (vliegen)[fliegen,2].;
Aufgabe {{ID}}(Textkorrektur): Korrigiere die Rechtschreibfehler im Text.
    Die Haupstadt von Deutschland ist (Baerlin)[Berlin,1].
    Ein Vogel kann (vliegen)[fliegen,2].;
Aufgabe {{ID}}(Auswahl):
    Wie viele Bits sind in einem Byte. -8 Bit(2) -4 Bit -1 Bit -16 Bit
    Welche Städte sind in Deutschland? -Berlin(1) -Minden(1) -Paris(-1) -London(-1);
"@

# 100 Dateien erzeugen
1..100 | ForEach-Object {
    $id = $_
    $txt = $base.Replace("{{ID}}", $id.ToString())
    $path = Join-Path $inDir ("case_{0:000}.txt" -f $id)
    Set-Content -Encoding Byte -Path $path -Value ([System.Text.Encoding]::GetEncoding(1252).GetBytes($txt))
}

Write-Host "Generated 100 inputs in $inDir"
