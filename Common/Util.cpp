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
		XmlNodeList^ argNodes, bool isStartupScript, bool* const &useLogger)
	{
		if (argNodes != nullptr)
		{
			IXmlNode^ textNode = argNodes->GetAt(0)->FirstChild;

			if (nullptr == textNode)
				return;

			std::wstring s(((Platform::String^)textNode->NodeValue)->Data());
			std::wstring delimiter = L" ";

			std::wstring token;
			std::shared_ptr<char> argChar;

			size_t offset = 0;
			size_t pos = s.find(delimiter);
			while (pos != string::npos)
			{
				token = s.substr(offset, pos - offset);

				argChar = WCharToChar(token.c_str(), token.size());
				if (0 == token.compare(L"--use-logger"))
				{
					*useLogger = true;
				}
				else
				{
					argVector.push_back(argChar);
				}

				offset = ++pos;
				pos = s.find(delimiter, pos);
				if (pos == string::npos)
				{
					token = s.substr(offset, pos - offset);
					argVector.push_back(argChar);
					break;
				}
			}

			if (isStartupScript)
			{
				std::wstring localFolder(ApplicationData::Current->LocalFolder->Path->Data());
				localFolder = localFolder.append(L"\\");
				s = localFolder.append(s);
			}

			argChar = WCharToChar(s.c_str(), s.size());
			argVector.push_back(argChar);
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