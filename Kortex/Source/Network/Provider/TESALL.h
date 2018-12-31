#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "Network/INetworkProvider.h"
#include "TESALLModInfo.h"
#include <KxFramework/KxSingleton.h>
class KxCURLSession;

namespace Kortex::NetworkManager
{
	class TESALLProvider: public INetworkProvider, public KxSingletonPtr<TESALLProvider>
	{
		public:
			static constexpr NetworkProviderID GetTypeID()
			{
				return NetworkProviderIDs::TESALL;
			}

		protected:
			virtual bool DoAuthenticate(wxWindow* window = nullptr) override;
			virtual bool DoValidateAuth(wxWindow* window = nullptr) override;
			virtual bool DoSignOut(wxWindow* window = nullptr) override;
			virtual bool DoIsAuthenticated() const override;

		public:
			TESALLProvider();

		public:
			virtual KImageEnum GetIcon() const override;
			virtual wxString GetName() const override;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const override;
			virtual wxString GetModURL(ModID modID, const wxString& modSignature = wxEmptyString, const GameID& id = GameIDs::NullGameID) override;
	};
}
