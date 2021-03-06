#include "stdafx.h"
#include "InterfaceSection.h"
#include "ModPackageProject.h"
#include <Kortex/Application.hpp>

namespace Kortex::PackageProject
{
	ImageItem::ImageItem(const wxString& path, const wxString& description, bool isVisible)
		:m_Path(path), m_Description(description), m_IsVisiable(isVisible)
	{
	}
	ImageItem::~ImageItem()
	{
	}
}

namespace Kortex::PackageProject
{
	InterfaceSection::InterfaceSection(ModPackageProject& project)
		:ProjectSection(project)
	{
	}
	InterfaceSection::~InterfaceSection()
	{
	}
	
	const ImageItem* InterfaceSection::FindImageByPath(const wxString& path) const
	{
		auto it = std::find_if(m_Images.begin(), m_Images.end(), [path](const ImageItem& v)
		{
			return v.GetPath().IsSameAs(path, false);
		});
		return it != m_Images.end() ? &(*it) : nullptr;
	}
	ImageItem* InterfaceSection::FindImageByPath(const wxString& path)
	{
		auto it = std::find_if(m_Images.begin(), m_Images.end(), [path](const ImageItem& v)
		{
			return v.GetPath().IsSameAs(path, false);
		});
		return it != m_Images.end() ? &(*it) : nullptr;
	}
}
