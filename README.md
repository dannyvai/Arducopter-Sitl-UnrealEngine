# Arducopter-Sitl-UnrealEngine
Create a binding between the arducopter sitl simulation and the unreal engine


Installation:
Unreal engine :
Download link (windows/Mac)
https://www.unrealengine.com/dashboard
Linux: (after profile change in the unreal page)
git clone https://github.com/EpicGames/UnrealEngine



Using python packages :
pymavlink - for the simulation
utm - for the conversion of geo to utm

Running the Unreal-Engine  :
~/UnrealEngine/Engine/Binaries/Linux/UE4Editor

Building the project source outside the Unreal engine to figure out what the problem is:
~/UnrealEngine/Engine/Binaries/DotNET$ mono UnrealBuildTool.exe FPSProject Linux Debug -project="\"/home/ohadanny/Documents/Unreal Projects/FPSProject/FPSProject.uproject\""


Running the arducopter sitl with the communication to the unreal-project:
./run_sim

Kill simulation:
./run_sim kill

In order for the simulation to run you need to overlook all the failsafes and such:
in the mavproxy screen run :
arm uncheck all

simulation:
ch1 - roll
ch2 - pitch
ch3 - throttle
ch4 - yaw
