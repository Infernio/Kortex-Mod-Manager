#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Common.h"
#include "IModSource.h"
#include <KxFramework/KxSingleton.h>
class KMainWindow;
class KxAuiToolBarItem;
class KxAuiToolBarEvent;

namespace Kortex
{
	namespace NetworkManager
	{
		class Config;
	}
	namespace NetworkManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	};

	class INetworkManager:
		public ManagerWithTypeInfo<IManager, NetworkManager::Internal::TypeInfo>,
		public KxSingletonPtr<INetworkManager>
	{
		friend class KMainWindow;

		private:
			void CallOnToolBarButton(KxAuiToolBarEvent& event)
			{
				OnToolBarButton(event);
			}

		protected:
			virtual void OnSetToolBarButton(KxAuiToolBarItem* button) = 0;
			virtual void OnToolBarButton(KxAuiToolBarEvent& event) = 0;

			template<class T> T& AddModSource()
			{
				IModSource::Vector& items = GetModSources();
				return static_cast<T&>(*items.emplace_back(IModSource::Create<T>()));
			}

		public:
			INetworkManager();

		public:
			virtual const NetworkManager::Config& GetConfig() const = 0;
			virtual wxString GetCacheFolder() const = 0;
		
			virtual const IModSource::Vector& GetModSources() const = 0;
			virtual IModSource::Vector& GetModSources() = 0;

			virtual IModSource* GetDefaultModSource() const = 0;
			virtual IModSource* GetModSource(const wxString& name) const = 0;
			bool HasModSource(const wxString& name) const
			{
				return GetModSource(name) != nullptr;
			}
			bool IsDefaultModSourceAuthenticated() const;
			
			virtual void OnAuthStateChanged() = 0;
	};
}
