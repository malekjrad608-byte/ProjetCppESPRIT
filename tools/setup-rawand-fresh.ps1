param([Parameter(Mandatory)][Security.SecureString]$Password)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Security
$oracleHome='C:/app/malek/product/21c/dbhomeXE'
$dsn='Source_Projet2A18'
$username='rawand'
$service='//localhost:1521/XE'
$dataDirectory=Join-Path $env:LOCALAPPDATA 'ANIWORLD/ANIWORLD'
$backup=Join-Path $PSScriptRoot ('../backups/before-rawand-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $backup -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $dataDirectory 'oracle.ini'),(Join-Path $dataDirectory 'oracle-password.dpapi') -Destination $backup
Get-OdbcDsn -Name $dsn -DsnType User -Platform '64-bit' | Export-Clixml -LiteralPath (Join-Path $backup 'dsn.xml')
$credential=[PSCredential]::new($username,$Password)
$plain=$credential.GetNetworkCredential().Password
$schema=(Resolve-Path (Join-Path $PSScriptRoot '../database/oracle/schema.sql')).Path
$sql=@"
whenever sqlerror exit failure rollback
set echo off verify off define off
connect $username/"$plain"@$service
DECLARE n NUMBER; BEGIN
 IF USER<>'RAWAND' OR SYS_CONTEXT('USERENV','CON_NAME')<>'CDB`$ROOT' THEN RAISE_APPLICATION_ERROR(-20001,'Expected RAWAND on XE'); END IF;
 SELECT COUNT(*) INTO n FROM user_tables;
 IF n<>0 THEN RAISE_APPLICATION_ERROR(-20002,'Schema must be empty; no existing data replaced'); END IF;
END;
/
@"$schema"
"@
foreach($account in @('admin','services','stock','employes','locaux','medical')) {
    $salt=[byte[]]::new(24)
    [Security.Cryptography.RandomNumberGenerator]::Fill($salt)
    $derive=[Security.Cryptography.Rfc2898DeriveBytes]::new(($account+'123'),$salt,100000,[Security.Cryptography.HashAlgorithmName]::SHA256)
    $hash=$derive.GetBytes(32);$derive.Dispose()
    $saltHex=[BitConverter]::ToString($salt).Replace('-','')
    $hashHex=[BitConverter]::ToString($hash).Replace('-','')
    $sql+="`nINSERT INTO accounts VALUES('$account','$account',HEXTORAW('$saltHex'),HEXTORAW('$hashHex'),100000);"
}
$sql+="`nUPDATE collection_versions SET revision=1;`nINSERT INTO integration_migrations VALUES('fresh-oracle-v1');`nCOMMIT;`nSELECT USER,SYS_CONTEXT('USERENV','CON_NAME') FROM dual;`nexit`n"
$env:NLS_LANG='.AL32UTF8';$OutputEncoding=[Text.UTF8Encoding]::new($false)
Push-Location (Split-Path $schema)
try { $result=$sql | & (Join-Path $oracleHome 'bin/sqlplus.exe') -L -S /nolog; $code=$LASTEXITCODE }
finally { Pop-Location }
$result | Where-Object {$_ -notmatch [regex]::Escape($plain)}
if($code -ne 0 -or ($result -match '^SP2-')) {throw 'Installation failed. Configuration Qt was not switched; inspect SQL errors.'}
Set-OdbcDsn -Name $dsn -DsnType User -Platform '64-bit' -SetPropertyValue @("ServerName=$service","UserID=$username")
$protected=[Security.Cryptography.ProtectedData]::Protect([Text.Encoding]::UTF8.GetBytes($plain),$null,[Security.Cryptography.DataProtectionScope]::CurrentUser)
[IO.File]::WriteAllBytes((Join-Path $dataDirectory 'oracle-rawand-password.dpapi'),$protected)
$config="[oracle]`nenabled=true`ndsn=$dsn`nusername=$username`nhome=$oracleHome`npasswordFile=oracle-rawand-password.dpapi`n"
[IO.File]::WriteAllText((Join-Path $dataDirectory 'oracle.ini'),$config,[Text.UTF8Encoding]::new($false))
$plain=$null;$sql=$null
Write-Output 'Fresh schema RAWAND ready; Qt now uses Source_Projet2A18. No old business data imported.'
