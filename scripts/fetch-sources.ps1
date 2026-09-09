param(
    [ValidateSet('libuv', 'node', 'all')]
    [string] $Component = 'libuv'
)

$ErrorActionPreference = 'Stop'

if (-not $env:NODE_RUNTIME_SOURCES) {
    . (Join-Path $PSScriptRoot 'activate-windows.ps1')
}

function Get-TaggedSource {
    param(
        [string] $Name,
        [string] $Url,
        [string] $Tag,
        [bool] $EnableSymlinks = $false
    )

    $target = Join-Path $env:NODE_RUNTIME_SOURCES $Name
    if (-not (Test-Path -LiteralPath (Join-Path $target '.git'))) {
        Write-Host "Cloning $Name at $Tag..."
        $arguments = @('clone', '--depth', '1', '--branch', $Tag)
        if ($EnableSymlinks) { $arguments += @('-c', 'core.symlinks=true') }
        $arguments += @($Url, $target)
        & git @arguments
    }
    else {
        Write-Host "Refreshing $Name at $Tag..."
        & git -C $target fetch --depth 1 origin "refs/tags/$Tag`:refs/tags/$Tag"
        & git -C $target checkout --detach $Tag
    }

    $commit = & git -C $target rev-parse --short HEAD
    Write-Host "$Name`t$commit`t$Tag"
}

switch ($Component) {
    'libuv' { Get-TaggedSource 'libuv' 'https://github.com/libuv/libuv.git' 'v1.52.1' }
    'node' { Get-TaggedSource 'node' 'https://github.com/nodejs/node.git' 'v24.20.0' $true }
    'all' {
        Get-TaggedSource 'libuv' 'https://github.com/libuv/libuv.git' 'v1.52.1'
        Get-TaggedSource 'node' 'https://github.com/nodejs/node.git' 'v24.20.0' $true
    }
}

Write-Host "`nSources are ready under $env:NODE_RUNTIME_SOURCES"
Write-Host 'Standalone V8 is intentionally fetched later with depot_tools.'
