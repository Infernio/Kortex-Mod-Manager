#include "stdafx.h"
#include "Fallout4.h"
#include <KxFramework/KxFileStream.h>

namespace
{
	template<class CounterType> void ReadPluginList(KxFileStream& stream, KxStringVector& plugins)
	{
		size_t count = stream.ReadObject<CounterType>();
		for (size_t i = 0; i < count; i++)
		{
			plugins.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()));
		}
	}
}

namespace Kortex::SaveManager::BethesdaSave
{
	bool Fallout4::OnRead(const KxFileItem& fileItem)
	{
		KxFileStream stream(fileItem.GetFullPath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (stream.IsOk())
		{
			if (stream.ReadStringASCII(12) == wxS("FO4_SAVEGAME"))
			{
				// Skip 'headerSize'
				stream.Skip<uint32_t>();

				// Read version
				m_SaveVersion = stream.ReadObject<uint32_t>();

				// Read basic info
				m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), KTr("SaveManager.Info.SaveIndex"));
				m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Name"));
				m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), KTr("SaveManager.Info.Level"));
				m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Location"));
				m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.TimeInGame"));
				m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.Race"));

				// Player sex
				uint16_t playerSex = stream.ReadObject<uint16_t>();
				m_BasicInfo.emplace_back(playerSex == 0 ? KTr("SaveManager.Info.SexMale") : KTr("SaveManager.Info.SexFemale"), KTr("SaveManager.Info.Sex"));

				// Skip 'playerCurExp', 'playerLvlUpExp', 'filetime' fields
				stream.Skip<float32_t, float32_t, FILETIME>();

				// Read image
				uint32_t width = stream.ReadObject<uint32_t>();
				uint32_t height = stream.ReadObject<uint32_t>();
				m_Bitmap = ReadBitmapRGBA(stream.ReadVector<uint8_t>(width * height * 4), width, height);

				// Skip 'formVersion' field
				stream.Skip<uint8_t>();

				// Read game version
				m_BasicInfo.emplace_back(stream.ReadStringUTF8(stream.ReadObject<uint16_t>()), KTr("SaveManager.Info.GameVersion"));

				// Skip 'pluginInfoSize' field
				stream.Skip<uint32_t>();

				// Read plugins list (ESM + ESP)
				ReadPluginList<uint8_t>(stream, m_PluginsList);

				// ESL
				ReadPluginList<uint16_t>(stream, m_PluginsList);
				return true;
			}
		}
		return false;
	}
}