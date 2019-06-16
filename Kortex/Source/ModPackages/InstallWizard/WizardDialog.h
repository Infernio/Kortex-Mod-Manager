#pragma once
#include "stdafx.h"
#include "StepStack.h"
#include "Info/InfoPage.h"
#include "Requiremets/RequirementsPage.h"
#include "Components/ComponentsPage.h"
#include "Installation/InstallationPage.h"
#include "Completed/CompletedPage.h"
#include "ModPackages/ModPackage.h"
#include "GameMods/ModManager/BasicGameMod.h"
#include "Utility/KTempFolderKeeper.h"
#include <KxFramework/KxButton.h>
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxWithOptions.h>

namespace Kortex::InstallWizard
{
	wxDECLARE_EVENT(KEVT_IW_DONE, wxNotifyEvent);

	enum class DialogOptions
	{
		None = 0,
		Debug = 1 << 0,
		Cleanup = 1 << 1,
	};
}

namespace Kortex::InstallWizard
{
	class WizardDialog:
		public KxStdDialog,
		public KTempFolderKeeper,
		public KxWithOptions<DialogOptions, DialogOptions::None>
	{
		friend class WizardPage;
		friend class InstallOperation;

		public:
			static void ShowInvalidPackageDialog(wxWindow* window, const wxString& packagePath);

		private:
			KxButton* m_CancelButton = nullptr;
			KxButton* m_BackwardButton = nullptr;
			KxButton* m_ForwardButton = nullptr;
			wxSimplebook* m_PageContainer = nullptr;
			WizardPageID m_CurrentPage = WizardPageID::None;

			InfoPage m_PageInfo;
			RequirementsPage m_PageRequirements;
			ComponentsPage m_PageComponents;
			InstallationPage m_PageInstallation;
			CompletedPage m_PageCompleted;

			std::unique_ptr<ModPackage> m_Package;
			ModManager::BasicGameMod m_ModEntry;
			const IGameMod* m_ExistingMod = nullptr;
			bool m_LeftInfoPage = false;

		private:
			bool CreateUI(wxWindow* parent);
			void LoadUIOptions();
			void SetLabelByCurrentPage();
			bool IsInfoPageLeft() const
			{
				return m_LeftInfoPage;
			}

		public:
			void OpenPackage(const wxString& packagePath);
			bool LoadPackage();
			bool ProcessLoadPackage();
			void FindExistingMod();
			void AcceptExistingMod(const Kortex::IGameMod& mod);
			void LoadHeaderImage();

			bool AskCancel(bool canCancel);
			void OnClose(wxCloseEvent& event);
			void OnCancelButton(wxCommandEvent& event);
			void OnGoBackward(wxCommandEvent& event);
			void OnGoForward(wxCommandEvent& event);

			void SetModData();
			KxUInt32Vector GetFilesOfFolder(const KPPFFolderEntry* folder) const;
			wxString GetFinalPath(uint32_t index, const wxString& installLocation, const KPPFFileEntry* fileEntry) const;
			KxStringVector GetFinalPaths(const KxUInt32Vector& filePaths, const wxString& installLocation, const KPPFFolderEntry* folder) const;
			void RunInstall();

		private:
			int GetViewSizerProportion() const override
			{
				return 1;
			}
			wxOrientation GetViewSizerOrientation() const override
			{
				return wxVERTICAL;
			}
			wxOrientation GetViewLabelSizerOrientation() const override
			{
				return wxVERTICAL;
			}
			bool IsEnterAllowed(wxKeyEvent& event, wxWindowID* id = nullptr) const override
			{
				return false;
			}
			bool IsCloseBoxEnabled() const override
			{
				return true;
			}
			wxWindow* GetDialogMainCtrl() const override
			{
				return m_PageContainer;
			}

		public:
			WizardDialog();
			WizardDialog(wxWindow* parent, const wxString& packagePath);
			bool Create(wxWindow* parent, const wxString& packagePath);
			bool Create(wxWindow* parent, std::unique_ptr<ModPackage>&& package);
			~WizardDialog();

		public:
			const KPackageProject& GetConfig() const
			{
				return m_Package->GetConfig();
			}
			KPackageProject& GetConfig()
			{
				return m_Package->GetConfig();
			}
			const KArchive& GetArchive() const
			{
				return m_Package->GetArchive();
			}
			KArchive& GetArchive()
			{
				return m_Package->GetArchive();
			}
			const IGameMod* GetExistingMod() const
			{
				return m_ExistingMod;
			}
			const ModPackage& GetPackage() const
			{
				return *m_Package;
			}

			std::array<WizardPage*, 5> GetPages()
			{
				return {&m_PageInfo, &m_PageRequirements, &m_PageComponents, &m_PageInstallation, &m_PageCompleted};
			}
			InfoPage& GetInfoPage()
			{
				return m_PageInfo;
			}
			RequirementsPage& GetRequirementsPage()
			{
				return m_PageRequirements;
			}
			ComponentsPage& GetComponentsPage()
			{
				return m_PageComponents;
			}
			InstallationPage& GetInstallationPage()
			{
				return m_PageInstallation;
			}
			CompletedPage& GetCompletedPage()
			{
				return m_PageCompleted;
			}

			const IGameMod& GetModEntry() const
			{
				return m_ModEntry;
			}
			IGameMod& GetModEntry()
			{
				return m_ModEntry;
			}

			void SwitchPage(WizardPageID page);
			bool OnLeavingPage(WizardPageID page);
	};
}