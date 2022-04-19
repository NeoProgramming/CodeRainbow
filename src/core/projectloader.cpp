#include "projectloader.h"
#include "../tools/tfbase.h"
#include "../tools/dosmatch.h"
#include <boost/filesystem.hpp>

//@ port to Qt

bool ProjectLoader::loadSrcDir(CRNode *node, const String& dirpath, const StrList &masks, CRNode *regen_base, bool inc)
{
	// load subnode from source directory on filesystem
    // scan ALL dirs (without use masks)
    
	for (boost::filesystem::directory_iterator it(dirpath); it!=boost::filesystem::directory_iterator(); ++it) {
        //QString path = it.next();
        //QFileInfo fi = it.fileInfo();
		String path = it->path().wstring();
		if(boost::filesystem::is_directory( it->path() )) {
			CRNode *dnode = nullptr;
			if(regen_base) {
				dnode = regen_base->findChildPath(path);
			//    qDebug() << "D.REGEN " << path << "==" << dnode;
			}
			if(!dnode) {
				dnode = CRNode::mkFSNode(node, CR::NT_DIR, it->path().filename().wstring(), path, inc);
			 //   qDebug() << "D.!DNODE " << dnode;
			}
			// recursively call the function for the given folder
			ProjectLoader::loadSrcDir(dnode, path, masks, regen_base, inc);
			// if the node is empty, then no files have been added, delete
			if(dnode->branchesCount() == 0)
				node->removeBranch(dnode);
		}
		else if(boost::filesystem::is_regular_file(it->path()) && checkMaskList(it->path().filename().wstring(), masks) ) {
			CRNode *fnode = nullptr;
			if(regen_base) {
				fnode = regen_base->findChildPath(path);
			//    qDebug() << "F.REGEN " << path << "==" << fnode;
			}
			if(!fnode) {
				fnode = CRNode::mkFSNode(node, CR::NT_FILE, it->path().filename().wstring(), path, inc);
			//   qDebug() << "F.!DNODE " << fnode;
			}
            fnode->setFlags(CRNode::NF_ACTIVE, 0);
		}
	}

	return true;
}

bool ProjectLoader::loadFileList(CRNode *node, const String& dirpath, const StrList &flist, CRNode *regen_base, bool inc)
{
    // from paths list
	Path dir(dirpath);
    for(auto i = flist.begin(), e = flist.end(); i!=e; ++i) {
		String rpath = boost::filesystem::relative(*i, dir).generic_wstring();// .wstring();

        // if regeneration - first look for the file
        CRNode *fnode = nullptr;
        if(regen_base) {
            fnode = regen_base->findChildPath(*i);
        }
        // if the file is not found:
        if(!fnode) {
            // first restore the chain of folders (no matter generation or regeneration)
            int slash, start = 0;
            CRNode *dnode = node;
            while((slash = rpath.find('/', start)) >=0 ) {
                String dname = rpath.substr(start, slash-start);
                String dpath = rpath.substr(0, slash);
				String abs_dpath = (dir/dpath).wstring();
				// looking for a node with this path;
				CRNode *inner_dnode = dnode->findChildPath(abs_dpath);
                if(!inner_dnode)
					dnode = CRNode::mkFSNode(dnode, CR::NT_DIR, dname, abs_dpath, false);
                else
                    dnode = inner_dnode;
                start = slash+1;
            }

            // now add a node
            fnode = CRNode::mkFSNode(dnode, CR::NT_FILE, rpath.substr(start), *i, inc);
//          qDebug() << "L.!DNODE " << fnode;
        }
    }
    return true;
}

bool ProjectLoader::checkMaskList(const String& fname, const StrList &masks)
{
	for(StrList::const_iterator i=masks.begin(), e = masks.end(); i!=e; ++i) {
		if(dosMatch<Utf16>(fname.c_str(), (*i).c_str()))
			return true;
	}
	return false;
}

