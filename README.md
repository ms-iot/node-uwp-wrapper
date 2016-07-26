##Node.js UWP Wrapper

This project is a Universal Windows Platform (UWP) application that wraps Node.js and enables deployment to Windows 10 devices from Visual Studio.
The application is packaged into the [Node.js Tools for UWP Apps](http://aka.ms/ntvsiotlatest) installer. These tools provide the ability
to create Node.js applications and easily deploy and debug them on Windows IoT Core devices. 
During deployment, the NTVS IoT extension will create a UWP package that contains:

* nodeuwp.dll: That's this UWP background application (in [Headless](./Headless)).
* node.dll (code [here](https://aka.ms/node-uwp)):  This is node.exe, renamed to node.dll, and with the following differences:
  * It uses the Chakra JavaScript engine.
  * Code that not allowed in a UWP app container is disabled or replaced.
  * Code links to onecore.lib (instead of legacy DLL's like kernel32.dll, etc.) to enable it to run on Windows 10.
* uwp.node (code [here](https://github.com/Microsoft/node-uwp)): This is the addon that allows you to access UWP namespaces from Node.js code.
* Your Node.js code and other files you choose to package.

To get started, take a look at the "Hello World" sample [here](http://ms-iot.github.io/content/en-US/win10/samples/NodejsWU.htm).

##To build

####Prerequisites:
* Windows 10
* [Python 2.6 or 2.7](https://www.python.org)
* [Visual Studio 2015 with Windows 10 SDK](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx)

####Steps:
* Clone the Node.js (Chakra) branch [here](https://aka.ms/node-uwp). 
* Set an environment variable on your PC called NODE_DIR with the path of the clone (don't include trailing slash).
* Checkout the 'chakra-uwp' branch and build with:

```batch
vcbuild chakra uwp-dll nosign [x86|x64|arm]
```
* In Visual Studio, open the solution (nodeuwp.sln) for the app in the [Headless](./Headless) folder.
* Select your configuration and platform then build.
* You may also build the solution in the Developer Command Prompt for VS2015 (i.e. with msbuild):

```batch
msbuild Headless\nodeuwp.sln /p:configuration=[release|debug] /p:platform=[x86|x64|arm]
```

  
##Testing

Use the following instructions to test [node.dll](https://github.com/ms-iot/node) in the UWP wrapper. [Node.js tests](https://github.com/ms-iot/node/tree/chakra-uwp/test) 
can be run using the [Test\run.ps1](Test/run.ps1) PowerShell script. The script takes input from a nodeuwp.testinfo file (with a list of tests to run), launches the app
for each test, and logs results to a file. For more information, take a look at the comments at the beginning of the script.

* Clone this repository.
* Copy Test/nodeuwp.testinfo to your Documents folder (i.e. C:\Users\YourUserName\Documents) and update depending
  on what you want to run. Take a look at the comments in [Test\run.ps1](Test/run.ps1) to see how to do this.
* Build the Headed application in the Developer Command Prompt for VS2015 (Healess app is currently no supported for testing)

```batch
msbuild Headed\nodeuwpui.sln /p:configuration=[release|debug] /p:platform=[x86|x64|arm]
```
  If you get an error that the SDK used is different than what you have, open the solution and change the SDK to a version installed already.

* Open a PowerShell window and navigate into the Test folder
* Run the script with the command below:

```batch
.\run.ps1 -app [Path to appx] -test [path to test dir] -appl [appx launcher path]
```
  Example:

```batch
.\run.ps1 -app C:\repos\node-uwp-wrapper\Headed\AppPackages\nodeuwpui\nodeuwpui_1.0.0.0_x64_Test -test C:\repos\node\test -appl C:\Tools\AppLauncher.exe
```
* Results will be saved to Test\results.log. Below shows an example of what the output looks like:

```batch
Example output:
2016-06-18 15:17:18Z TestLog: Test count = [3]
2016-06-18 15:17:18Z TestLog: 1  - Start Test: test\parallel\test-assert.js
All OK
Exit Code: 0
2016-06-18 15:17:21Z TestLog: End Test
2016-06-18 15:17:22Z TestLog: 2  - Start Test: test\parallel\test-buffer.js
Error: Cannot find module 'C:\Users\username\AppData\Local\Packages\nodeuwpui_gqmz2j608xdxe\LocalState\test\parallel\test-buffer.js'
   at Module._resolveFilename (module.js:341:5)
   at Module._load (module.js:290:3)
   at Module.runMain (module.js:453:3)
   at startup (node.js:148:11)
   at Anonymous function (node.js:519:3)
2016-06-18 15:17:25Z TestLog: End Test
2016-06-18 15:17:25Z TestLog: **TEST SUMMARY**
2016-06-18 15:17:25Z TestLog: Tests run = [2/3]. Tests passed = [1/2]
```


This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) 
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.