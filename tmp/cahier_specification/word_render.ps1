param([string]$InputDoc, [string]$OutputPdf)
$ErrorActionPreference='Stop'
$specWord=$null
$specDocument=$null
try {
    $specWord=New-Object -ComObject Word.Application
    $specWord.Visible=$false
    $specWord.DisplayAlerts=0
    $specDocument=$specWord.Documents.Open($InputDoc,$false,$false)
    $specDocument.Fields.Update() | Out-Null
    $specDocument.Repaginate()
    foreach($toc in $specDocument.TablesOfContents){$toc.Update()}
    $specDocument.Repaginate()
    foreach($toc in $specDocument.TablesOfContents){$toc.UpdatePageNumbers()}
    $specDocument.Save()
    $specDocument.ExportAsFixedFormat($OutputPdf,17)
    Write-Output ('Pages: '+$specDocument.ComputeStatistics(2))
    Write-Output ('Saved: '+$InputDoc)
    Write-Output ('QA PDF: '+$OutputPdf)
} finally {
    if($null -ne $specDocument){$specDocument.Close(0);[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($specDocument)}
    if($null -ne $specWord){$specWord.Quit();[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($specWord)}
}
