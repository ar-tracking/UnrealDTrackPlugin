# DTrack Plugin for Unreal Engine 4

This is a plug-in for the Unreal Engine 4.23 or later with the purpose of native integration of the Advanded Realtime Tracking DTrack and DTrack2 tracking solutions. It injects data into the engine through LiveLink. Data can be accessed through Blueprint or C++. The plugin currently supports the DTrack body`6d`and flystick`6df2`data format.


## Prerequisites

- Unreal Engine 4.23 or later
- Windows 64 bit
- Microsoft Visual Studio 2015 or later (Express or Community Edition should suffice)
- ART [DTrackSDK](http://github.com/ar-tracking/DTrackSDK)


## Installation

### Prepare the DTrackSDK library
- Download the [DTrackSDK](http://github.com/ar-tracking/DTrackSDK) source
- Build a static 64bit library *DTrackSDK.lib*. <br>Make sure to use the same Visual Studio version as you use with UE4Editor, and compile for the x64 platform.<br>You can find a general guide on how to configure VS for static libraries [here](https://docs.microsoft.com/de-de/cpp/build/walkthrough-creating-and-using-a-static-library-cpp?view=vs-2017).



### Install into the global Engine plugin folder
- Copy *DTrackSDK.lib* to the plugin source directory *DTrackPlugin\ThirdParty\DTrackSDK\Lib*
- Copy the DTrackSDK *include* directory to the plugin source directory *DTrackPlugin\ThirdParty\DTrackSDK\Include*
- Compile the plugin manually:<br> *&lt;UE4Dir&gt;\Engine\Build\BatchFiles\RunUAT.bat* BuildPlugin -Plugin=*DTrackPlugin.uplugin* - TargetPlatforms=Win64 -Package=*&lt;OutDir&gt;* -Rocket
- In case you later want to package your projects with DTrack for shipping: Copy *DTrackSDK.lib* to *&lt;OutDir&gt;\Thirdparty\DTrackSDK\Lib*
- Copy *&lt;OutDir&gt;* to *&lt;UE4Dir&gt;\Engine\Plugins*



### Alternatively, install into your local project plugin folder
- Open the UE4Editor and create an Unreal C++ project
- Copy the plugin to *&lt;project&gt;\Plugins\DTrackPlugin*
- Copy *DTrackSDK.lib* to *&lt;project&gt;\Plugins\DTrackPlugin\ThirdParty\DTrackSDK\Lib*
- Compilation then takes place automatically when starting your Unreal project

To edit the plugin source yourself, install the plugin into the local project folder and update the Visual Studio project files.


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

<br>


## Plugin Usage

See [UnrealDTrackSample](https://www.github.com/ar-tracking/UnrealDTrackSample) for a detailed example.

The mapping of Flystick buttons and joystick is listed in *DTrackFlystickInputDevice.cpp* within the *DTrackPlugin\Source\DTrackInput\Private* directory.


