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
* [Visual Studio 2015](https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx)


* Clone the Node.js (Chakra) branch [here](https://aka.ms/node-uwp). 
* Set an environment variable on your PC called NODE_DIR with the path of the clone (don't include trailing slash).
* Checkout the 'chakra-uwp' branch and build with:
  ```batch
  vcbuild chakra uwp-dll nosign [x86|x64|arm]
  ```
* In Visual Studio, open the solution (nodeuwp.sln) for the app in the [Headless](./Headless) folder.
* Select your configuration and platform then build.

  
##Testing

See comments in [Test\run.ps1](Test/run.ps1)

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) 
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.