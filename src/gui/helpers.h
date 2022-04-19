#pragma once

#include "../core/crnode.h"
#include "../core/mark.h"

inline QDebug operator<<(QDebug debug, const CRMark& mi)
{
    QDebugStateSaver saver(debug);
    debug.nospace() <<  "{id:" << mi.id << ", tags:" << mi.tags << ", type:" << mi.type << ", clr:" << hex << mi.clr << "}";
    return debug;
}

inline QDebug operator<<(QDebug debug, const CRInfo& ni)
{
    QDebugStateSaver saver(debug);
    debug.nospace() <<  "{name:" << ni.name << ", path:" << ni.path << ", id:" << ni.id << ", text:" << ni.text << ", clr:" << hex << ni.clr << "}";
    return debug;
}

// a special declaration in order to be able to pack the node into variants
Q_DECLARE_METATYPE(CRNode*)


inline QDebug operator<<(QDebug debug, const CR::Mark& word)
{
    QDebugStateSaver saver(debug);
    debug.nospace() <<  "(" << word.lineNumber << ":" << word.length << ")";
    return debug;
}

