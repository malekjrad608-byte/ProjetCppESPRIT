# ANIWORLD - Gestion des Dons + PETSS Animated Login

The project starts with the PETSS animated login and opens the ANIWORLD donation dashboard after successful authentication.

## Local user database

The login now uses a local SQLite database named `aniworld_users.db`. It is created automatically in the application's writable data directory on first launch. The database is initialized with these demo accounts:

| Username | Password | Role |
|---|---|---|
| `admin` | `admin123` | Administrator |
| `manager` | `manager123` | Manager |
| `staff` | `staff123` | Staff |

Recovery also checks the email/phone stored in the database and updates the password in SQLite.

For a school/demo project, the passwords are stored as plain text in the local SQLite database. For a real production application, passwords should be salted and hashed.

## Qt

Designed for Qt 6.12.0 MinGW 64-bit / Qt Creator 20.0.2.
