##Node.js UWP
This project is a Universal Windows Platform (UWP) application that wraps Node.js and enables deployment to Windows IoT Core devices from Visual Studio.
The application is packaged into the [NTVS IoT Extension (Beta)](https://github.com/ms-iot/ntvsiot) installer.

##To build:

Prerequisites:

    * Windows 10 (latest)
    * Python 2.6 or 2.7
    * Visual Studio 2015 (RC or later)
    * Windows 10 Tools (Bundled in Visual Studio 2015, or install separately)
	* Clone Node.js from https://github.com/microsoft/node

Steps:

    * Open cmd window
    * Set node_dir to the path of your Node.js clone
    * Set release_dir if desired (optional if copyrelease is not used)
    * Run "build.bat [x86|x64|arm] [copyrelease]"
