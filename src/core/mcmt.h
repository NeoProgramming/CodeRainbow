#ifndef MCMT_H
#define MCMT_H
#include "../tools/tdefs.h"
#include "crtree.h"

String		makeLine(CRMark &mi, const String &head, const String &tail);
CR::Type	parseLine(const String &str, CRMark &mi, String *head = nullptr, String *tail = nullptr);
String		makeMarkStr(CRMark &mi);
void		parseMarkStr(const String &str, int i, CRMark &mi, String *tail = nullptr);

void createMarkers(CRMark &mi, String &beg, String &end);

String makeSigRegExp(const String &sig);
String makeMarkRegExp(const String &id, CR::Type type);
String removeComments(const String &str);

bool extractId(String &str, int index);
void replaceNonId(String &str);
void ensureUniqueId(CRNode *fnode, String &id);

//@ QColor  colorToVal(const QString &sclr);
//@ QString colorToStr(QColor clr);

#endif // MCMT_H
