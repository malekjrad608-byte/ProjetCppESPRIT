$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Security
$dataDirectory = Join-Path $env:LOCALAPPDATA 'ANIWORLD\ANIWORLD'
$config=@{}
Get-Content -LiteralPath (Join-Path $dataDirectory 'oracle.ini') | ForEach-Object {
    if($_ -match '^([^=\[]+)=(.*)$') {$config[$matches[1].Trim()]=$matches[2].Trim()}
}
if($config.username -notin @('rawand','ANIWORLD_APP')) {throw 'Unsupported application schema.'}
$dsn=Get-OdbcDsn -Name $config.dsn -DsnType User -Platform '64-bit'
$service=$dsn.Attribute['ServerName']
$encrypted = [IO.File]::ReadAllBytes((Join-Path $dataDirectory $config.passwordFile))
$password = [Text.Encoding]::UTF8.GetString([Security.Cryptography.ProtectedData]::Unprotect($encrypted,$null,[Security.Cryptography.DataProtectionScope]::CurrentUser))
$schema = Join-Path $PSScriptRoot '..\database\oracle\schema.sql'
$env:NLS_LANG = '.AL32UTF8'
$OutputEncoding = [Text.UTF8Encoding]::new($false)
$script = "whenever sqlerror exit failure rollback`nset echo off verify off define off`nconnect $($config.username)/`"$password`"@$service`nDECLARE n NUMBER; BEGIN SELECT COUNT(*) INTO n FROM user_tables; IF n<>0 THEN RAISE_APPLICATION_ERROR(-20001,'Schema must be empty; no existing tables replaced'); END IF; END;`n/`n@schema.sql`nexit"
Push-Location (Split-Path $schema)
try {$result = $script | & (Join-Path $config.home 'bin/sqlplus.exe') -L -s /nolog; $code=$LASTEXITCODE}
finally {Pop-Location}
$result | Where-Object { $_ -notmatch [regex]::Escape($password) }
if ($code -ne 0 -or ($result -match '^SP2-')) { throw 'Schema installation failed. Inspect the error; do not delete existing tables.' }
