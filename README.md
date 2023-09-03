# klimV2
an oss destiny 2 netlimiter that doesn't rely on NL4/5 DLL/service

# features
- 3074DL/3074UL/27k/30k/7500
- customizable hotkeys
- undetected
- stream proof overlay 
- limits without a delay 
 
# installation
- download https://github.com/KawaiiKraken/klimV2/releases/latest
- run exe 

# how to use
  soon™
  
# upcoming features
- gui and/or config file for the hotkeys
- better code
- if its proves useful, other ways to mess with packets

# known issues

# compiling from source
- overlay
  ```
  clang++.exe -o krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall .\krekens_overlay.cpp
  ```
- klimV2
  ```
  clang++.exe -o .\klimV2.exe -D "_UNICODE" -D "UNICODE" -l .\WinDivert.lib -Wall .\klimV2.cpp
  ```
note: you may also have to include and/or link the windows 10 sdk 

