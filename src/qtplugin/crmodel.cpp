#include "crmodel.h"
#include "../core/crtree.h"
#include "../core/crnode.h"
#include "../gui/gui.h"
#include "contentview.h"

#include <QIcon>
#include <QDebug>

CRModel::CRModel(CRTree *pTree, QObject *parent)
    : QAbstractItemModel(parent)
    , m_pTree(pTree)
{
}

QModelIndex CRModel::index(int row, int column, const QModelIndex &parent) const
{
	// generate an index for a given element of a given node
    // row is not global but local inside parent, that's good

    // determine if there is a parent index
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    // define parent node
    CRNode *parentNode;
    if (!parent.isValid())
        parentNode = m_pTree->getRoot();
    else
        parentNode = static_cast<CRNode *>(parent.internalPointer());

    // create index for child node
    if (parentNode && row >= 0 && row < parentNode->branchesCount()) {
        CRNode *childNode = parentNode->branchByIndex(row);
        return createIndex(row, column, childNode);
    }

    return QModelIndex();
}

QModelIndex CRModel::parent(const QModelIndex &child) const
{
    // get the parent node for a given node
    if (!child.isValid())
        return QModelIndex();

    CRNode *childNode = static_cast<CRNode*>(child.internalPointer());
    if(!childNode)
        return QModelIndex();
    CRNode *parentNode = childNode->getParent();
    if (!parentNode || parentNode == m_pTree->getRoot())
        return QModelIndex();

    CRNode *grandParentNode = parentNode->getParent();
    int row = -1;
    if (grandParentNode)
        row = grandParentNode->branchIndex(parentNode);

    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, parentNode);
    return QModelIndex();
}

int CRModel::rowCount(const QModelIndex &parent) const
{
    // return the number of elements (rows) for a given node
    if (parent.column() > 0)
        return 0;

    CRNode *parentNode = parent.isValid() ? static_cast<CRNode*>(parent.internalPointer()) : m_pTree->getRoot();
    if(!parentNode)
        return 0;
    return parentNode->branchesCount();
}

int CRModel::columnCount(const QModelIndex &parent) const
{
    // return the number of columns for a given node
    Q_UNUSED(parent);
    return 1;
}

QVariant CRModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // return header data
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            if(section == 0) {
                if(pCR->pathRecentProject.empty())
                    return tr("<new project>");
                return QString::fromStdWString( pCR->pathRecentProject );
            }
        }
    }

    return QVariant();
}

QVariant CRModel::data(const QModelIndex &index, int role) const
{
    // return cell data at index
    int column(index.column());
    if (!index.isValid())
        return QVariant();

    CRNode *node = static_cast<CRNode*>(index.internalPointer());
    if(node) {
//        qDebug() << "data for " << node->getInfo(' ');

        // update index
        node->idx = index;

        if (column == 0) {
            // return data
            if (role == Qt::DisplayRole) {
                return QString::fromStdWString(node->dispName());
            }
            else if (role == Qt::DecorationRole) {
                return Gui::getTreeIcon( node->type );
            }
            else if(role == Qt::UserRole) {
                return QVariant::fromValue<CRNode*>(node);
            }
            else if(role == Qt::BackgroundRole) {
                if(node->tclr != CLR_NONE)
                    return QBrush(node->tclr);
            }
            else if(role == Qt::ForegroundRole) {
                if(node->tclr != CLR_NONE)
                    return QBrush(Gui::luminance(node->tclr)<127 ? Qt::white : Qt::black);
            }
            else if(role == Qt::CheckStateRole) {
                return (node->flags & CRNode::NF_CHECKED) ? Qt::Checked : Qt::Unchecked;
            }
            else if(role == Qt::FontRole) {
                if(node->flags & CRNode::NF_CURRENT) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
            }
        }
    }
    return QVariant();
}

//setData method of TreeModel
bool CRModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    CRNode *node = static_cast<CRNode*>(index.internalPointer());
    if(node && role == Qt::CheckStateRole) {
        node->flags ^= CRNode::NF_CHECKED;
//      emit dataChanged(index, index);
        emit checkChanged(node);
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags CRModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if ( index.column() == 0 )
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

void CRModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

void CRModel::layoutModel(CRNode *par)
{
    QList<QPersistentModelIndex> parents;
    parents << par->idx;
    emit layoutAboutToBeChanged(parents);
    emit layoutChanged(parents);
}

void CRModel::layoutModelAll()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

/*@
void CRModel::updateModel(CR::NodeAction na, CRNode *node)
{
    if(!node)
        return;
    if(na == CR::NA_EDIT) {
    //    setData(node->idx, QVariant());
        emit dataChanged(node->idx,node->idx);
    }
    else if(na == CR::NA_ADDCHILD) {
        int i = node->parent->branchesCount()-1;
        beginInsertRows(node->parent->idx, i, i);
        endInsertRows();
    }
    else if(na == CR::NA_ADDBELOW) {
        int i = node->parent->branches.indexOf(node);
        beginInsertRows(node->parent->idx, i, i);
        endInsertRows();
    }
    if(na == CR::NA_MOVEUP || na == CR::NA_MOVEDOWN) {
        QList<QPersistentModelIndex> parents;
        parents << node->parent->idx;
        emit layoutAboutToBeChanged(parents);
        emit layoutChanged(parents);
    }
    else if(na == CR::NA_MOVELEFT || na == CR::NA_MOVERIGHT) {
        QList<QPersistentModelIndex> parents;
        parents << node->parent->idx;
        parents << node->idx;
        emit layoutAboutToBeChanged(parents);
        emit layoutChanged(parents);
    }
}
*/

void CRModel::beginRemoveItem(CRNode *node, int num)
{
    beginRemoveRows(node->idx, num, num);
}

void CRModel::endRemoveItem()
{
    endRemoveRows();
}

QModelIndex CRModel::getIndexByNode(CRNode *wanted_node, const QModelIndex &parent) const
{
    // searching for an index by a node - iterate through the whole tree!
    if(!wanted_node)
        return QModelIndex();
    CRNode *parentNode;
    if (!parent.isValid())
        parentNode = m_pTree->getRoot();
    else
        parentNode = static_cast<CRNode *>(parent.internalPointer());

    // checking element
    if(parentNode == wanted_node)
        return parent;

    // delegate to children
    int n = parentNode->branchesCount();
    for(int i=0; i<n; i++) {
        CRNode *childNode = parentNode->branchByIndex(i);
        QModelIndex res = getIndexByNode(wanted_node, createIndex(i, 0, childNode));
        if(res.isValid())
            return res;
    }

    return QModelIndex();
}
