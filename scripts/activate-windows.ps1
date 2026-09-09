$ErrorActionPreference = 'Stop'

if (-not $env:NODE_RUNTIME_LAB_HOME) {
    if (Test-Path -LiteralPath 'D:\') {
        $env:NODE_RUNTIME_LAB_HOME = 'D:\node-runtime-lab-data'
    }
    else {
        $env:NODE_RUNTIME_LAB_HOME = Join-Path $env:LOCALAPPDATA 'node-runtime-lab-data'
    }
}

$env:NODE_RUNTIME_SOURCES = Join-Path $env:NODE_RUNTIME_LAB_HOME 'sources'
$env:NODE_RUNTIME_BUILD = Join-Path $env:NODE_RUNTIME_LAB_HOME 'build'
$env:NODE_RUNTIME_TRACES = Join-Path $env:NODE_RUNTIME_LAB_HOME 'traces'
$env:NODE_RUNTIME_TOOLS = Join-Path $env:NODE_RUNTIME_LAB_HOME 'tools'

@(
    $env:NODE_RUNTIME_SOURCES,
    $env:NODE_RUNTIME_BUILD,
    $env:NODE_RUNTIME_TRACES,
    $env:NODE_RUNTIME_TOOLS
) | ForEach-Object {
    New-Item -ItemType Directory -Force -Path $_ | Out-Null
}

Write-Host 'Node Runtime Lab Windows environment activated'
Write-Host "  sources: $env:NODE_RUNTIME_SOURCES"
Write-Host "  build:   $env:NODE_RUNTIME_BUILD"
Write-Host "  traces:  $env:NODE_RUNTIME_TRACES"
