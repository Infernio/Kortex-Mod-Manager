#pragma once
#include "stdafx.h"
#include "StepStack.h"
#include "Info/InfoPage.h"
#include "Requiremets/RequirementsPage.h"
#include "Components/ComponentsPage.h"
#include "ModPackages/ModPackage.h"
#include "GameMods/ModManager/BasicGameMod.h"
#include "Utility/KOperationWithProgress.h"
#include "Utility/KTempFolderKeeper.h"
#include <KxFramework/KxPanel.h>
#include <KxFramework/KxButton.h>
#include <KxFramework/KxLabel.h>
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxStdDialog.h>
#include <KxFramework/KxNotebook.h>
#include <KxFramework/KxAuiNotebook.h>
#include <KxFramework/KxTreeList.h>
#include <KxFramework/KxListBox.h>
#include <KxFramework/KxImageView.h>
#include <KxFramework/KxThumbView.h>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxSplitterWindow.h>
#include <KxFramework/KxProgressBar.h>
#include <KxFramework/KxWithOptions.h>
#include <KxFramework/KxDataView.h>
class KImageViewerEvent;

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
		friend class InstallationOperation;

		public:
			static void ShowInvalidPackageDialog(wxWindow* window, const wxString& packagePath);

		private:
			/* UI */
			KxButton* m_CancelButton = nullptr;
			KxButton* m_BackwardButton = nullptr;
			KxButton* m_ForwardButton = nullptr;
			wxSimplebook* m_PageContainer = nullptr;
			WizardPageID m_CurrentPage = WizardPageID::None;

			InfoPage m_PageInfo;
			RequirementsPage m_PageRequirements;
			ComponentsPage m_PageComponents;

			// Installing
			KxPanel* m_Installing_Pane = nullptr;
			KxProgressBar* m_Installing_MinorProgress = nullptr;
			KxProgressBar* m_Installing_MajorProgress = nullptr;
			KxLabel* m_Installing_MinorStatus = nullptr;
			KxLabel* m_Installing_MajorStatus = nullptr;
			bool m_ShouldCancel = false;
			bool m_IsComplete = false;
			bool m_InfoPageLeft = false;
			InstallationOperation* m_InstallThread = nullptr;

			// Done
			KxPanel* m_Done_Pane = nullptr;
			KxLabel* m_Done_Label = nullptr;

			/* Package */
			std::unique_ptr<ModPackage> m_Package;
			ModManager::BasicGameMod m_ModEntry;
			const IGameMod* m_ExistingMod = nullptr;
			KPPFFileEntryRefArray m_InstallableFiles;

		private:
			bool CreateUI(wxWindow* parent);
			void LoadUIOptions();

			std::array<WizardPage*, 3> GetPages()
			{
				return {&m_PageInfo, &m_PageRequirements, &m_PageComponents};
			}

			wxWindow* CreateUI_Installing();
			wxWindow* CreateUI_Done();

			void SetLabelByCurrentPage();

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

			void CollectAllInstallableEntries();
			void SortInstallableFiles();
			void ShowInstallableFilesPreview();

			bool OnBeginInstall();
			bool OnEndInstall();
			void OnMinorProgress(KxFileOperationEvent& event);
			void OnMajorProgress(KxFileOperationEvent& event);

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

			const ModManager::BasicGameMod* GetModEntry() const
			{
				return &m_ModEntry;
			}
			ModManager::BasicGameMod* GetModEntry()
			{
				return &m_ModEntry;
			}

			bool ShouldCancel() const
			{
				return m_ShouldCancel;
			}
			bool IsCompleted() const
			{
				return m_IsComplete;
			}
			bool IsInfoPageLeft() const
			{
				return m_InfoPageLeft;
			}

			void SwitchPage(WizardPageID page);
			bool OnLeavingPage(WizardPageID page);
	};
}

namespace Kortex::InstallWizard
{
	class InstallationOperation: public KOperationWithProgressBase
	{
		private:
			WizardDialog* m_InstallWizard = nullptr;

		public:
			InstallationOperation(WizardDialog* installWizard)
				:KOperationWithProgressBase(true), m_InstallWizard(installWizard)
			{
			}

		public:
			void LinkHandler(wxEvtHandler* eventHandler, wxEventType type) override
			{
				using T = wxEventTypeTag<KxFileOperationEvent>;
				GetEventHandler()->Bind(static_cast<T>(type), &WizardDialog::OnMinorProgress, m_InstallWizard);
				KOperationWithProgressBase::LinkHandler(eventHandler, type);
			}
	};
}
