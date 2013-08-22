#include "bitsharestreemodel.h"
#include <algorithm>
#include <assert.h>

struct TIdentity
{
    QString _name;
};

struct TContact
{
    QString _name;
};

class ITreeNode
{
public:
    virtual   QVariant name() = 0;
    virtual ITreeNode* parent() = 0;
    virtual        int childCount() = 0;
    virtual   QVariant data(int row) = 0;
    virtual        int getRow() = 0;
    virtual        int findRow(ITreeNode* child) = 0;
};

class TTreeRoot : public ITreeNode
{
  std::vector<ITreeNode*> _children;
public:
               TTreeRoot();
    virtual   QVariant name() { return QVariant("invisibleRoot"); }
    ITreeNode* parent() { return 0; }
           int childCount()  { return _children.size(); }
      QVariant data(int row) { return _children[row]->name(); }
           int getRow()      { return 0; }
           int findRow(ITreeNode* child) 
             {
             auto foundI = std::find( _children.begin(), _children.end(), child); 
             return foundI - _children.begin();
             }
};

TTreeRoot gTreeRoot;

class AGuiMode : public ITreeNode
{
  QString _name;
public:
               AGuiMode(const char* name) : _name(name) {}
      QVariant name()   { return QVariant(_name); }
    ITreeNode* parent() { return &gTreeRoot; }
           int getRow() { return parent()->findRow(this); }
           int findRow(ITreeNode*) { assert(false && "no tree node children for AGuiNodes"); return 0; }
};

class TIdentityMode : public AGuiMode
{
    std::vector<TIdentity*> _identities;
public:
               TIdentityMode() : AGuiMode("Identities") {}
           int childCount() { return _identities.size(); }
      QVariant data(int row) { return _identities[row]->_name; }
};

class TContactMode : public AGuiMode
{
    std::vector<TContact*> _contacts;
public:
               TContactMode() : AGuiMode("Contacts") {}
           int childCount() { return _contacts.size(); }
      QVariant data(int row) { return _contacts[row]->_name; }
};

TTreeRoot::TTreeRoot()
{
  _children.push_back(new TIdentityMode());
  _children.push_back(new TContactMode());
}


BitSharesTreeModel::BitSharesTreeModel(QObject* parent) :
    QAbstractItemModel(parent)
{    
}

int BitSharesTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int BitSharesTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid())
        return gTreeRoot.childCount();
    ITreeNode* parentItem = static_cast<ITreeNode*>(parent.internalPointer());
    return parentItem->childCount();
}

Qt::ItemFlags BitSharesTreeModel::flags(const QModelIndex & ) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant BitSharesTreeModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    ITreeNode* item = static_cast<ITreeNode*>(index.internalPointer());
    return item->data(index.row());
}

QModelIndex BitSharesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ITreeNode* parentItem;
    if (!parent.isValid())
        parentItem = &gTreeRoot;
    else
        parentItem = static_cast<ITreeNode*>(parent.internalPointer());

    if (parentItem->childCount() < row)
        return createIndex(row, column, parentItem);
    else
        return QModelIndex();
}

QModelIndex BitSharesTreeModel::parent(const QModelIndex& index) const
{
  ITreeNode* item = static_cast<ITreeNode*>(index.internalPointer());
  ITreeNode* parent = item->parent();
  if (parent)
    return createIndex(parent->getRow(),0,parent);
  else
    return QModelIndex();
}
