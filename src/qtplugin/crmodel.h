#pragma once

#include <QAbstractItemModel>
#include <QSet>

class CRTree;
class ContentView;
#include "../core/crnode.h"

struct CRNode;

//@@@ navigation tree model - single object for all trees
class CRModel : public QAbstractItemModel
{
    Q_OBJECT
    CRTree *m_pTree;
public:
    CRModel(CRTree *pTree, QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void resetModel();
    void layoutModel(CRNode *par);
    void layoutModelAll();
    void beginRemoveItem(CRNode *node, int num);
    void endRemoveItem();
signals:
    void checkChanged(CRNode *node);
public:
    QModelIndex getIndexByNode(CRNode *node, const QModelIndex &parent = QModelIndex()) const;
};
