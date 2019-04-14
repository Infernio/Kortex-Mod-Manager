#include "stdafx.h"
#include "IDownloadEntry.h"
#include <Kortex/ModManager.hpp>
#include <Kortex/NetworkManager.hpp>
#include <Kortex/GameInstance.hpp>
#include <KxFramework/KxFileItem.h>

namespace Kortex
{
	GameID IDownloadEntry::GetTargetGameID() const
	{
		const IGameInstance* instance = GetTargetGame();
		return instance ? instance->GetGameID() : GameIDs::NullGameID;
	}
	void IDownloadEntry::SetTargetGameID(const GameID& id)
	{
		SetTargetGame(IGameInstance::GetTemplate(id));
	}

	void IDownloadEntry::ResetDownloadLink()
	{
		GetDownloadInfo() = {};
	}

	bool IDownloadEntry::Save() const
	{
		KxFileStream stream(GetMetaFilePath(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
		return stream.IsOk() && Serialize(stream);
	}
	bool IDownloadEntry::Load(const wxString& xmlFile, const KxFileItem& fileItem)
	{
		KxFileStream stream(xmlFile, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
		if (stream.IsOk() && DeSerialize(stream))
		{
			if (fileItem.IsOK())
			{
				SetDownloadedSize(fileItem.GetFileSize());
				if (fileItem.GetFileSize() == GetFileInfo().Size)
				{
					SetPaused(false);
					SetFailed(false);
				}
			}
			return true;
		}
		return false;
	}
}
