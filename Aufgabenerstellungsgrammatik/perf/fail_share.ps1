# perf\fail_share.ps1
# ---------------------------------------------
# Split examples.txt into single-task files,
# run aufgaben_dsl.exe per task, count ok/fail,
# and track timings.
# ---------------------------------------------

$ErrorActionPreference = "Stop"

$root = Get-Location
$exe  = Join-Path $root "build\Release\aufgaben_dsl.exe"

$inputFile = Join-Path $root "perf\examples.txt"
$tmpInDir  = Join-Path $root "perf\tmp_in"
$tmpOutDir = Join-Path $root "perf\tmp_out"

if (-not (Test-Path $exe)) { throw "Executable not found: $exe" }
if (-not (Test-Path $inputFile)) { throw "Input file not found: $inputFile" }

Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $tmpInDir, $tmpOutDir
New-Item -ItemType Directory -Path $tmpInDir  | Out-Null
New-Item -ItemType Directory -Path $tmpOutDir | Out-Null

function Percentile([double[]]$xs, [double]$p) {
    if ($xs.Count -eq 0) { return [double]::NaN }
    $sorted = $xs | Sort-Object
    $n = $sorted.Count
    $idx = [Math]::Ceiling($p * $n) - 1
    if ($idx -lt 0) { $idx = 0 }
    if ($idx -ge $n) { $idx = $n - 1 }
    return [double]$sorted[$idx]
}

# 1) Read & split corpus
$raw = Get-Content $inputFile -Raw

# Split by "Aufgabe <number>("
$blocks = [regex]::Split($raw, '(?=Aufgabe\s+\d+\s*\()') |
    Where-Object { $_.Trim().Length -gt 0 }

$total = $blocks.Count
Write-Host "Found $total Aufgaben in examples.txt"
Write-Host ""

# 2) Warmup (first block) - not counted
$warmContent = ($blocks[0] -split "`r?`n") |
    ForEach-Object { $_.TrimEnd() } |
    Where-Object { $_.Trim().Length -gt 0 } |
    ForEach-Object { $_ } |
    Out-String
$warmContent = $warmContent.Trim()

$warmIn  = Join-Path $tmpInDir  "warmup.txt"
$warmOut = Join-Path $tmpOutDir "warmup.json"
Set-Content -Encoding Byte -Path $warmIn -Value ([System.Text.Encoding]::GetEncoding(1252).GetBytes($warmContent))

Start-Process -FilePath $exe -ArgumentList @($warmIn, $warmOut) -NoNewWindow -Wait `
    -RedirectStandardOutput (Join-Path $tmpOutDir "warmup.stdout.txt") `
    -RedirectStandardError  (Join-Path $tmpOutDir "warmup.stderr.txt") | Out-Null

# 3) Run all tasks
$ok = 0
$fail = 0
$fails = New-Object System.Collections.Generic.List[object]
$timesMs = New-Object System.Collections.Generic.List[double]

$swTotal = [System.Diagnostics.Stopwatch]::StartNew()

for ($i = 0; $i -lt $blocks.Count; $i++) {
    $idx = $i + 1

    # IMPORTANT: remove leading/trailing blank lines & pure-whitespace lines
    $content = ($blocks[$i] -split "`r?`n") |
        ForEach-Object { $_.TrimEnd() } |
        Where-Object { $_.Trim().Length -gt 0 } |
        Out-String
    $content = $content.Trim()

    $inPath  = Join-Path $tmpInDir  ("task_{0:000}.txt"  -f $idx)
    $outPath = Join-Path $tmpOutDir ("task_{0:000}.json" -f $idx)

    Set-Content -Encoding Byte -Path $inPath -Value ([System.Text.Encoding]::GetEncoding(1252).GetBytes($content))
    Remove-Item -Force -ErrorAction SilentlyContinue $outPath

    $stderr = Join-Path $tmpOutDir ("task_{0:000}.stderr.txt" -f $idx)
    $stdout = Join-Path $tmpOutDir ("task_{0:000}.stdout.txt" -f $idx)

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process -FilePath $exe -ArgumentList @($inPath, $outPath) -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $sw.Stop()

    $ms = $sw.Elapsed.TotalMilliseconds
    $timesMs.Add($ms)

    if ($p.ExitCode -eq 0 -and (Test-Path $outPath)) {
        $ok++
    } else {
        $fail++
        $errLine = ""
        if (Test-Path $stderr) {
            $errLine = (Get-Content $stderr -Raw) -replace "\r?\n"," "
        }
        $fails.Add([PSCustomObject]@{
            Aufgabe = $idx
            Exit    = $p.ExitCode
            Ms      = [Math]::Round($ms, 2)
            Error   = ($errLine.Trim() | Select-Object -First 1)
        })
    }
}

$swTotal.Stop()

# 4) Report
$xs = $timesMs.ToArray()
$mean = ($xs | Measure-Object -Average).Average
$p50 = Percentile $xs 0.50
$p95 = Percentile $xs 0.95
$p99 = Percentile $xs 0.99
$totalMs = $swTotal.Elapsed.TotalMilliseconds
$throughput = ($total / $swTotal.Elapsed.TotalSeconds)

Write-Host ""
Write-Host "========== RESULT =========="
Write-Host ("Total Aufgaben : {0}" -f $total)
Write-Host ("OK             : {0}" -f $ok)
Write-Host ("FAILED         : {0}" -f $fail)
Write-Host ("Success rate   : {0:P2}" -f ($ok / [double]$total))
Write-Host ("Total time     : {0:N1} ms" -f $totalMs)
Write-Host ("Mean/P50/P95/P99: {0:N2} / {1:N2} / {2:N2} / {3:N2} ms" -f $mean, $p50, $p95, $p99)
Write-Host ("Throughput     : {0:N2} tasks/sec" -f $throughput)
Write-Host "============================"

if ($fail -gt 0) {
    Write-Host ""
    Write-Host "Failed Aufgaben (first 20):"
    $fails | Select-Object -First 20 | Format-Table -AutoSize
    Write-Host ""
    Write-Host "See stderr logs in perf\tmp_out\"
}
