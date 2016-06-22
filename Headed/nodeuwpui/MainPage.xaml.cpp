//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "v8.h"
#include "node.h"
#include "Logger.h"
#include <filesystem>
#include <ppltasks.h>
#include <sstream>
#include "..\..\Common\Util.h"

using namespace nodeuwpui;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Search;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace std;
using namespace std::tr2::sys;
using namespace Windows::Data::Xml::Dom;
using namespace concurrency;
using namespace node::logger;
using namespace nodeuwputil;

// startupinfo.xml is used by Visual Studio to pass arguments to Node (see StartNode method).
// It's updated through Node.js UWP project properties and packaged in the project appx.
#define STARTUP_FILE L"startupinfo.xml"

// If --use-logger argument is passed to to Node, console.* methods will redirect
// output to a file (nodeuwp.log) in this applications local storage folder.
bool useLogger = false;

void MainPage::Run()
{
	StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;

	// Copy files to this applications local storage so that node can read/write to files
	// or folders relative to the location of the starup JavaScript file
	CopyFolderSync(appFolder, localFolder);

	useLogger = true;
	auto docsLibrary = KnownFolders::DocumentsLibrary;

	// TODO: Headed app is only used for testing at the moment so this is hardcoded
	bool testMode = true;
	task<StorageFile^> getStartupInfo;
	if (testMode)
	{
		getStartupInfo = create_task(docsLibrary->GetFileAsync(STARTUP_FILE));
	}
	else
	{
		getStartupInfo = create_task(appFolder->GetFileAsync(STARTUP_FILE));
	}

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

			string scriptName(argumentVector.at(argumentVector.size() - 1).get());

			argumentNodes = startupInfoXml->SelectNodes(L"StartupInfo/ScriptArgs");
			PopulateArgsVector(argumentVector, argumentNodes, false, &useLogger);


			int argc = argumentVector.size();

			shared_ptr<char*> argv;
			argv.reset(new char*[argc], [](char** ptr) { delete[] ptr; });

			for (int i = 0; i < argumentVector.size(); ++i)
			{
				argv.get()[i] = (argumentVector[i]).get();
			}

			if (!useLogger)
			{
				node::Start(argc, argv.get());
			}
			else
			{	
				String^ logFileName = "nodeuwp.log";
				int ret = node::Start(argc, argv.get(), Logger::GetInstance(logFileName));
				string exitMsg = "Exit Code: " + std::to_string(ret);
				Logger::GetInstance(logFileName)->Log(ILogger::LogLevel::Info, exitMsg.c_str());
			}
			App::Current->Exit();
		},task_continuation_context::use_arbitrary());
	});
}


MainPage::MainPage()
{
	InitializeComponent();
	Run();
}
