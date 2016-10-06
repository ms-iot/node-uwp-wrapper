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
#include "unzip.h"
#include <direct.h>

#define WRITEBUFFERSIZE 8192
#define NODE_MODULE_MAX_PATH 1024 

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
				if (c == ' ') 
				{
					insert = true;
				}
				else if (c == '\"') 
				{
					i++;
					while (args[i] != '\"' && i < args.size())
					{ 
						arg += args[i];
						i++;
					}
					i++;
					insert = true;
				}
				else 
				{
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

					// Update the script path.
					// The script to start will be the first argument that doesn't start with '-'.
					// Node usage: node [options] [v8 options] [script.js | -e "script"] [arguments]
					// 'options' and 'v8 options' start with '-'
					if (arg[0] != '-' && scriptFound == false)
					{
						scriptFound = true;
						arg = localFolder + arg;
					}

					if (arg.compare(L"--use-logger") == 0)
					{
						useLogger = true;
						arg.clear();
						continue;
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

    // Based on https://github.com/madler/zlib/blob/master/contrib/minizip/miniunz.c#L138
	void MakePath(const char *newdir)
	{
		char *buffer;
		char *p;
		int  len = (int)strlen(newdir);
		int err;

		if (len <= 0)
		{
			return;
		}

		buffer = (char*)malloc(len + 1);
		if (buffer == NULL)
		{
			throw ref new ::Platform::Exception(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY), 
				L"nodeuwputil::MakePath Error allocating memory");
		}
		strcpy_s(buffer, (len + 1), newdir);

		if (buffer[len - 1] == '/') {
			buffer[len - 1] = '\0';
		}
		
		err = _mkdir(buffer);
		if (err == 0)
		{
			return;
		}

		p = buffer + 1;
		while (1)
		{
			char hold;

			while (*p && *p != '\\' && *p != '/')
				p++;
			hold = *p;
			*p = 0;
			err = _mkdir(buffer);
			if (err == -1 && (errno == ENOENT))
			{
				throw ref new ::Platform::Exception(HRESULT_FROM_WIN32(errno), L"nodeuwputil::MakePath _mkdir failed. Directory: " +
					ref new String(CharToWChar(buffer, (len + 1)).get()));
			}
			if (hold == 0)
				break;
			*p++ = hold;
		}
		free(buffer);
	}

	// Based on https://github.com/ms-iot/node/blob/chakra-uwp/deps/zlib/contrib/minizip/miniunz.c#L312
	void DoExtractCurrentfile(unzFile uf, char* dest)
	{
		char filename_inzip[NODE_MODULE_MAX_PATH];
		char* filename_withoutpath;
		char* p;
		FILE *fout = NULL;
		void* buf;
		uInt size_buf;
		unz_file_info64 file_info;
		uLong ratio = 0;

		int err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile unzGetCurrentFileInfo64 failed. Filename: " + 
				ref new String(CharToWChar(filename_inzip, sizeof(filename_inzip)).get()));
		}

		// Prepend the destination to the path (filename_inzip)
		char temp[NODE_MODULE_MAX_PATH];
		strcpy_s(temp, NODE_MODULE_MAX_PATH, filename_inzip);
		strcpy_s(filename_inzip, NODE_MODULE_MAX_PATH, dest);
		strcat_s(filename_inzip, NODE_MODULE_MAX_PATH, "\\");
		strcat_s(filename_inzip, NODE_MODULE_MAX_PATH, temp);

		size_buf = WRITEBUFFERSIZE;
		buf = (void*)malloc(size_buf);
		if (buf == NULL)
		{
			throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile Error allocating memory");
		}

		p = filename_withoutpath = filename_inzip;
		while ((*p) != '\0')
		{
			if (((*p) == '/') || ((*p) == '\\'))
			{
				filename_withoutpath = p + 1;
			}
			p++;
		}

		if ((*filename_withoutpath) == '\0')
		{
			err = _mkdir(filename_inzip);
			if (0 != err)
			{
				throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile _mkdir failed. Directory: " +
					ref new String(CharToWChar(filename_inzip, sizeof(filename_inzip)).get()));
			}
		}
		else
		{
			const char* write_filename = filename_inzip;

			if (err == UNZ_OK)
			{
				fopen_s(&fout, write_filename, "wb");
				/* some zipfile don't contain directory alone before file */
				if ((fout == NULL) && (filename_withoutpath != (char*)filename_inzip))
				{
					char c = *(filename_withoutpath - 1);
					*(filename_withoutpath - 1) = '\0';
					MakePath(write_filename);
					*(filename_withoutpath - 1) = c;
					fopen_s(&fout, write_filename, "wb");
				}

				if (fout == NULL)
				{
					throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile fopen_s failed. Filename: " +
						ref new String(CharToWChar(write_filename, sizeof(filename_inzip)).get()));
				}
			}

			// Needs to be called even when zip has no password
			err = unzOpenCurrentFilePassword(uf, 0);
			if (err != UNZ_OK)
			{
				throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile unzOpenCurrentFilePassword failed");
			}

			if (fout != NULL)
			{
				do
				{
					err = unzReadCurrentFile(uf, buf, size_buf);
					if (err < 0)
					{
						throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtractCurrentfile unzReadCurrentFile failed. Filename: " +
							ref new String(CharToWChar(write_filename, sizeof(filename_inzip)).get()));
					}
					if (err > 0)
					{
						// TODO: See if there would be improvement by only overwritting if timestamp has changed
						if (fwrite(buf, err, 1, fout) != 1)
						{
							throw ref new ::Platform::Exception(errno, L"nodeuwputil::DoExtractCurrentfile fwrite failed. Filename: " +
								ref new String(CharToWChar(write_filename, sizeof(filename_inzip)).get()));
						}
					}
				} while (err > 0);

				fclose(fout);
			}

			unzCloseCurrentFile(uf);
		}

		free(buf);
	}

	// Based on https://github.com/ms-iot/node/blob/chakra-uwp/deps/zlib/contrib/minizip/miniunz.c#L475
	void DoExtract(unzFile uf, char* dest)
	{
		unz_global_info64 gi;

		int err = unzGetGlobalInfo64(uf, &gi);
		if (err != UNZ_OK)
		{
			throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtract unzGetGlobalInfo failed");
		}

		for (uLong i = 0; i < gi.number_entry; i++)
		{
			DoExtractCurrentfile(uf, dest);

			if ((i + 1) < gi.number_entry)
			{
				err = unzGoToNextFile(uf);
				if (err != UNZ_OK)
				{
					throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtract unzGoToNextFile failed");
				}
			}
		}
	}


	void Extract(StorageFile^ zipFile, String^ destination)
	{
		shared_ptr<char> zipFileCharStr = WCharToChar(zipFile->Path->Data(), zipFile->Path->Length());
		unzFile uf = unzOpen64(zipFileCharStr.get());

		if (uf)
		{
			DoExtract(uf, WCharToChar(destination->Data(), destination->Length()).get());

			unzClose(uf);
		}
		else
		{
			throw ref new ::Platform::Exception(ERROR_INVALID_PARAMETER, L"nodeuwputil::Extract Invalid zip file");
		}
	}
}
