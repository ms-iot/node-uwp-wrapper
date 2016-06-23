##Node.js UWP Wrapper
This project is a Universal Windows Platform (UWP) application that wraps Node.js and enables deployment to Windows IoT Core devices from Visual Studio.
The application is packaged into the [Node.js Tools for UWP Apps](http://aka.ms/ntvsiotlatest) installer. These tools provide the ability
to create Node.js applications and easily deploy and debug them on Windows IoT Core devices. 
During deployment, the NTVS IoT extension will create a UWP package that contains:

* nodeuwp.dll: That's this UWP application.
* node.dll (code will be available soon):  This is node.exe, renamed to node.dll, and with the following differences:
  * It uses the Chakra JavaScript engine.
  * Code that not allowed in a UWP app container is disabled.
  * Code links to onecore.lib (instead of legacy DLL's like kernel32.dll, etc.) to enable it to run on Windows IoT Core.
* uwp.node (code [here](https://github.com/Microsoft/node-uwp)): This is the addon that allows you to access UWP namespaces from Node.js code.
* Your Node.js code and other files you choose to package.

To get started, take a look at the "Hello World" sample [here](http://ms-iot.github.io/content/en-US/win10/samples/NodejsWU.htm).

##To build
Coming soon.
  
##Testing
See comments in [Test\run.ps1](Test/run.ps1)

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). 
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) 
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.