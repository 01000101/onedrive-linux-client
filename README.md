# OneDrive Linux Client

A Microsoft OneDrive client for Linux built with Qt 6 and QML.

Browse, navigate, and download files from your OneDrive directly on Linux.

## Features

- OAuth 2.0 authentication with Microsoft accounts
- Browse folders and files with visual icons
- View storage quota
- Download files to your local system
- Open files/folders in the web browser
- View file details (size, dates, type)
- Back/Home navigation with breadcrumb path display

## Demo

[![Watch the video](https://raw.githubusercontent.com/01000101/onedrive-linux-client/master/screenshots/demo.png)](https://raw.githubusercontent.com/01000101/onedrive-linux-client/master/screenshots/demo.mp4)


## Requirements

- Qt 6 (qt6-base-dev, qt6-declarative-dev)
- QML modules (qml6-module-qtquick-controls, qml6-module-qtquick-dialogs, qml6-module-qtquick-layouts)
- C++17 compiler (g++ or clang++)
- qmake6

### Ubuntu/Debian Installation

```bash
sudo apt install qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick-controls qml6-module-qtquick-dialogs \
    qml6-module-qtquick-layouts qml6-module-qtquick-window \
    build-essential
```

## Azure App Registration

Before building, you need to register an application with Microsoft:

1. Go to the [Azure Portal - App Registrations](https://portal.azure.com/#blade/Microsoft_AAD_RegisteredApps/ApplicationsListBlade)

2. Click **"New registration"**

3. Configure your app:
   - **Name**: Choose any name (e.g., "OneDrive Linux Client")
   - **Supported account types**: Select **"Accounts in any organizational directory and personal Microsoft accounts"** (this is required for personal OneDrive accounts)
   - **Redirect URI**:
     - Platform: **"Public client/native (mobile & desktop)"**
     - URI: `https://login.microsoftonline.com/common/oauth2/nativeclient`

4. Click **"Register"**

5. Copy the **"Application (client) ID"** from the Overview page

6. Edit `auth.h` and update:
   ```cpp
   const QString api_client_id = "YOUR_APPLICATION_ID_HERE";
   ```

**Note**: For public/desktop clients, you do NOT need a client secret. The app uses the native client redirect URI which doesn't require one.

## Building

```bash
qmake6 Beltrix.pro
make
```

## Running

```bash
./onedrive-linux-client
```

## Usage

1. Click **"Get Code"** - this opens your browser to Microsoft's login page
2. Sign in with your Microsoft account and authorize the app
3. After authorization, you'll be redirected to a URL containing a `code=` parameter
4. Copy the code value (everything after `code=` and before any `&`)
5. Paste the code into the app and click **"Sign In"**

### Navigation

- **Click** a folder to open it
- **Click** a file to open it in your browser
- **Right-click** any item for a context menu with options:
  - Open
  - Open in Browser
  - Download (files only)
  - Details
- Use the **Back** (`<`) button to go to the previous folder
- Use the **Home** button to return to the root

## Technical Details

- **API**: Microsoft Graph API v1.0
- **Authentication**: OAuth 2.0 with PKCE (public client flow)
- **OAuth Endpoint**: `https://login.microsoftonline.com/common/oauth2/v2.0`
- **Scopes**: `Files.Read`, `Files.ReadWrite`, `User.Read`, `offline_access`

## License

LGPL v3 - See [LICENSE](LICENSE) for details.

## Credits

Originally created by [Joscor Technical Research](https://joscor.com/) (2014).
Updated for Qt6 and Microsoft Graph API (2024).
