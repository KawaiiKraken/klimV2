# klimV2
an oss destiny 2 netlimiter that doesnt rely on NL4/5 DLL/service

# features
- upload/download 3074/27k/30k/7500
- hotkeys (soon customizable)
- undetected
- stream proof overlay 
- instant rule application
 
# installation
- download https://github.com/KawaiiKraken/klimV2/releases/latest
- run exe as admin

# how to use
  soon™

# troubleshooting
- run as admin
- disable anti virus 
- if this didnt help head over to our discord #support: https://discord.com/invite/KGyjysA5WY
  
# upcoming features
- gui and/or config file for the hotkeys
- better code
- if its proves useful, other ways to mess with packets ()

# known issues
- hotkeys can't be triggered if other keys are pressed

# compiling from source
- overlay
  ```
  clang++.exe -o krekens_overlay.dll -l "kernel32" -l "user32" -D "_UNICODE" -D "UNICODE" -lgdi32 -shared -Wall .\krekens_overlay.cpp
  ```
- klimV2
  ```
  clang++.exe -o .\klimV2.exe -l "kernel32" -l "user32" -D "_UNICODE" -D "UNICODE" -l .\WinDivert.lib -I -Wall .\klimV2.cpp
  ```
note: you may also have to include and/or link the windows 10 sdk 

