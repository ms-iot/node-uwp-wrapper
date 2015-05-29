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

    * Clone Node.js from https://github.com/microsoft/node
	* Install the NTVS IoT Extension using the steps [here](http://ms-iot.github.io/content/en-US/win10/samples/NodejsWU.htm) and create a new Node.js (Windows Universal) project
    * Copy &lt;Node.js clone path&gt;\tests to &lt;Node.js UWP project path (location of .njsproj file)&gt;\tests
    * Right click on the test you want to run and select "Set as Node.js Startup File". The file text will be made bold (see test-assert.js example below)
      ![Set test as Startup File]({{site.baseurl}}/images/test-startup-file.png)
    * Press F5 (or click on Debug->Start Debugging menu) to run the test
    * Console output can be redirected to file. You can view the logs on the device in c:\Users\DefaultAccount\AppData\Local\Packages\&lt;Your app ID (get it from VS build logs)&gt;\LocalState\nodeuwp.log
	
##Node.js compatibility with UWP
The following API's are not supported in Node.js UWP:

    * Child Processes
    * Cluster
    * Debugger
    * TTY

There are a few other limitations in other modules. For example, the [fs](https://nodejs.org/api/fs.html) module can only access files within its the UWP package or within paths declared in the [package capabilities](https://msdn.microsoft.com/en-us/library/windows/apps/hh464936.aspx).

Detailed documentation on which API's are supported in Node.js UWP can be found [here]({{site.baseurl}}/compatibility.xlsx).
