#pragma once
#include "stdafx.h"
#include <KxFramework/DataView2/DataView2.h>

namespace Kortex
{
	class IModNetwork;
	class IGameMod;
	class IModTag;
}
namespace Kortex::ModManager
{
	class DisplayModel;
	class PriorityGroup;
	class FixedGameMod;
}

namespace Kortex::ModManager
{
	enum class DisplayModelColumnID
	{
		Name,
		Priority,
		Version,
		Author,
		Tags,

		DateInstall,
		DateUninstall,
		ModFolder,
		PackagePath,
		Signature,

		MAX,

		ModSource = -1,
	};
}

namespace Kortex::ModManager
{
	class DisplayModelModNode: public KxRTTI::ExtendInterface<DisplayModelModNode, KxDataView2::Node>
	{
		friend class DisplayModel;

		public:
			using ColumnID = DisplayModelColumnID;

		private:
			IGameMod* m_Mod = nullptr;
			std::vector<DisplayModelModNode> m_Children;

		private:
			void OnAttachNode();
			std::pair<bool, bool> HasAnyNetworkUpdates(std::vector<IModNetwork*>* networks = nullptr) const;

		public:
			DisplayModelModNode(IGameMod& mod)
				:m_Mod(&mod)
			{
			}
			
		public:
			bool IsEnabled(const KxDataView2::Column& column) const override;
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			wxAny GetEditorValue(const KxDataView2::Column& column) const override;

			KxDataView2::ToolTip GetToolTip(const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Column& column, const PriorityGroup& priorityGroup) const;
			wxAny GetValue(const KxDataView2::Column& column, const FixedGameMod& fixedGameMod) const;
			bool SetValue(KxDataView2::Column& column, const wxAny& value) override;

			bool Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const override;
			bool Compare(const IGameMod& left, const IGameMod& right, const KxDataView2::Column& column) const;

			bool GetAttributes(const KxDataView2::Column& column, const KxDataView2::CellState& cellState, KxDataView2::CellAttributes& attributes) const override;
			bool IsCategoryNode() const override;
			int GetRowHeight() const override;

		public:
			IGameMod& GetMod() const
			{
				return *m_Mod;
			}
			DisplayModelModNode& AddModNode(IGameMod& mod)
			{
				return m_Children.emplace_back(mod);
			}

			DisplayModel& GetDisplayModel() const;
	};
}

namespace Kortex::ModManager
{
	class DisplayModelTagNode: public KxRTTI::ExtendInterface<DisplayModelTagNode, KxDataView2::Node>
	{
		friend class DisplayModel;

		public:
			using ColumnID = DisplayModelColumnID;

		private:
			IModTag* m_Tag = nullptr;
			std::vector<DisplayModelModNode> m_Children;

		private:
			void OnAttachNode();

		public:
			DisplayModelTagNode(IModTag& tag)
				:m_Tag(&tag)
			{
			}
			
		public:
			KxDataView2::Editor* GetEditor(const KxDataView2::Column& column) const override;
			wxAny GetValue(const KxDataView2::Column& column) const override;
			
			bool Compare(const KxDataView2::Node& other, const KxDataView2::Column& column) const override;
			bool Compare(const IModTag& left, const IModTag& right, const KxDataView2::Column& column) const;

			bool GetAttributes(const KxDataView2::Column& column, const KxDataView2::CellState& cellState, KxDataView2::CellAttributes& attributes) const override;
			bool IsCategoryNode() const override;

		public:
			IModTag& GetTag() const
			{
				return *m_Tag;
			}

			DisplayModelModNode& AddModNode(IGameMod& mod)
			{
				return m_Children.emplace_back(mod);
			}
			bool HasMods() const
			{
				return !m_Children.empty();
			}
	};
}
