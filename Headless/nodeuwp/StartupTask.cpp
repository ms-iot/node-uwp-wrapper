/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "pch.h"
#include "StartupTask.h"
#include <ppltasks.h>
#include "v8.h"
#include "node.h"
#include "Logger.h"
#include "..\..\Common\Util.h"

using namespace nodeuwp;
using namespace std;
using namespace Windows::ApplicationModel::Background;
using namespace concurrency;
using namespace nodeuwputil;

// startupinfo.xml is used by Visual Studio to pass arguments to Node (see StartNode method).
// It's updated through Node.js UWP project properties and packaged in the project appx.
#define STARTUP_FILE L"startupinfo.xml"

// If --use-logger argument is passed to to Node, console.* methods will redirect
// output to a file (nodeuwp.log) in this applications local storage folder.
bool useLogger = false;

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
	StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;

	// Copy files to this applications local storage so that node can read/write to files
	// or folders relative to the location of the starup JavaScript file
	CopyFolderSync(appFolder, localFolder);

	BackgroundTaskDeferral^ deferral = taskInstance->GetDeferral();

	auto installationLocation = Windows::ApplicationModel::Package::Current->InstalledLocation;

	task<StorageFile^> getStartupInfo(installationLocation->GetFileAsync(STARTUP_FILE));

	getStartupInfo.then([=](StorageFile^ storageFile)
	{
		task<XmlDocument^> getStartupInfoXml(XmlDocument::LoadFromFileAsync(storageFile));

		getStartupInfoXml.then([=](XmlDocument^ startupInfoXml)
		{
			vector<shared_ptr<char>> argumentVector;

			shared_ptr<char> argChar = WCharToChar(L" ", 1);
			argumentVector.push_back(argChar);

			XmlNodeList^ argumentNodes = startupInfoXml->SelectNodes(L"StartupInfo/NodeOptions");
			PopulateArgsVector(argumentVector, argumentNodes, false, &useLogger);

			argumentNodes = startupInfoXml->SelectNodes(L"StartupInfo/Script");
			PopulateArgsVector(argumentVector, argumentNodes, true, &useLogger);

			argumentNodes = startupInfoXml->SelectNodes(L"StartupInfo/ScriptArgs");
			PopulateArgsVector(argumentVector, argumentNodes, false, &useLogger);

			int argc = argumentVector.size();

			shared_ptr<char*> argv;
			argv.reset(new char*[argc], [](char** ptr) { delete[] ptr; });
			for (unsigned int i = 0; i < argumentVector.size(); ++i)
			{
				argv.get()[i] = (argumentVector[i]).get();
			}

			if (!useLogger)
			{
				node::Start(argc, argv.get());
			}
			else
			{
				node::Start(argc, argv.get(), &Logger::GetInstance("nodeuwp.log"));
			}

			deferral->Complete();
		});
	});
}
