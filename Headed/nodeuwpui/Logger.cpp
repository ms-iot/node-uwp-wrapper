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
#include "Logger.h"
#include <windows.storage.h>
#include <ppltasks.h>

using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Platform;
using namespace concurrency;

namespace nodeuwpui 
{
	std::unique_ptr<Logger> Logger::s_pInstance;

	const Logger& Logger::GetInstance(String^ logFileName)
	{
		if (s_pInstance == nullptr)
		{
			if (s_pInstance == nullptr)
			{
				s_pInstance.reset(new (std::nothrow) Logger(logFileName));
			}
		}
		return *s_pInstance.get();
	}

	Logger::Logger(String^ logFileName)
	{
		// Create a new file in the local folder if it doesn't exist
		StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;	
		auto createFileTask = create_task(localFolder->CreateFileAsync(logFileName, Windows::Storage::CreationCollisionOption::OpenIfExists));
		createFileTask.then([this](StorageFile^ file) {
			m_file = file;
		}).wait(); // wait so if Log is called directly after it won't fail
	}

	void Logger::Log(ILogger::LogLevel logLevel, const char* str) const
	{
		// Convert char* to Platform::String
		size_t newsize = strlen(str) + 1;
		wchar_t * wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, str, _TRUNCATE);
		String^ pstr = ref new String(wcstring);

		// Append console logs to file
		IAsyncAction^ Action = FileIO::AppendTextAsync(m_file, pstr + "\r\n");
		create_task(Action).then([this](){}).wait(); // wait is required to log messages sequentially.
	}
}