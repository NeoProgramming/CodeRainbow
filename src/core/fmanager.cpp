#include "fmanager.h"

FManager::FManager()
{
}

FManager::~FManager()
{
	clear();
}

// find and get a file element along the path
FItem* FManager::getFileItem(const String &fpath)
{
	if(fpath.empty())
		return nullptr;
	for(auto i=m_flist.begin(), e=m_flist.end(); i!=e; ++i) {
		if( boost::filesystem::equivalent( (*i)->fpath, fpath) )
			return (*i);
	}
	return nullptr;
}

// register file in manager
FItem* FManager::regFileItem(const String &fpath)
{
	if(fpath.empty())
		return nullptr;
	FItem *item = getFileItem(fpath);
	if(!item) {
		item = new FItem(fpath);
		m_flist.push_back(item);
	}
	return item;
}

void FManager::delFileItem(FItem* item)
{
	if(!item)
		return;

}

void FManager::clear()
{
	for(auto i=m_flist.begin(), e=m_flist.end(); i!=e; ++i) {
		delete (*i);
	}
	m_flist.clear();
}
