# Setting Up Qt Visual Studio Tools Extension

This project uses Qt 6.10.1 with Visual Studio. To build the project, you need to install and configure the Qt Visual Studio Tools extension.

## Step 1: Install Qt Visual Studio Tools Extension

1. Open Visual Studio
2. Go to **Extensions** → **Manage Extensions**
3. Search for **"Qt Visual Studio Tools"**
4. Click **Download** (if not already installed)
5. Close Visual Studio to complete the installation
6. Restart Visual Studio

## Step 2: Install Qt 6.10.1

1. Download Qt 6.10.1 from https://www.qt.io/download
2. During installation, make sure to select:
   - **MSVC 2022 64-bit** component
   - The installation path (default is `C:\Qt\6.10.1\msvc2022_64`)

## Step 3: Configure Qt in Visual Studio

1. In Visual Studio, go to **Extensions** → **Qt VS Tools** → **Qt Versions**
2. Click **Add** or **New**
3. Set the following:
   - **Version name:** `6.10.1_msvc2022_64`
   - **Path:** `C:\Qt\6.10.1\msvc2022_64` (or your Qt installation path)
4. Click **OK**

## Step 4: Open the Project

1. Open `Mainwinow2.sln` in Visual Studio
2. The Qt Visual Studio Tools extension will automatically:
   - Detect the Qt installation
   - Set up QtMsBuild paths
   - Configure the project for Qt

## Step 5: Build the Project

1. Select **Debug** or **Release** configuration
2. Select **x64** platform
3. Build the solution (Ctrl+Shift+B)

## Troubleshooting

### If you still get designtime build errors:

1. Close Visual Studio
2. Delete the `.vs` folder in your project directory (if it exists)
3. Delete `Mainwinow2/Mainwinow2.vcxproj.user` (user-specific settings)
4. Reopen the project in Visual Studio
5. Right-click the project → **Qt Project Settings** → Verify Qt version is set correctly

### If Qt version is not detected:

1. Go to **Extensions** → **Qt VS Tools** → **Qt Versions**
2. Verify the path points to your Qt installation
3. Make sure the path contains `bin\qmake.exe`

## Notes

- The project file has been updated to work with Qt Visual Studio Tools extension
- No QtMsBuild files need to be committed to the repository
- Each developer needs to install Qt and configure the extension on their machine

