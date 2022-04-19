#pragma once
#include "crnode.h"
#include "../tools/tdefs.h"

class ProjectLoader
{
public:

    static bool loadSrcDir(CRNode *node, const String& dpath, const StrList &masks, CRNode *regen_base, bool inc);
    static bool loadFileList(CRNode *node, const String& dpath, const StrList &flist, CRNode *regen_base, bool inc);

	static bool checkMaskList(const String& fname, const StrList &masks);
};
