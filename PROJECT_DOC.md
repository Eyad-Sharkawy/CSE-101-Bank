Project: CSE (formerly Apex Trust Bank)

Overview

This is a Qt/C++ (C++17) desktop banking demo application. It provides a simple multi-page GUI for user registration, login, account modification, and a dashboard that shows balance and recent transactions. Data is stored in plain text files in the application's working directory: `users.txt` and `transactions.txt`.

High-level components

- `main.cpp` - Application entry point. Normally launches the Qt GUI. It now supports a command-line option to print this documentation and exit.
- `Mainwinow2.h` / `Mainwinow2.cpp` - The main QMainWindow that contains a `QStackedWidget` and coordinates page navigation between:
  - `LoginWindow`
  - `RegistrationWindow`
  - `DashboardWindow`
  - `ModifyAccountWindow`

- `LoginWindow` (UI & code)
  - Files: `LoginWindow.ui`, `login_window.h`, `login_window.cpp`
  - Responsibilities: allow users to login using username or account number, toggle password visibility (eye button), and navigate to registration.
  - Authentication: reads `users.txt` and validates password by computing SHA-256 of (password + accountNumber) and comparing to stored hash.

- `RegistrationWindow` (UI & code)
  - Files: `RegistrationWindow.ui`, `RegistrationWindow.cpp`
  - Responsibilities: create a new account. If account number is omitted a generated numeric id (timestamp-derived) is assigned.
  - On save: password is hashed (SHA-256) with the account number salt and appended to `users.txt` as: `Name|HashedPass|AccNum|Address|Balance`.

- `DashboardWindow` (UI & code)
  - Files: `dashboard_window.ui`, `dashboard_window.h`, `dashboard_window.cpp`
  - Responsibilities: show an account's balance, let user deposit/withdraw/transfer funds, display a recent transactions table with filters, export CSV, and live update when `transactions.txt` changes.
  - Transactions file format: `User|yyyy-MM-dd HH:mm|Type|Amount`.
  - Table formatting: human-friendly date (e.g., "February 3, 2021"), colored amounts (+ green, - red), filters for date range and type.
  - Live updates: implemented with `QFileSystemWatcher` watching `transactions.txt`.

- `ModifyAccountWindow` (UI & code)
  - Files: `modify_account_window.ui`, `modify_account_window.h`, `modify_account_window.cpp`
  - Responsibilities: edit user details (name, address, optionally change password). Passwords are never displayed; if a new password is set it is hashed and saved.

Data formats and examples

1) users.txt
- Format per line:
  Name|HashedPass|AccNum|Address|Balance
- Example line:
  alice|9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08|1001|123 Main St, Anytown|2500.00
- Notes:
  - `HashedPass` is the hex-encoded SHA-256 of (password + accNum).
  - Balance is stored as a decimal string with two fractional digits.

2) transactions.txt
- Format per line:
  User|yyyy-MM-dd HH:mm|Type|Amount
- Examples:
  alice|2021-02-03 00:00|Withdraw|-2500.00
  bob|2021-01-10 00:00|Deposit|15000.00
  alice|2021-01-07 00:00|Transfer|1000.00

How authentication works

- At login the app reads `users.txt` looking for a match by username or account number.
- To verify the password, it computes `SHA256(password + accNum)` and compares with the stored hash.
- Admin fallback: if `users.txt` is unreadable, the special credentials `admin/admin` still work for convenience.

Build and run (developer machine)

Prerequisites: Visual Studio (MSVC), Qt (matching MSVC toolchain). Example Qt path used in scripts: `C:\Qt\6.10.1\msvc2022_64`.

1) Automated script (recommended):
- `build_and_deploy.ps1` is supplied. From project root run in Developer PowerShell:
  powershell -NoProfile -ExecutionPolicy Bypass -File .\build_and_deploy.ps1 -QtPath "C:\Qt\6.10.1\msvc2022_64"
- The script will build the `Mainwinow2.vcxproj` (Release/x64) and run `windeployqt` to copy the Qt runtime next to the EXE.

2) Manual build steps:
- msbuild ".\Mainwinow2\Mainwinow2.vcxproj" /p:Configuration=Release /p:Platform=x64
- Run `windeployqt` on the produced EXE: 
  "C:\Qt\6.10.1\msvc2022_64\bin\windeployqt.exe" "C:\path\to\Mainwinow2.exe" --release

Command-line documentation option

- The application supports a CLI flag to print this documentation and exit. Run the built executable with:
  Mainwinow2.exe --describe
  or
  Mainwinow2.exe --doc

This prints the project documentation to stdout and exits (useful for CI or packaging notes).

Testing tips

- Create a new account, log in, make deposits/withdrawals and transfers between users. Use the Dashboard filters to view subsets of transactions.
- If the UI does not show new transactions after manual edits to `transactions.txt`, ensure the file uses the expected `yyyy-MM-dd HH:mm` timestamp format.
- For testing transfers, create two users and use the transfer flow. The app writes transaction entries for both sender and recipient.

Security notes and suggested improvements

- Current hashing: SHA-256(pass + accNum) provides basic protection but is not ideal. Use Argon2 or PBKDF2 with a random per-user salt and iterations for production.
- Switch from flat text files to a database (SQLite) to handle concurrent updates and avoid corruption.
- Protect `users.txt` with file permissions and/or encrypt sensitive fields.

Where to look in code for specific behaviors

- Login logic: `Mainwinow2/login_window.cpp`
- Registration: `Mainwinow2/RegistrationWindow.cpp`
- Dashboard, balance updates, transactions: `Mainwinow2/dashboard_window.cpp`
- Account modification: `Mainwinow2/modify_account_window.cpp`
- Application wiring and page navigation: `Mainwinow2/Mainwinow2.cpp`
- Entry point and CLI handling: `Mainwinow2/main.cpp`

Contact / Next steps

If you want, I can:
- Add richer per-user salt and PBKDF2/Argon2 hashing implementation.
- Migrate storage to SQLite and rewrite CRUD operations accordingly.
- Produce an installer (Inno Setup) for Windows with bundled Qt runtime.

