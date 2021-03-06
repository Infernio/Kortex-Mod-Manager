#pragma once
#include "stdafx.h"
#include "Application/AppOption.h"
#include <KxFramework/KxSingleton.h>
class KxXMLDocument;

namespace Kortex
{
	class IGameMod;
	class IGamePlugin;
	class IGameInstance;
	class AppOption;
	class IVariableTable;

	namespace GameInstance
	{
		namespace FolderName
		{
			constexpr static const auto Overwrites = wxS("Overwrites");
			constexpr static const auto Saves = wxS("Saves");
			constexpr static const auto Config = wxS("Config");
		}
	}
}

namespace Kortex::GameInstance
{
	class ProfileMod
	{
		public:
			using Vector = std::vector<ProfileMod>;

		public:
			wxString m_Signature;
			intptr_t m_Priority = -1;
			bool m_IsActive = false;

		public:
			ProfileMod(const IGameMod& mod, bool active);
			ProfileMod(const wxString& signature, bool active, intptr_t priority = -1);

		public:
			bool IsOK() const
			{
				return !m_Signature.IsEmpty();
			}

			bool IsActive() const
			{
				return m_IsActive;
			}
			intptr_t GetPriority() const
			{
				return m_Priority;
			}

			const wxString& GetSignature() const
			{
				return m_Signature;
			}
			IGameMod* GetMod() const;
	};
}

namespace Kortex::GameInstance
{
	class ProfilePlugin
	{
		public:
			using Vector = std::vector<ProfilePlugin>;

		public:
			wxString m_Name;
			intptr_t m_Priority = -1;
			bool m_IsActive = false;

		public:
			ProfilePlugin(const IGamePlugin& plugin, bool active);
			ProfilePlugin(const wxString& name, bool active, intptr_t priority = -1);

		public:
			bool IsOK() const
			{
				return !m_Name.IsEmpty();
			}
			
			bool IsActive() const
			{
				return m_IsActive;
			}
			intptr_t GetPriority() const
			{
				return m_Priority;
			}

			const wxString& GetName() const
			{
				return m_Name;
			}
			IGamePlugin* GetPlugin() const;
	};
}

namespace Kortex
{
	class IGameProfile:
		public Application::IWithConfig,
		public Application::WithProfileOptions<IGameProfile>
	{
		friend class IGameInstance;
		friend class AppOption;

		public:
			using ProfileMod = GameInstance::ProfileMod;
			using ProfilePlugin = GameInstance::ProfilePlugin;

			using Vector = std::vector<std::unique_ptr<IGameProfile>>;
			using RefVector = std::vector<IGameProfile*>;
			using CRefVector = std::vector<const IGameProfile*>;

		public:
			static IGameProfile* GetActive();
			static wxString ProcessID(const wxString& id);
			static void UpdateVariablesUsingActive(IVariableTable& variables);

		protected:
			static bool CreateLocalFolder(const wxString& id, const wxString& name);

		public:
			virtual ~IGameProfile() = default;

		public:
			bool IsActive() const
			{
				return GetActive() == this;
			}

			wxString GetConfigFile() const;
			wxString GetProfileDir() const;
			wxString GetProfileRelativePath(const wxString& name) const;
			wxString GetSavesDir() const;
			wxString GetConfigDir() const;
			wxString GetOverwritesDir() const;

			virtual std::unique_ptr<IGameProfile> Clone() const = 0;
			virtual void LoadConfig() = 0;
			virtual void SyncWithCurrentState() = 0;

			virtual wxString GetID() const = 0;
			virtual void SetID(const wxString& id) = 0;

			virtual bool IsLocalSavesEnabled() const = 0;
			virtual void SetLocalSavesEnabled(bool value) = 0;

			virtual bool IsLocalConfigEnabled() const = 0;
			virtual void SetLocalConfigEnabled(bool value) = 0;

			virtual const ProfileMod::Vector& GetMods() const = 0;
			virtual ProfileMod::Vector& GetMods() = 0;

			virtual const ProfilePlugin::Vector& GetPlugins() const = 0;
			virtual ProfilePlugin::Vector& GetPlugins() = 0;
	};
}
