#include "stdafx.h"
#include "KDispatcher.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KFileTreeNode.h"
#include "KVariablesDatabase.h"
#include "KVirtualGameFolderWorkspace.h"
#include "UI/KMainWindow.h"
#include "KEvents.h"
#include "KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileFinder.h>
#include <chrono>
#include <execution>

namespace
{
	int64_t GetClockTime()
	{
		using namespace std::chrono;
		return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
	};

	struct FinderHashComparator
	{
		bool operator()(const wxString& lhs, const wxString& rhs) const
		{
			return KxComparator::IsEqual(lhs, rhs, true);
		}
	};
	struct FinderHashHasher
	{
		size_t operator()(const wxString& value) const
		{
			return KFileTreeNode::HashFileName(value);
		}
	};
	using FinderHash = std::unordered_map<wxString, size_t, FinderHashHasher, FinderHashComparator>;

	void FindFilesInTree(KFileTreeNode::CRefVector& nodes, const KFileTreeNode& rootNode, const KDispatcher::FilterFunctor& filter, bool recurse)
	{
		auto ScanSubTree = [&nodes, &filter, recurse](KFileTreeNode::CRefVector& directories, const KFileTreeNode& folderNode)
		{
			for (const KFileTreeNode& node: folderNode.GetChildren())
			{
				// Add node if no filter is specified or filter accepts this node
				if (!filter || filter(node))
				{
					nodes.push_back(&node);
				}

				// Add directory to scan on next round
				if (recurse && node.IsDirectory())
				{
					directories.push_back(&node);
				}
			}
		};

		// Top level
		KFileTreeNode::CRefVector directories;
		ScanSubTree(directories, rootNode);
		
		// Subdirectories
		while (!directories.empty())
		{
			KFileTreeNode::CRefVector roundDirectories;
			for (const KFileTreeNode* node: directories)
			{
				ScanSubTree(roundDirectories, *node);
			}
			directories = std::move(roundDirectories);
		}
	}
	void BuildTreeBranch(const KDispatcher::ModRefVector& mods, KFileTreeNode::Vector& children, const KFileTreeNode* rootNode, KFileTreeNode::RefVector& directories)
	{
		std::unordered_map<size_t, size_t> hash;
		hash.reserve(mods.size());
		const wxString rootPath = rootNode ? rootNode->GetRelativePath() : wxEmptyString;

		// Iterate manually, without using 'IterateOverModsEx'
		for (auto it = mods.rbegin(); it != mods.rend(); ++it)
		{
			const KModEntry& modEntry = **it;
			if (modEntry.IsInstalled())
			{
				// If we have root node, look for files in real file tree
				// Otherwise use mod's tree root
				const KFileTreeNode* searchNode = &modEntry.GetFileTree();
				if (rootNode)
				{
					searchNode = KFileTreeNode::NavigateToFolder(modEntry.GetFileTree(), rootPath);
				}

				if (searchNode)
				{
					// Not enough, but at least something
					children.reserve(searchNode->GetChildrenCount());

					for (const KFileTreeNode& node: searchNode->GetChildren())
					{
						auto hashIt = hash.try_emplace(node.GetNameHash(), (size_t)-1);
						if (hashIt.second)
						{
							KFileTreeNode& newNode = children.emplace_back(modEntry, node.GetItem(), rootNode);
							newNode.CopyBasicAttributes(node);

							// Save index to new node to add alternatives to it later
							// I'd use pointer, but it can be invalidated on reallocation
							hashIt.first->second = children.size() - 1;
						}
						else
						{
							const size_t index = hashIt.first->second;
							KFileTreeNode& newAlternative = children[index].GetAlternatives().emplace_back(modEntry, node.GetItem(), rootNode);
							newAlternative.CopyBasicAttributes(node);
						}
					}
				}
			}
		}

		// Fill directories array
		for (KFileTreeNode& node: children)
		{
			if (node.IsDirectory())
			{
				directories.push_back(&node);
			}
		}
	}
}

wxString KDispatcherCollision::GetLocalizedCollisionName(KMMDispatcherCollisionType type)
{
	switch (type)
	{
		case KMM_DCT_NONE:
		{
			return KTr(KxID_NONE);
		}
		case KMM_DCT_UNKNOWN:
		{
			return KTr("ModExplorer.Collision.Unknown");
		}
		case KMM_DCT_OVERWRITTEN:
		{
			return KTr("ModExplorer.Collision.Overwritten");
		}
		case KMM_DCT_OVERWRITES:
		{
			return KTr("ModExplorer.Collision.Owerwrites");
		}
	};
	return wxEmptyString;
}

//////////////////////////////////////////////////////////////////////////
bool KDispatcher::SimpleSearcher::operator()(const KFileTreeNode& node) const
{
	return node.GetItem().IsElementType(m_ElementType) &&
		(!m_ActiveOnly || (m_ActiveOnly && node.GetMod().IsEnabled())) &&
		KxComparator::Matches(node.GetName(), m_Filter, true);
}

//////////////////////////////////////////////////////////////////////////
KModEntry* KDispatcher::IterateOverModsEx(const ModRefVector& mods, const IterationFunctor& functor, IterationOrder order, bool activeOnly) const
{
	switch (order)
	{
		case IterationOrder::Direct:
		{
			for (KModEntry* entry: mods)
			{
				if (CheckConditionsAndCallFunctor(functor, *entry, activeOnly))
				{
					return entry;
				}
			}
			break;
		}
		case IterationOrder::Reversed:
		{
			for (auto it = mods.rbegin(); it != mods.rend(); ++it)
			{
				if (CheckConditionsAndCallFunctor(functor, **it, activeOnly))
				{
					return *it;
				}
			}
			break;
		}
	};
	return NULL;
}
bool KDispatcher::CheckConditionsAndCallFunctor(const IterationFunctor& functor, const KModEntry& modEntry, bool activeOnly) const
{
	return modEntry.IsInstalled() && (activeOnly && modEntry.IsEnabled() || !activeOnly) && functor(modEntry);
}

void KDispatcher::RebuildTreeIfNeeded() const
{
	if (m_VirtualTreeInvalidated)
	{
		const_cast<KDispatcher*>(this)->m_VirtualTreeInvalidated = false;
		const_cast<KDispatcher*>(this)->UpdateVirtualTree();
	}
}
void KDispatcher::OnVirtualTreeInvalidated(KEvent& event)
{
	InvalidateVirtualTree();
}

void KDispatcher::UpdateVirtualTree()
{
	const KModEntry::RefVector mods = KModManager::GetInstance()->GetAllEntries(true);
	m_VirtualTree.ClearChildren();

	constexpr const bool useRecursive = true;
	const int64_t t1 = GetClockTime();

	if constexpr(useRecursive)
	{
		// Recursive (parallel)
		// Build top level
		KFileTreeNode::RefVector directories;
		BuildTreeBranch(mods, m_VirtualTree.GetChildren(), NULL, directories);

		// Build subdirectories
		std::function<void(KFileTreeNode::RefVector&&)> Execute;
		Execute = [this, &Execute, &mods](KFileTreeNode::RefVector&& directories)
		{
			std::for_each(std::execution::par_unseq, directories.begin(), directories.end(), [this, &Execute, &mods](KFileTreeNode* node)
			{
				KFileTreeNode::RefVector directories;
				BuildTreeBranch(mods, node->GetChildren(), node, directories);
				Execute(std::move(directories));
			});
		};
		Execute(std::move(directories));
	}
	else
	{
		// Iterational (sequential)
		// Build top level
		KFileTreeNode::RefVector directories;
		BuildTreeBranch(mods, m_VirtualTree.GetChildren(), NULL, directories);

		// Build subdirectories
		while (!directories.empty())
		{
			KFileTreeNode::RefVector roundDirectories;
			roundDirectories.reserve(directories.size());

			for (KFileTreeNode* node: directories)
			{
				BuildTreeBranch(mods, node->GetChildren(), node, roundDirectories);
			}
			directories = std::move(roundDirectories);
		}
	}
	wxLogInfo("KDispatcher::UpdateVirtualTree: %lld", GetClockTime() - t1);
}
void KDispatcher::InvalidateVirtualTree()
{
	m_VirtualTreeInvalidated = true;
	KEvent::MakeSend<KModEvent>(KEVT_MOD_VIRTUAL_TREE_INVALIDATED);
}

const KFileTreeNode& KDispatcher::GetVirtualTree() const
{
	RebuildTreeIfNeeded();
	return m_VirtualTree;
}

KModEntry* KDispatcher::IterateOverMods(IterationFunctor functor, IterationOrder order, bool includeWriteTarget, bool activeOnly) const
{
	RebuildTreeIfNeeded();
	return IterateOverModsEx(KModManager::GetInstance()->GetAllEntries(includeWriteTarget), functor, order, activeOnly);
}

const KFileTreeNode* KDispatcher::ResolveLocation(const wxString& relativePath) const
{
	RebuildTreeIfNeeded();
	return KFileTreeNode::NavigateToAny(m_VirtualTree, relativePath);
}
wxString KDispatcher::ResolveLocationPath(const wxString& relativePath, const KModEntry** owningMod) const
{
	// This is an absolute path, return it as is.
	if (relativePath.Length() >= 2 && relativePath[1] == wxT(':'))
	{
		KxUtility::SetIfNotNull(owningMod, nullptr);
		return relativePath;
	}

	if (const KFileTreeNode* node = ResolveLocation(relativePath))
	{
		KxUtility::SetIfNotNull(owningMod, &node->GetMod());
		return node->GetFullPath();
	}

	// Fallback to write target
	KxUtility::SetIfNotNull(owningMod, nullptr);
	return KModManager::GetInstance()->GetModEntry_WriteTarget()->GetModFilesDir() + wxS('\\') + relativePath;
}

const KFileTreeNode* KDispatcher::BackTrackFullPath(const wxString& fullPath) const
{
	RebuildTreeIfNeeded();
	return m_VirtualTree.WalkTree([&fullPath](const KFileTreeNode& node)
	{
		return KxComparator::IsEqual(node.GetFullPath(), fullPath, true);
	});
}

KFileTreeNode::CRefVector KDispatcher::Find(const wxString& relativePath, const FilterFunctor& filter, bool recurse) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	const KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(m_VirtualTree, relativePath);
	if (folderNode)
	{
		FindFilesInTree(nodes, *folderNode, filter, recurse);
	}
	return nodes;
}
KFileTreeNode::CRefVector KDispatcher::Find(const KFileTreeNode& rootNode, const FilterFunctor& filter, bool recurse) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, rootNode, filter, recurse);

	return nodes;
}
KFileTreeNode::CRefVector KDispatcher::Find(const KModEntry& modEntry, const FilterFunctor& filter, bool recurse) const
{
	RebuildTreeIfNeeded();

	KFileTreeNode::CRefVector nodes;
	FindFilesInTree(nodes, modEntry.GetFileTree(), filter, recurse);

	return nodes;
}

KDispatcherCollision::Vector KDispatcher::FindCollisions(const KModEntry& scannedMod, const wxString& relativePath) const
{
	RebuildTreeIfNeeded();

	KDispatcherCollision::Vector collisions;
	KMMDispatcherCollisionType type = KMM_DCT_OVERWRITTEN;
	auto CheckMod = [&scannedMod, &collisions, &type, &relativePath](const KModEntry& modEntry)
	{
		// Flip collision type after this mod is found
		if (&modEntry == &scannedMod)
		{
			type = KMM_DCT_OVERWRITES;

			// Don't count scanned mod itself
			return false;
		}

		// If this mod is enabled and it have such file - add collision info for it.
		if (modEntry.IsEnabled())
		{
			const KFileTreeNode* node = KFileTreeNode::NavigateToAny(modEntry.GetFileTree(), relativePath);
			if (node && node->IsFile())
			{
				collisions.emplace_back(&modEntry, type);
			}
		}
		return false;
	};
	
	IterateOverMods(CheckMod, IterationOrder::Reversed);
	std::reverse(collisions.begin(), collisions.end());
	return collisions;
}

KDispatcher::KDispatcher()
{
	KEvent::Bind(KEVT_MOD_FILES_CHANGED, &KDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_INSTALLED, &KDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MOD_UNINSTALLED, &KDispatcher::OnVirtualTreeInvalidated, this);
	KEvent::Bind(KEVT_MODS_REORDERED, &KDispatcher::OnVirtualTreeInvalidated, this);
}
