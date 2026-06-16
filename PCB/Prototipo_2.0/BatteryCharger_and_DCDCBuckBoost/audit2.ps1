$file = "Battery_and_DCDC.kicad_pcb"

$content = Get-Content -Raw $file

Write-Output "--- VIAS ---"
$matches = [regex]::Matches($content, '\(via.*?\s+\(at ([\d\.\-]+\s+[\d\.\-]+)\).*?\(net (\d+)\)', [System.Text.RegularExpressions.RegexOptions]::Singleline)
foreach ($m in $matches) {
    Write-Output "Via at $($m.Groups[1].Value), Net $($m.Groups[2].Value)"
}
