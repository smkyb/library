[CmdletBinding()]
param(
    [switch]$Push,
    [switch]$DryRun,
    [switch]$IncludeAll,
    [string]$Message = "",
    [string]$Src = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

$repoRoot = Split-Path -Parent $PSScriptRoot

Push-Location $repoRoot
try {
    if ($Push -and $DryRun) {
        throw "-Push と -DryRun は同時に指定できません。"
    }

    $syncArgs = @("scripts/sync.mjs")
    if ($DryRun) {
        $syncArgs += "--dry-run"
    }
    if (-not [string]::IsNullOrWhiteSpace($Src)) {
        $syncArgs += "--src"
        $syncArgs += $Src
    }

    if (-not $Push) {
        & node @syncArgs
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
        Write-Host ""
        Write-Host "--- git status ---"
        git status --short
        exit 0
    }

    $preStatus = @(git status --porcelain)
    if ($LASTEXITCODE -ne 0) {
        throw "git status の実行に失敗しました。"
    }

    if (-not $IncludeAll) {
        $outsideChanges = @()
        foreach ($line in $preStatus) {
            if ([string]::IsNullOrWhiteSpace($line)) { continue }
            $pathPart = $line.Substring(3).Trim()
            if ($pathPart -match '^libraries(/|\\)' -or $pathPart -eq 'libraries') {
                continue
            }
            $outsideChanges += $line
        }
        if ($outsideChanges.Count -gt 0) {
            Write-Error ("libraries/ 以外に未コミットの変更があるため中断しました。先にコミットするか -IncludeAll を付けて実行してください:`n" + ($outsideChanges -join "`n"))
            exit 1
        }
    }

    & node @syncArgs
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    $postStatus = @(git status --porcelain)
    if ($postStatus.Count -eq 0) {
        Write-Host "同期による差分はありません。"
        exit 0
    }

    Write-Host ""
    Write-Host "ビルド検証を実行しています..."
    npm run build
    if ($LASTEXITCODE -ne 0) {
        throw "npm run build が失敗したため commit / push を中止しました。"
    }

    if ($IncludeAll) {
        git add -A
    } else {
        git add -- libraries/
    }
    if ($LASTEXITCODE -ne 0) {
        throw "git add に失敗しました。"
    }

    $stagedStatus = @(git diff --cached --name-status)
    if ($stagedStatus.Count -eq 0) {
        Write-Host "ステージされた差分がありません。"
        exit 0
    }

    if ([string]::IsNullOrWhiteSpace($Message)) {
        $addedCount = @($stagedStatus | Where-Object { $_ -match '^A\s' }).Count
        $modifiedCount = @($stagedStatus | Where-Object { $_ -match '^M\s' }).Count
        $deletedCount = @($stagedStatus | Where-Object { $_ -match '^D\s' }).Count
        $Message = "sync: update libraries (+$addedCount ~$modifiedCount -$deletedCount)"
    }

    git commit -m $Message
    if ($LASTEXITCODE -ne 0) {
        throw "git commit に失敗しました。"
    }

    git push
    if ($LASTEXITCODE -ne 0) {
        throw "git push に失敗しました。"
    }

    Write-Host "同期・検証・push が完了しました: $Message"
} finally {
    Pop-Location
}
