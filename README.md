# klimV2
krekens destiny 2 netlimiter 

for support join our server at the [thrallway](https://thrallway.com)

# features
- ports: 3074DL, 3074UL, 27kDL, 27UL, 30kDL, 7500DL, whole game 800b/sDL
- game pauser
- customizable hotkeys
- undetected
- limits without a delay 

# screenshot of overlay
![image failed to load..](https://github.com/KawaiiKraken/klimV2/blob/master/example_screenshot.png "My Config")
 
# installation
- download [lastest release](https://github.com/KawaiiKraken/klimV2/releases/latest)
- run exe 
- if needed ctrl+k to exit and adjust the generated config file

# how to use
- [netlimiter bible](https://docs.google.com/document/d/1CuFbJ4KlbSMqf22lVap2yiSMHxLWRJpiMO1eIIpgtJQ)
- [vid example](https://www.youtube.com/watch?v=zTgaYyAxNZ4&pp=ygUPYXotMSBuZXRsaW1pdGVy) ...but using a different lim

# how it works/credits 
- packet manipulation: [WinDivert](https://github.com/basil00/Divert)
- game pauser/suspend: [NtSuspendProcess](https://github.com/diversenok/Suspending-Techniques#suspend-via-ntsuspendprocess)
- whole game limit: [NetQosPolicy](https://learn.microsoft.com/en-us/powershell/module/netqos/)
  
# upcoming 
- better code
- auto buffer
- other ways to mess with packets?
- better how to? maybe even a vid

# known issues
- the mod seasonal monochromatic maestro can cause desync to be permanent 
- whole game limit requires windows pro

# switch windows version to pro 
this could be potentially be considered piracy and i don't endorse it
- run cmd.exe as admin 
- paste `changepk /ProductKey VK7JG-NPHTM-C97JM-9MPGT-3V66T`
- reboot 
- run powershell.exe as admin
- paste `irm https://massgrave.dev/get | iex`
- chose hwid (unless youre using a spoofer)
- reboot

# how to contribute
- give this repo a star on github
- give feature ideas/requests at the [thrallway](https://thrallway.com)
- report bugs via a github issue or at the [thrallway](https://thrallway.com)
- clone this repo and make a pull request with your changes
- idk buy me an expansion or smth if you really like this project

# compiling from source
```
git clone https://github.com/KawaiiKraken/klimV2
cd klimV2
git submodule update --init --recursive
make release
```
notes: 
- you will have to change some <> to "" in phnt
- you may also have to include/link the windows sdk 

