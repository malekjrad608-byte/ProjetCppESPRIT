param([string]$InputDoc, [string]$OutputPdf)
$ErrorActionPreference='Stop'
$logoWord=$null
$logoDocument=$null
try {
    $logoWord=New-Object -ComObject Word.Application
    $logoWord.Visible=$false
    $logoWord.DisplayAlerts=0
    $logoDocument=$logoWord.Documents.Open($InputDoc,$false,$true)
    $logoDocument.Repaginate()
    $logoDocument.ExportAsFixedFormat($OutputPdf,17)
    Write-Output ('Pages: '+$logoDocument.ComputeStatistics(2))
} finally {
    if($null -ne $logoDocument){$logoDocument.Close(0);[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($logoDocument)}
    if($null -ne $logoWord){$logoWord.Quit();[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($logoWord)}
}
