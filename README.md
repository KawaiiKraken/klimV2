# klim v2
kreken's destiny 2 netlimiter 

give this repo a star on github if you like the project!

for support join our server at the [thrallway](https://thrallway.com)

<a href='https://ko-fi.com/kreky' target='_blank'><img height='46' width="200" src='https://cdn.ko-fi.com/cdn/kofi4.png' alt='Buy Coffee for kreky' />
</a>

# screenshots
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/hotkey_tab_v3.png "hotkey tab")
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/visuals_tab_v3.png "visual tab")
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/resources/overlay.png "overlay")
 
# how to use
- [netlimiter bible](https://docs.google.com/document/d/1CuFbJ4KlbSMqf22lVap2yiSMHxLWRJpiMO1eIIpgtJQ)
- [vid example](https://www.youtube.com/watch?v=zTgaYyAxNZ4&pp=ygUPYXotMSBuZXRsaW1pdGVy) ...vid is using a different lim

# known issues
- whole game limit requires windows pro
- the overlay can be seen when screensharing for some people (but only if discord is not focused?)
- passthrough mode does not log packets
- reinjection can be inconsistent when using multiple lims at the same time or for a long period of time

# how to contribute
- give feature ideas/requests at the [thrallway](https://thrallway.com)
- report bugs via a github issue or at the [thrallway](https://thrallway.com)
- clone this repo and make a pull request with your changes

# how it works/credits 
- packet manipulation: [WinDivert](https://github.com/basil00/Divert)
- game pauser/suspend: [NtSuspendProcess](https://github.com/diversenok/Suspending-Techniques#suspend-via-ntsuspendprocess)
- whole game limit: [NetQosPolicy](https://learn.microsoft.com/en-us/powershell/module/netqos/)

# compiling from source
- you will need to also install visual studio and the c++ desktop development pack
- if you dont have git also do `winget install git.git`
- all the commands are intended to be run NOT as admin
```
git clone https://github.com/KawaiiKraken/klimV2
cd klimv2
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe install jsoncpp opengl spdlog imgui[core,opengl3-binding,win32-binding]:x86-windows --triplet x86-windows
.\vcpkg\vcpkg.exe integrate install
.\klim.sln
```
hit build (release, x86)

