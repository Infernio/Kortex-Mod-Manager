#include "stdafx.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KModManager.h"
#include <KxFramework/KxComparator.h>

const KFileTreeNode* KFileTreeNode::NavigateToElement(const KFileTreeNode& rootNode, const wxString& relativePath, KxFileSearchType type)
{
	if (type == KxFileSearchType::KxFS_FOLDER)
	{
		if (relativePath.IsEmpty() || *relativePath.begin() == wxS('\\') || *relativePath.begin() == wxS('/') || *relativePath.begin() == wxS('.'))
		{
			return &rootNode;
		}
	}

	if (rootNode.HasChildren())
	{
		auto ScanChildren = [](const KFileTreeNode& rootNode, const wxString& folderName) -> const KFileTreeNode*
		{
			for (const KFileTreeNode& node: rootNode.GetChildren())
			{
				if (KxComparator::IsEqual(folderName, node.GetName(), true))
				{
					return &node;
				}
			}
			return NULL;
		};

		const KFileTreeNode* finalNode = NULL;

		// This ugly construction is faster than
		// for (const wxString& folderName: KxString::Split(relativePath, wxS("\\")))
		// So using it.
		const wxChar separator = wxS('\\');
		size_t pos = 0;
		size_t separatorPos = relativePath.find(separator);
		if (separatorPos == wxString::npos)
		{
			separatorPos = relativePath.length();
		}

		while (pos < relativePath.length() && separatorPos <= relativePath.length())
		{
			const wxString folderName = relativePath.SubString(pos, separatorPos - 1);
			finalNode = ScanChildren(finalNode ? *finalNode : rootNode, folderName);
			if (finalNode == NULL)
			{
				break;
			}

			pos += folderName.length() + 1;
			separatorPos = relativePath.find(separator, pos);

			// No separator found, but this is not the last element
			if (separatorPos == wxString::npos && pos < relativePath.length())
			{
				separatorPos = relativePath.length();
			}
		}

		if (finalNode && finalNode->GetItem().IsElementType(type))
		{
			return finalNode;
		}
	}
	return NULL;
}

const KFileTreeNode* KFileTreeNode::WalkTree(const TreeWalker& functor) const
{
	std::function<const KFileTreeNode*(const KFileTreeNode::Vector&)> Recurse;
	Recurse = [&Recurse, &functor](const KFileTreeNode::Vector& children) -> const KFileTreeNode*
	{
		for (const KFileTreeNode& node: children)
		{
			if (!functor(node))
			{
				return &node;
			}

			if (node.HasChildren())
			{
				Recurse(node.GetChildren());
			}
		}
		return NULL;
	};
	return Recurse(m_Children);
}
const KFileTreeNode* KFileTreeNode::WalkToRoot(const TreeWalker& functor) const
{
	const KFileTreeNode* node = this;
	while (node && !node->IsRootNode() && node->IsOK())
	{
		if (functor(*node))
		{
			node = node->GetParent();
		}
		else
		{
			break;
		}
	}
	return node;
}

bool KFileTreeNode::HasAlternativesFromActiveMods() const
{
	if (!m_Alternatives.empty())
	{
		for (const KFileTreeNode& node: m_Alternatives)
		{
			if (!node.GetMod().IsEnabled())
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

wxString KFileTreeNode::GetRelativePath() const
{
	wxString path = m_Item.GetFullPath();
	if (path.Replace(m_Mod->GetLocation(KMM_LOCATION_MOD_FILES), wxEmptyString, false) == 1 && !path.IsEmpty() && path[0] == wxS('\\'))
	{
		// Remove leading slash
		path.Remove(0, 1);
	}
	return path;
}
