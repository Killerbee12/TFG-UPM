$file = "Battery_and_DCDC.kicad_pcb"

Write-Output "--- TRACE WIDTHS ---"
$in_segment = $false
$width = ""
$net = ""
$results = @{}
switch -Regex -File $file {
    '^\s*\(segment' { $in_segment = $true; $width = ""; $net = "" }
    '^\s*\(width ([\d\.]+)\)' { if ($in_segment) { $width = $matches[1] } }
    '^\s*\(net (\d+)\)' { if ($in_segment) { $net = $matches[1] } }
    '^\s*\)' { 
        if ($in_segment) { 
            if ($net -ne "") {
                if (-not $results.ContainsKey($net)) { $results[$net] = @{} }
                $results[$net][$width] = $true
            }
            $in_segment = $false 
        } 
    }
}
foreach ($k in $results.Keys | Sort-Object { [int]$_ }) {
    $widths = $results[$k].Keys -join ", "
    Write-Output "Net $($k): $widths"
}

$content = Get-Content -Raw $file

Write-Output ""
Write-Output "--- FOOTPRINT POSITIONS ---"
$matches = [regex]::Matches($content, '\(footprint.*?(?=\n\s*\((footprint|segment|via|zone)|\z)', [System.Text.RegularExpressions.RegexOptions]::Singleline)
foreach ($m in $matches) {
    $block = $m.Value
    $ref = ""
    $at = ""
    if ($block -match '\(property "Reference" "([^"]+)"') { $ref = $matches[1] }
    if ($block -match '\(at ([\d\.\-]+\s+[\d\.\-]+)') { $at = $matches[1] }
    if ($ref -ne "") { Write-Output "$($ref): $at" }
}

Write-Output ""
Write-Output "--- ZONES ---"
$matches = [regex]::Matches($content, '\(zone.*?(?=\n\s*\((zone|segment|via)|\z)', [System.Text.RegularExpressions.RegexOptions]::Singleline)
foreach ($m in $matches) {
    $block = $m.Value
    $net_name = ""
    $layer = ""
    if ($block -match '\(net_name "([^"]+)"\)') { $net_name = $matches[1] }
    if ($block -match '\(layer "([^"]+)"\)') { $layer = $matches[1] }
    Write-Output "Zone: Layer $layer, Net $net_name"
}
