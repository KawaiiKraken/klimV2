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
- if needed ctrl+k to exit and adjust keybinds in the generated config file

# how to use
  soon™
  
# upcoming features
- better code
- if its proves useful, other ways to mess with packets

# known issues
- the mod seasonal monochromatic maestro can cause desync to be permanent 

# compiling from source
- overlay dll
  ```
  clang++.exe -o krekens_overlay.dll -D "_UNICODE" -D "UNICODE" -shared -Wall .\krekens_overlay.cpp
  ```
- klimV2
  ```
  clang++.exe -o .\klimV2.exe -D "_UNICODE" -D "UNICODE" -l .\WinDivert.lib -Wall .\klimV2.cpp
  ```
notes: 
- when compiling first clone the phnt module as well, you will have to manually change some <> to "" in it
- you may also have to include/link the windows sdk 

