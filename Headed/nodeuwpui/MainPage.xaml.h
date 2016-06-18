//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace nodeuwpui
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
	private:
		void Run();
		void CopyFolderSync(Windows::Storage::StorageFolder^ source, Windows::Storage::StorageFolder^ destination);
		void PopulateArgsVector(std::vector<std::shared_ptr<char>> &argVector, Windows::Data::Xml::Dom::XmlNodeList^ argNodes,
			bool isStartupScript = false);
		std::shared_ptr<char> PlatformStringToChar(const wchar_t* str, int strSize);
	};
}
