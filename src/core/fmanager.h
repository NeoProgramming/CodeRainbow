#pragma once
#include <string>
#include <list>
#include "../tools/tdefs.h"
#include "sig.h"



class FManager {
public:
	FManager();
	~FManager();
	FItem* getFileItem(const String &fpath);
	FItem* regFileItem(const String &fpath);
	void delFileItem(FItem* item);
	void clear();
private:
	FList m_flist;
};
