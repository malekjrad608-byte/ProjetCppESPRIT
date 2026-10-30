# Run manually when SQL Developer asks for the configured Oracle account password.
# The password never appears in this source file or in console output.
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Security
$directory=Join-Path $env:LOCALAPPDATA 'ANIWORLD\ANIWORLD'
$config=@{}
Get-Content -LiteralPath (Join-Path $directory 'oracle.ini') | ForEach-Object {
    if($_ -match '^([^=\[]+)=(.*)$') {$config[$matches[1].Trim()]=$matches[2].Trim()}
}
$path = Join-Path $directory $config.passwordFile
$encrypted = [IO.File]::ReadAllBytes($path)
$plain = [Security.Cryptography.ProtectedData]::Unprotect($encrypted,$null,[Security.Cryptography.DataProtectionScope]::CurrentUser)
Set-Clipboard -Value ([Text.Encoding]::UTF8.GetString($plain))
[Array]::Clear($plain,0,$plain.Length)
Write-Host "Mot de passe $($config.username) copie. Collez-le dans SQL Developer (Ctrl+V)."
