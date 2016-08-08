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
#include "Util.h"
#include <Windows.h>
#include <filesystem>
#include <sstream>

using namespace Platform;
using namespace std::tr2::sys;

namespace nodeuwputil
{
	shared_ptr<char> WCharToChar(const wchar_t* str, int strSize)
	{
		// Calculate the needed buffer size
		DWORD bufferSize = WideCharToMultiByte(CP_UTF8,
			0,
			str,
			strSize,
			nullptr,
			0,
			nullptr,
			nullptr);

		if (bufferSize == 0)
		{
			throw ref new ::Platform::Exception(GetLastError(), L"nodeuwputil::WCharToChar failed");
		}

		shared_ptr<char> buffer(new char[bufferSize + 1], [](char* ptr) { delete[] ptr; });
		buffer.get()[bufferSize] = '\0';
		// Do the actual conversion
		WideCharToMultiByte(CP_UTF8,
			0,
			str,
			strSize,
			buffer.get(),
			bufferSize,
			nullptr,
			nullptr);

		return buffer;
	}

	shared_ptr<wchar_t> CharToWChar(const char* str, int strSize)
	{
		// Calculate the needed buffer size
		DWORD bufferSize = MultiByteToWideChar(CP_UTF8,
			0,
			str,
			strSize,
			nullptr,
			0);

		if (bufferSize == 0)
		{
			throw ref new ::Platform::Exception(GetLastError(), L"nodeuwputil::CharToWChar failed");
		}

		shared_ptr<wchar_t> buffer(new wchar_t[bufferSize + 1], [](wchar_t* ptr) { delete[] ptr; });
		buffer.get()[bufferSize] = '\0';
		// Do the actual conversion
		MultiByteToWideChar(CP_UTF8,
			0,
			str,
			strSize,
			buffer.get(),
			bufferSize);

		return buffer;
	}

	void PopulateArgsVector(vector<shared_ptr<char>> &argVector,
		String^ startStr, bool &useLogger)
	{
		if (startStr != nullptr)
		{
			wstring localFolder(ApplicationData::Current->LocalFolder->Path->Data());
			localFolder.append(L"\\");
			wstring args(startStr->Data());
			args.erase(0, args.find_first_not_of(L" \n\r\t")); // Trim leading whitespace
			wstring arg;
			shared_ptr<char> in;
			bool insert = false;
			bool scriptFound = false;		

			for (size_t i = 0; i < args.length(); i++) {

				wchar_t c = args[i];
				if (c == ' ') {
					insert = true;
				}
				else if (c == '\"') 
				{
					i++;
					while (args[i] != '\"')
					{ 
						arg += args[i];
						i++; 
					}
					i++;
					insert = true;
				}
				else {
					arg += args[i];
				}

				// We insert an argument into the vector if we see a space or we've reached the 
				// end of the string.
				if ((insert == true || args.size() - 1 == i) && arg.size() != 0)
				{
					insert = false;

					// 'node' will usually be the first argument if the package.json file is also
					// used with the console version of node. We'll ignore it if it exists.
					if (_wcsicmp(arg.c_str(), L"node") == 0 && argVector.size() == 1)
					{
						arg.clear();
						continue;
					}

					if (arg.compare(L"--use-logger") == 0)
					{
						useLogger = true;
						arg.clear();
						continue;
					}

					// The first '*.js' argument will be considered as the script to start
					if (arg.find(L".js") != string::npos && scriptFound == false)
					{
						scriptFound = true;
						arg = localFolder + arg;
					}

					in = WCharToChar(arg.c_str(), arg.size());
					argVector.push_back(in);
					arg.clear();
				}
			}
		}
	}

	void CopyFolderSync(StorageFolder^ source, StorageFolder^ destination)
	{
		path from(source->Path->Data());
		path to(destination->Path->Data());
		copy_options opts = copy_options::recursive | copy_options::update_existing;
		copy(from, to, opts);
	}
}