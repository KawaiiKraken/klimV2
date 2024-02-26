# klim v2
kreken's destiny 2 netlimiter 

for support join our server at the [thrallway](https://thrallway.com)

# features
- lims: 3074DL, 3074UL, 27kDL, 27UL, 30kDL, 7500DL, whole game 800b/s DL
- game pauser
- config gui 
- undetected
- limits without a delay 
- overlay 
- timers 
- packet logs
- hotkeys only work when d2 is focused

# screenshots
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/hotkey_tab.png "hotkey tab")
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/visual_tab.png "visual tab")
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/overlay.png "overlay")
 
# installation
- download [lastest release](https://github.com/KawaiiKraken/klimV2/releases/latest)
- unzip
- run exe

# how to use
- [netlimiter bible](https://docs.google.com/document/d/1CuFbJ4KlbSMqf22lVap2yiSMHxLWRJpiMO1eIIpgtJQ)
- [vid example](https://www.youtube.com/watch?v=zTgaYyAxNZ4&pp=ygUPYXotMSBuZXRsaW1pdGVy) ...vid is using a different lim

# how it works/credits 
- packet manipulation: [WinDivert](https://github.com/basil00/Divert)
- game pauser/suspend: [NtSuspendProcess](https://github.com/diversenok/Suspending-Techniques#suspend-via-ntsuspendprocess)
- whole game limit: [NetQosPolicy](https://learn.microsoft.com/en-us/powershell/module/netqos/)
  
# known issues
- whole game limit requires windows pro
- the overlay can be seen when screensharing for some people 

# upcoming features
- turn off hotkeys when d2 chatbox is open
- hide overlay when d2 isnt focused
- make overlay relative to d2 window (so that it works in windowed mode)
- custom overlay position
- changing binds without restarting klim
- better lims?

# how to contribute
- give this repo a star on github
- give feature ideas/requests at the [thrallway](https://thrallway.com)
- report bugs via a github issue or at the [thrallway](https://thrallway.com)
- clone this repo and make a pull request with your changes
- idk buy me an expansion or smth if you really like this project
- kofi soonTM

# compiling from source
```
git clone https://github.com/KawaiiKraken/klimV2
cd klimv2
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe install jsoncpp opengl spdlog imgui[core,opengl3-binding,win32-binding]:x64-windows 
.\vpkg\vcpkg.exe integrate install
.\klim.sln
```
hit build

