# perf\run_perf.ps1
$ErrorActionPreference = "Stop"

$exe = Resolve-Path ".\build\Release\aufgaben_dsl.exe"
$inDir  = Resolve-Path ".\perf\input"
$outDir = Resolve-Path ".\perf\output"

# Ausgabe leeren
Remove-Item -Force -Recurse -ErrorAction SilentlyContinue "$outDir\*" | Out-Null

function Percentile([double[]]$xs, [double]$p) {
    if ($xs.Count -eq 0) { return [double]::NaN }
    $sorted = $xs | Sort-Object
    $n = $sorted.Count
    $idx = [Math]::Ceiling($p * $n) - 1
    if ($idx -lt 0) { $idx = 0 }
    if ($idx -ge $n) { $idx = $n - 1 }
    return [double]$sorted[$idx]
}

$files = Get-ChildItem -Path $inDir -Filter "*.txt" | Sort-Object Name
if ($files.Count -eq 0) { throw "No inputs found in $inDir" }

# Warmup (nicht zählen) - Start-Process, damit stderr nicht als PowerShell-Error zählt
$warmIn  = $files[0].FullName
$warmOut = Join-Path $outDir "warmup.json"

Start-Process -FilePath $exe `
  -ArgumentList @($warmIn, $warmOut) `
  -NoNewWindow -Wait `
  -RedirectStandardOutput (Join-Path $outDir "warmup.stdout.txt") `
  -RedirectStandardError  (Join-Path $outDir "warmup.stderr.txt") | Out-Null

$timesMs = New-Object System.Collections.Generic.List[double]
$ok = 0
$fail = 0
$failSamples = @()

$swTotal = [System.Diagnostics.Stopwatch]::StartNew()

foreach ($f in $files) {
    $outPath = Join-Path $outDir ($f.BaseName + ".json")

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process -FilePath $exe -ArgumentList @($f.FullName, $outPath) -NoNewWindow -PassThru -Wait -RedirectStandardError (Join-Path $outDir ($f.BaseName + ".stderr.txt")) -RedirectStandardOutput (Join-Path $outDir ($f.BaseName + ".stdout.txt"))
    $sw.Stop()

    $ms = $sw.Elapsed.TotalMilliseconds
    $timesMs.Add($ms)

    if ($p.ExitCode -eq 0) {
        $ok++
    } else {
        $fail++
        if ($failSamples.Count -lt 5) {
            $errFile = Join-Path $outDir ($f.BaseName + ".stderr.txt")
            $err = ""
            if (Test-Path $errFile) { $err = (Get-Content $errFile -Raw) }
            $failSamples += [PSCustomObject]@{
                file = $f.Name
                exit = $p.ExitCode
                stderr = ($err -replace "\r?\n"," " | Select-Object -First 1)
            }
        }
    }
}

$swTotal.Stop()

$xs = $timesMs.ToArray()
$mean = ($xs | Measure-Object -Average).Average
$p50 = Percentile $xs 0.50
$p95 = Percentile $xs 0.95
$p99 = Percentile $xs 0.99
$totalMs = $swTotal.Elapsed.TotalMilliseconds
$throughput = ($files.Count / ($swTotal.Elapsed.TotalSeconds))

Write-Host ""
Write-Host "=== PERF SUMMARY ==="
Write-Host ("total={0}, ok={1}, fail={2}" -f $files.Count, $ok, $fail)
Write-Host ("total_ms={0:N1}" -f $totalMs)
Write-Host ("mean_ms={0:N2}  p50_ms={1:N2}  p95_ms={2:N2}  p99_ms={3:N2}" -f $mean, $p50, $p95, $p99)
Write-Host ("throughput={0:N2} files/sec" -f $throughput)

if ($fail -gt 0) {
    Write-Host ""
    Write-Host "=== FAIL SAMPLES (max 5) ==="
    $failSamples | Format-Table -AutoSize
}
