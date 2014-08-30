onedrive-linux-client
=====================

Microsoft OneDrive (formerly SkyDrive) Linux Client built using Qt 5 (QtQuick)
This is a Qt Creator project and can be imported as such.

Screenshots
===========

![OneDrive Linux Client Login Screen](/screenshots/Screenshot from 2014-08-10 11:34:03.png?raw=true "Login Screen")

![OneDrive Linux Client Folders Screen](/screenshots/Screenshot from 2014-08-10 21:14:43.png?raw=true "Folders Screen")

Prerequisites
=============

1) Register a new application with Microsoft Developer Center. Head over 
to https://account.live.com/developers/applications/index and click "Create Application". 
Give the new application a name (it can be whatever you want).  

Go into "API Settings" and change "Mobile or desktop client app" to "Yes". Then set the 
"Redirect URL" to either a website you own or you can sent it to somewhere that won't 
redirect you to another page.  

2) Edit the auth.h header file and change the definitions for "api_client_id", 
"api_client_secret", and "api_redirect_url" to match what you used for your new app.  


Usage
=====

Build + Run the GUI app, click on "Get Code", then when you're redirected (after signing in) 
copy the code in the URL to the "Authorization Code" input field in the GUI.  

Notes
=====

I have absolutely no clue what the implications of building this and distributing it 
with your own static app settings so end-users don't have to (client secret, specifically). 
I'm not going to do this as I don't plan on making a business around this app, but if you 
do decide to do this, please research the legalities around it. 

