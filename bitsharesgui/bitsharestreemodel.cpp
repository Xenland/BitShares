#include "bitsharestreemodel.h"
#include <algorithm>
#include <assert.h>

struct TIdentity
{
    QString _name;
    TIdentity(const char* name) : _name(name) {};
};

struct TContact
{
    QString _name;
    TContact(const char* name) : _name(name) {};
};

class ITreeNode
{
public:
    virtual   QVariant name() = 0;
    virtual ITreeNode* parent() = 0;
    virtual        int childCount() = 0;
    virtual ITreeNode* child(int row) = 0;
    virtual   QVariant data(int row) = 0;
    virtual        int getRow() = 0;
    virtual        int findRow(ITreeNode* child) = 0;
};

class TTreeRoot : public ITreeNode
{
  std::vector<ITreeNode*> _children;
public:
               TTreeRoot();
      QVariant name() { return QVariant("invisibleRoot"); }
    ITreeNode* parent() { return 0; }
           int childCount()  { return _children.size(); }
           //return nullptr if children are not ITreeNodes (implies they are leaf nodes)
    ITreeNode* child(int row) { return _children[row]; }
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
public:
    std::vector<TIdentity*> _identities;
               TIdentityMode() : AGuiMode("Identities") {}
           int childCount() { return _identities.size(); }
    ITreeNode* child(int /* row */) { return nullptr; }
      QVariant data(int row) { return _identities[row]->_name; }
};

class TContactMode : public AGuiMode
{
    std::vector<TContact*> _contacts;
public:
               TContactMode() : AGuiMode("Contacts") {}
           int childCount() { return _contacts.size(); }
    ITreeNode* child(int /* row */) { return nullptr; }
      QVariant data(int row) { return _contacts[row]->_name; }
};

TTreeRoot::TTreeRoot()
{
  TIdentityMode* identityMode = new TIdentityMode;
  identityMode->_identities.push_back(new TIdentity("Dan1"));
  identityMode->_identities.push_back(new TIdentity("Dan2"));
  identityMode->_identities.push_back(new TIdentity("Dan3"));
  _children.push_back(identityMode);
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
    //internalPointer points to item's parent item
    ITreeNode* parentParentItem = static_cast<ITreeNode*>(parent.internalPointer());
    ITreeNode* parentItem = parentParentItem->child(parent.row());
    //if item is not leaf item (non-null), ask for it's child count
    if (parentItem)
      return parentItem->childCount();
    else
      return 0;
}

Qt::ItemFlags BitSharesTreeModel::flags(const QModelIndex & ) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant BitSharesTreeModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    ITreeNode* parentItem = static_cast<ITreeNode*>(index.internalPointer());
    return parentItem->data(index.row());
}

QModelIndex BitSharesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ITreeNode* parentItem;
    if (!parent.isValid())
       {
       parentItem = &gTreeRoot;
       }
    else
      {
      ITreeNode* parentParentItem = static_cast<ITreeNode*>(parent.internalPointer());
      parentItem = parentParentItem->child(parent.row());
      }

    if (parentItem->childCount() > row)
        return createIndex(row, column, parentItem);
    else
        return QModelIndex();
}

QModelIndex BitSharesTreeModel::parent(const QModelIndex& index) const
{
  ITreeNode* parentItem = static_cast<ITreeNode*>(index.internalPointer());
  ITreeNode* parentParentItem = parentItem->parent();
  if (parentParentItem)
    return createIndex(parentParentItem->findRow(parentItem),0,parentParentItem);
  else
    return QModelIndex();
}
