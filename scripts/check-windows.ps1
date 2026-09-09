$ErrorActionPreference = 'Continue'
$script:Failures = 0
$script:Warnings = 0

function Write-Pass([string] $Message) {
    Write-Host "[PASS] $Message" -ForegroundColor Green
}

function Write-Warn([string] $Message) {
    $script:Warnings++
    Write-Host "[WARN] $Message" -ForegroundColor Yellow
}

function Write-Fail([string] $Message) {
    $script:Failures++
    Write-Host "[FAIL] $Message" -ForegroundColor Red
}

function Test-Tool([string] $Name, [bool] $Required = $true) {
    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        Write-Pass "$Name`: $($command.Source)"
        return $true
    }

    if ($Required) { Write-Fail "$Name is missing from PATH" }
    else { Write-Warn "optional $Name is missing from PATH" }
    return $false
}

Write-Host '== Platform =='
if ([Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT) {
    Write-Pass "Windows $([Environment]::OSVersion.Version) $env:PROCESSOR_ARCHITECTURE"
}
else {
    Write-Fail 'This check targets native Windows'
}

Write-Host "`n== Required tools =="
$hasGit = Test-Tool git
$hasCMake = Test-Tool cmake
$hasNode = Test-Tool node
$hasNpm = Test-Tool npm
$hasPython = (Test-Tool python -Required $false) -or (Test-Tool py -Required $false)

if (-not $hasPython) {
    Write-Fail 'Python is required before building Node or V8 from source'
}

$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
if (Test-Path -LiteralPath $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath
    $vsVersion = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationVersion
    if ($vsPath) {
        Write-Pass "Visual Studio C++ workload: $vsPath"
        $vsMajor = [int]($vsVersion -split '\.')[0]
        if ($vsMajor -ge 17) { Write-Pass "Visual Studio version supports Node 24 source builds: $vsVersion" }
        else { Write-Warn "Visual Studio 2022 or 2026 is required for Node 24; found $vsVersion" }
    }
    else { Write-Fail 'Visual Studio Desktop development with C++ workload is missing' }

    $clangPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Llvm.Clang `
        -requires Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset `
        -property installationPath
    if ($clangPath) { Write-Pass 'Visual Studio ClangCL components are installed' }
    else { Write-Warn 'ClangCL components are required for a Node 24 Windows source build' }
}
else {
    Write-Fail 'Visual Studio Installer/vswhere was not found'
}

Test-Tool dumpbin -Required $false | Out-Null
Test-Tool wpr -Required $false | Out-Null
Test-Tool cdb -Required $false | Out-Null

Write-Host "`n== Runtime baseline =="
if ($hasNode) {
    $nodeVersion = & node -p 'process.version'
    $uvVersion = & node -p 'process.versions.uv'
    $v8Version = & node -p 'process.versions.v8'
    Write-Host "       Node $nodeVersion"
    Write-Host "       libuv $uvVersion"
    Write-Host "       V8 $v8Version"

    if ($nodeVersion -eq 'v24.20.0') { Write-Pass 'Node baseline matches' }
    else { Write-Warn 'expected Node v24.20.0' }
    if ($uvVersion -eq '1.52.1') { Write-Pass 'libuv baseline matches' }
    else { Write-Warn 'expected libuv 1.52.1' }
    if ($v8Version -eq '13.6.233.17-node.53') { Write-Pass 'V8 baseline matches' }
    else { Write-Warn 'expected V8 13.6.233.17-node.53' }
}

Write-Host "`n== Lab directories =="
if (-not $env:NODE_RUNTIME_LAB_HOME) {
    Write-Fail 'Run: . .\scripts\activate-windows.ps1'
}
else {
    foreach ($name in @('sources', 'build', 'traces', 'tools')) {
        $path = Join-Path $env:NODE_RUNTIME_LAB_HOME $name
        if (Test-Path -LiteralPath $path) { Write-Pass "exists: $path" }
        else { Write-Fail "missing: $path" }
    }
}

Write-Host "`nSummary: $script:Failures failure(s), $script:Warnings warning(s)"
if ($script:Failures -gt 0) { exit 1 }
