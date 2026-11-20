param(
    [string]$OracleHome = 'C:\app\malek\product\21c\dbhomeXE',
    [string]$Service = '//localhost:1521/XEPDB1'
)
$ErrorActionPreference = 'Stop'
# Local development provisioning. Never resets an existing Oracle account.
Add-Type -AssemblyName System.Security
$dataDirectory = Join-Path $env:LOCALAPPDATA 'ANIWORLD\ANIWORLD'
$config = Join-Path $dataDirectory 'oracle.ini'
if (Test-Path -LiteralPath $config) { throw 'oracle.ini exists; keep the existing account and configuration.' }
$bytes = New-Object byte[] 18
$rng = [Security.Cryptography.RandomNumberGenerator]::Create()
$rng.GetBytes($bytes)
$password = 'Aw_' + [Convert]::ToBase64String($bytes).Replace('+','A').Replace('/','B').Replace('=','C')
$sql = @"
whenever sqlerror exit failure rollback
set echo off verify off feedback off heading off
alter session set container=XEPDB1;
create user ANIWORLD_APP identified by "$password" default tablespace USERS quota 50M on USERS;
grant create session, create table, create sequence, create trigger to ANIWORLD_APP;
exit
"@
$result = $sql | & (Join-Path $OracleHome 'bin\sqlplus.exe') -L -s '/ as sysdba'
if ($LASTEXITCODE -ne 0) { throw ('Oracle setup failed: ' + (($result | Where-Object { $_ -match '^ORA-|^SP2-' }) -join "`n")) }
New-Item -ItemType Directory -Force -Path $dataDirectory | Out-Null
$encrypted = [Security.Cryptography.ProtectedData]::Protect([Text.Encoding]::UTF8.GetBytes($password),$null,[Security.Cryptography.DataProtectionScope]::CurrentUser)
[IO.File]::WriteAllBytes((Join-Path $dataDirectory 'oracle-password.dpapi'),$encrypted)
Add-OdbcDsn -Name 'ANIWORLD_ORACLE' -DriverName 'Oracle in OraDB21Home1' -DsnType User -Platform '64-bit' -SetPropertyValue @("ServerName=$Service",'UserID=ANIWORLD_APP')
@"
[oracle]
enabled=false
dsn=ANIWORLD_ORACLE
username=ANIWORLD_APP
home=$($OracleHome.Replace('\','/'))
passwordFile=oracle-password.dpapi
"@ | Set-Content -LiteralPath $config -Encoding utf8
Write-Output 'ANIWORLD_APP created in XEPDB1; 64-bit user DSN ready; password protected with Windows DPAPI. Activation awaits migration tests.'
