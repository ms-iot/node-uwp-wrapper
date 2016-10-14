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
#define MD5HASHSIZE 16

using namespace Platform;
using namespace std::tr2::sys;
using namespace concurrency;

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
		copy_options opts = copy_options::recursive | copy_options::overwrite_existing;
		copy(from, to, opts);
	}

	int MakePath(String^ newDir)
	{
		wstring newdir(newDir->Data());
		vector<String^> subdirs;
		wchar_t *nextTok = NULL;
		StorageFolder^ storagefolder;

		wchar_t* tok = wcstok_s((wchar_t*)newDir->Data(), L"\\", &nextTok);

		while (tok != NULL)
		{
			subdirs.push_back(ref new String(tok));
			tok = wcstok_s(NULL, L"\\", &nextTok);
		}

		// Walk the path until we get a valid StorageFolder object.
		// This function will fail if path provided isn't abosolute.
		// In future if needed this can be modified.
		newDir = subdirs.at(0);
		subdirs.erase(subdirs.begin());
		vector<String^>::iterator it = subdirs.begin();
		
		for (; it != subdirs.end(); ++it)
		{		
			newDir = newDir + "\\" + *it;

			try
			{
				storagefolder = create_task(StorageFolder::GetFolderFromPathAsync(newDir)).get();
				break;
			}
			catch (Exception^ e)
			{
				if (e->HResult != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
				{
					throw;
				}
				else
				{
					continue;
				}
			}
			
		}

		if (storagefolder == nullptr)
		{
			return UNZ_INTERNALERROR;
		}

		// Create the folders that don't exist
		advance(it, 1);
		for (; it != subdirs.end(); ++it)
		{
			String^ temp = *it;
			try
			{
				storagefolder = create_task(storagefolder->CreateFolderAsync(*it)).get();
			}
			catch (Exception^ e)
			{
				if (e->HResult != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
				{
					throw;
				}
				else
				{
					storagefolder = create_task(storagefolder->GetFolderAsync(*it)).get();
				}
			}
		}
	}

	// Based on https://github.com/ms-iot/node/blob/chakra-uwp/deps/zlib/contrib/minizip/miniunz.c#L312
	int DoExtractCurrentfile(unzFile uf, String^ dest)
	{
		char filename_inzip[NODE_MODULE_MAX_PATH];
		char* filename_withoutpath;
		char* p;
		FILE *fout = NULL;
		unz_file_info64 file_info;
		uLong ratio = 0;

		int err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			return err;
		}

		// Prepend the destination to the path (filename_inzip)
		char temp[NODE_MODULE_MAX_PATH];
		strcpy_s(temp, NODE_MODULE_MAX_PATH, filename_inzip);
		strcpy_s(filename_inzip, NODE_MODULE_MAX_PATH, WCharToChar(dest->Data(), dest->Length()).get());
		strcat_s(filename_inzip, NODE_MODULE_MAX_PATH, "\\");
		strcat_s(filename_inzip, NODE_MODULE_MAX_PATH, temp);

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
				return err;
			}
		}
		else
		{
			const char* write_filename = filename_inzip;

			// Needs to be called even when zip has no password
			err = unzOpenCurrentFilePassword(uf, 0);
			if (err == UNZ_OK)
			{
				fopen_s(&fout, write_filename, "wb");
				/* some zipfile don't contain directory alone before file */
				if ((fout == NULL) && (filename_withoutpath != (char*)filename_inzip))
				{
					char c = *(filename_withoutpath - 1);
					*(filename_withoutpath - 1) = '\0';
					MakePath(ref new String(CharToWChar(write_filename, sizeof(filename_inzip)).get()));
					*(filename_withoutpath - 1) = c;
					fopen_s(&fout, write_filename, "wb");
				}
			}

			if (fout != NULL)
			{
				shared_ptr<byte> buf(new byte[WRITEBUFFERSIZE], [](byte* ptr) { delete[] ptr; });

				do
				{
					err = unzReadCurrentFile(uf, buf.get(), WRITEBUFFERSIZE);
					if (err < 0)
					{
						break;
					}
					if (err > 0)
					{
						if (fwrite(buf.get(), err, 1, fout) != 1)
						{
							err = UNZ_ERRNO;
							break;
						}
					}
				} 
				while (err > 0);

				if (fout)
				{
					fclose(fout);
				}

			}

			if (err == UNZ_OK)
			{
				err = unzCloseCurrentFile(uf);
			}
			else 
			{
				unzCloseCurrentFile(uf); /* don't lose the error */
			}
		}

		return err;
	}

	// Based on https://github.com/ms-iot/node/blob/chakra-uwp/deps/zlib/contrib/minizip/miniunz.c#L475
	int DoExtract(unzFile uf, String^ dest)
	{
		unz_global_info64 gi;

		int err = unzGetGlobalInfo64(uf, &gi);
		if (err != UNZ_OK)
		{
			throw ref new ::Platform::Exception(err, L"nodeuwputil::DoExtract unzGetGlobalInfo failed");
		}

		for (uLong i = 0; i < gi.number_entry; i++)
		{
			err = DoExtractCurrentfile(uf, dest);
			if (err != UNZ_OK)
			{
				return err;
			}

			if ((i + 1) < gi.number_entry)
			{
				err = unzGoToNextFile(uf);
				if (err != UNZ_OK)
				{
					return err;
				}
			}
		}

		return err;
	}

	int CompareHashFiles(char* file1, int file1Size, char* file2, int file2Size, bool& isEqual)
	{
		FILE* f1 = NULL;
		FILE* f2 = NULL;
		int err;
		isEqual = false;

		err = fopen_s(&f1, file1, "r");
		if (err != 0)
		{
			return err;
		}

		err = fopen_s(&f2, file2, "r");
		if (err != 0)
		{
			if (f1)
			{
				fclose(f1);
			}
			return err;
		}

		char b1[MD5HASHSIZE];
		char b2[MD5HASHSIZE];

		int bytesRead = fread(b1, 1, MD5HASHSIZE, f1);
		if (bytesRead != MD5HASHSIZE)
		{
			err = UNZ_INTERNALERROR;
		}
		else
		{
			bytesRead = fread(b2, 1, MD5HASHSIZE, f2);
			if (bytesRead != MD5HASHSIZE)
			{
				err = UNZ_INTERNALERROR;
			}
			else
			{
				if (0 == memcmp(b1, b2, MD5HASHSIZE))
				{
					isEqual = true;
				}			
			}
		}

		if (f1)
		{
			fclose(f1);
		}
		if (f2)
		{
			fclose(f2);
		}
		return err;
	}

	bool ModuleUpdateRequired()
	{
		StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;

		String^ currHashFile;
		bool hashesAreEqual;
		int err;

		try
		{
			currHashFile = create_task(localFolder->GetFileAsync("node_modules.hash")).get()->Path;
		}
		catch (Exception^ e) 
		{
			if (e->HResult != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			{
				throw;
			}
			else
			{
				return true;
			}
		}

		String^ newHashFile = create_task(appFolder->GetFileAsync("node_modules.hash")).get()->Path;

		err = CompareHashFiles(WCharToChar(newHashFile->Data(), newHashFile->Length()).get(), newHashFile->Length(),
			WCharToChar(currHashFile->Data(), currHashFile->Length()).get(), currHashFile->Length(), hashesAreEqual);

		if (err != UNZ_OK)
		{
			return false;
		}
		else
		{
			if (hashesAreEqual)
			{
				// No update required if the hashes match
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	void Extract(StorageFile^ zipFile, String^ destination)
	{
		shared_ptr<char> zipFileCharStr = WCharToChar(zipFile->Path->Data(), zipFile->Path->Length());
		unzFile uf = unzOpen64(zipFileCharStr.get());
		int err;

		if (uf)
		{
			err = DoExtract(uf, destination);

			unzClose(uf);

			if (err != UNZ_OK)
			{
				throw ref new ::Platform::Exception(err, L"nodeuwputil::Extract error");
			}
		}
		else
		{
			throw ref new ::Platform::Exception(ERROR_INVALID_PARAMETER, L"nodeuwputil::Extract Invalid zip file");
		}
	}
}
