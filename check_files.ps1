$files = Get-ChildItem 'C:\Programs\mhook' -Filter '*.mhook' | Sort-Object -Property LastWriteTime -Descending | Select-Object -First 50
$sorted = $files | Sort-Object -Property Name
Write-Host "Total: $($sorted.Count)"
foreach ($f in $sorted) {
    Write-Host "$($f.Name) - $($f.LastWriteTime)"
}