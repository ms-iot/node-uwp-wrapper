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
	
##To test:
Follow the steps below to run [tests](https://github.com/joyent/node/tree/master/test) included with Node.js.

* Clone Node.js from [https://github.com/microsoft/node](https://github.com/microsoft/node)
* Install the NTVS IoT Extension using the steps [here](http://ms-iot.github.io/content/en-US/win10/samples/NodejsWU.htm) and create a new Node.js (Windows Universal) project
* Copy <Node.js clone path>\tests to <Node.js UWP project path (location of .njsproj file)>\tests
* Right click on the test you want to run and select "Set as Node.js Startup File". The file text will be made bold (see test-assert.js example below)

  ![Set test as Startup File](./images/test-startup-file.png)

* Press F5 (or click on Debug->Start Debugging menu) to run the test
* Console output can be redirected to file. You can view the logs on the device in C:\Users\DefaultAccount\AppData\Local\Packages\&lt;Your app ID (get it from VS build logs)&gt;\LocalState\nodeuwp.log
	
##Node.js API compatibility with UWP

API | Supported
--- | ---
Assert | Yes
Buffer | Yes
Child Processes | **No**
Cluster | **No**
Console | **No** (output can optionally be redirected to file)
Crypto | Yes
Debugger | **No**
DNS | Yes
Domain | Yes
Events | Yes
File System | Yes
Globals | Yes
HTTP | Yes
HTTPS | Yes
Modules | Yes
Net | Yes
OS | Yes
Path | Yes
Process | Yes
Punycode | Yes
Query Strings | Yes
Readline | Yes
REPL | Yes
Smalloc | Yes
Stream | Yes
String Decoder | Yes
Timers | Yes
TLS/SSL | Yes
TTY | **No**
UDP/Datagram | Yes
URL | Yes
Utilities | Yes
VM | Yes
ZLIB | Yes

**Note:** There may be some limitations in supported modules. For example, the [fs](https://nodejs.org/api/fs.html) module can only access files within its the UWP package or within paths declared in its [package capabilities](https://msdn.microsoft.com/en-us/library/windows/apps/hh464936.aspx).

Detailed documentation on which API's are supported in Node.js UWP can be found [here](./compatibility.xlsx).
