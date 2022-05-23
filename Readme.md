# DTrack Plugin for Unreal Engine 5

This is a plug-in for the Unreal Engine with the purpose of native integration of the [Advanded Realtime Tracking][1] _DTrack_ and _DTrack2_ tracking solutions. It injects data into the engine through LiveLink. Data can be accessed through Blueprint or C++. The plugin currently supports the DTrack body`6d`and flystick`6df2`data format.


## Prerequisites

- Unreal Engine 5.0.1. It also works for Unreal4 from Unreal 4.23 and later
- Windows
- Microsoft Visual Studio 2022 for UE5 and Visual Studio 2017 and 2019 for UE4 (Express or Community Edition should suffice)


## Installation

### Preparation

- Adapt the _.uplugin_ file to the version of your _UnrealEditor_:<br>The `EngineVersion` key in the file _DTrackPlugin.uplugin_ comes with a default value corresponding to the latest tested _UE4Editor_ version (e.g. `"EngineVersion": "5.0.1"` or `"EngineVersion": "4.25.0"`).<br>If you are using an _UE4Editor_ version with a different minor version number (e.g. 4.23 instead of 4.25), you should adjust this value (e.g. to `"EngineVersion": "4.23.0"`).

### Install into the global Engine plugin folder
- Compile the plugin manually:<br> *&lt;UE5Dir&gt;\Engine\Build\BatchFiles\RunUAT.bat* BuildPlugin -Plugin=*/Path/to/DTrackPlugin.uplugin* -TargetPlatforms=Win64 -Package=*&lt;OutDir&gt;* -Rocket
- Copy *&lt;OutDir&gt;* to *&lt;UE5Dir&gt;\Engine\Plugins\DTrackPlugin*



### Alternatively, install into your local project plugin folder
- Open the UE5Editor and create an Unreal C++ project
- Copy the plugin to *&lt;project&gt;\Plugins\DTrackPlugin*
- Compilation then takes place automatically when starting your Unreal project



## DTrack Configuration

### Room Calibration

For general information about the DTrack room calibration and room adjustment see the DTrack User Manual.
Here we discuss details relevant for use with the Unreal Engine.

The calibration angle which comes with your ART tracking system defines the coordinate system layout in your tracking area.
It consists of four retroreflective or active markers mounted onto a L-shaped frame.

![DTrack calibration angle](Doc/images/calibration-angle.PNG)

The marker in the edge of this L-shape by default designates the origin of the DTrack coordinate system.
When using the _Normal_ calibration mode, the long arm of this L-shape corresponds to the X axis, the short arm to the Y axis.
DTrack coordinates refer to a right-handed coordinate system, so when the angle is placed flat on the ground with the markers on top the Z axis points upwards.

You can change orientation and position of the DTrack coordinate system with respect to the calibration angle via _Tracking > Room adjustment_ in the DTrack UI.


The plugin transforms a right-handed position of a DTrack 6DoF measurement to a left-handed Unreal position  by inverting the Y axis:
***( X<sub>Unreal</sub> , Y<sub>Unreal</sub> , Z<sub>Unreal</sub> ) = ( X<sub>DTrack</sub> , -Y<sub>DTrack</sub> , Z<sub>DTrack</sub> )*** .
<br><br>

![DTrack room calibration dialog](Doc/images/dtrack-roomcal.PNG)
![DTrack and Unreal coordinates systems](Doc/images/coords-dtrack+unreal.png)



<br>


### DTrack Output Configuration

Via _Tracking > Output_ in the DTrack UI you can set up IP and port of the host of your Unreal Editor or application.
In the corresponding dialog, you can also enable the DTrack output types `6d` and `6df2`.

<br>

![DTrack output dialog](Doc/images/dtrack-output.PNG)
[1]
<br>


## Plugin Usage

See [UnrealDTrackSample](https://www.github.com/ar-tracking/UnrealDTrackSample) for a detailed example.

The mapping of Flystick buttons and joystick is listed in *DTrackFlystickInputDevice.cpp* within the *DTrackPlugin\Source\DTrackInput\Private* directory.



[1]: https://ar-tracking.com/
