#include "bitsharestreemodel.h"
#include <algorithm>
#include <assert.h>

//bitshares database info
#if 1
#include <bts/profile.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/addressbook/contact.hpp>
#endif

/** For now I think a contact should represent a single identity of another person. Later we should probably define
a class that represents a single person with multiple identities (as far as the user knows, anyways), but do we need it right away? */
struct TContact
{
    QString _name;
    TContact(const char* name) : _name(name) {};
};

/** ITreeNodes are non-leaf nodes in the bitshares tree model. All leaf nodes will have an ITreeNode as a parent.
*/
class ITreeNode
{
public:
    virtual   QVariant name() = 0;
    virtual ITreeNode* parent() = 0;
    virtual       void addChild(ITreeNode*) = 0;
    virtual        int childCount() = 0;
    virtual ITreeNode* child(int row) = 0; ///implmentations should return nullptr if children are not ITreeNodes (implies they are leaf nodes)
    virtual   QVariant data(int row) = 0;
    virtual        int getRow() = 0;
    virtual        int findRow(ITreeNode* child) = 0;
};

/** Base class for any interior tree nodes */
class ATreeNode : public ITreeNode
{
       QString _name;
    ITreeNode* _parent;
public:
               ATreeNode(const char* name, ITreeNode* parent) : _name(name), _parent(parent) {}
          void addChild(ITreeNode*) { assert(false && "only override this if derived class accepts ITreeNodes as children"); }
      QVariant name()   { return QVariant(_name); }
    ITreeNode* parent() { return _parent; }
           int getRow() { return parent()->findRow(this); }
           int findRow(ITreeNode*) { assert(false && "no tree node children for ATreeNode"); return 0; }
};

/** Interior tree node with ITreeNodes as children */
class ATreeNodeParentNode : public ATreeNode
{
protected:
    std::vector<ITreeNode*> _children;
public:
               ATreeNodeParentNode(const char* name, ITreeNode* parent) : ATreeNode(name,parent) {}
           int childCount()  { return _children.size(); }
          void addChild(ITreeNode* child) { _children.push_back(child); }           
    ITreeNode* child(int row) { return _children[row]; } 
      QVariant data(int row) { return _children[row]->name(); }
           int findRow(ITreeNode* child) 
             {
             auto foundI = std::find( _children.begin(), _children.end(), child); 
             return foundI - _children.begin();
             }
};



/** TTreeRoot is a singleton that represents the root of the tree. It manages the top level nodes of the tree,
    but it isn't shown in the tree view. 
*/
class TTreeRoot : public ATreeNodeParentNode
{
public:
               TTreeRoot();
           int getRow() { return 0; }
};


TTreeRoot gTreeRoot; /// Singleton tree root

/** A group of email messages */
class TMailBox
{
    QString _name;
public:
               TMailBox(const char* name) : _name(name) {}
       QString name()   { return _name; }
};

/** Tree node that manages list of mail boxes for an identity.
*/
class TMailBoxListNode : public ATreeNode
{
           TMailBox _inBox;         //incoming emails shown here
           TMailBox _draftBox;      //unfinished/unsent emails shown here
           TMailBox _pendingBox;    //emails scheduled to be sent, but not yet acknowledged by recipient (what if multiple recipients?)
           TMailBox _sentBox;       //emails successfully sent to recipient

    std::vector<TMailBox*> _mailBoxes; //all mailboxes for current identity
public:
               TMailBoxListNode(ITreeNode* parent);               
           int childCount() { return _mailBoxes.size(); }
    ITreeNode* child(int /* row */) { return nullptr; } 
      QVariant data(int row) { return _mailBoxes[row]->name(); }
};

TMailBoxListNode::TMailBoxListNode(ITreeNode* parent) 
  : ATreeNode("Mail",parent),
    _inBox("Inbox"),
    _draftBox("Drafts"),
    _pendingBox("Pending"),
    _sentBox("Sent")
{
  _mailBoxes.push_back(&_inBox);
  _mailBoxes.push_back(&_draftBox);
  _mailBoxes.push_back(&_pendingBox);
  _mailBoxes.push_back(&_sentBox);
}

/** Tree node that manages the list of email and chat contacts (people you communicate with) for an identity
*/
class TContactListNode : public ATreeNode
{
    std::vector<TContact*> _contacts;
public:
               TContactListNode(ITreeNode* parent) : ATreeNode("Contacts",parent) {}
           int childCount() { return _contacts.size(); }
    ITreeNode* child(int /* row */) { return nullptr; }
      QVariant data(int row) { return _contacts[row]->_name; }
};

/** A tree node that represents one of the user's identities (bitname) */
class TIdentityNode : public ATreeNodeParentNode
{
  TMailBoxListNode mailBoxList;
  TContactListNode contactList;
public:
    TIdentityNode(const char* name, ITreeNode* parent);
};

TIdentityNode::TIdentityNode(const char* name, ITreeNode* parent)
 : ATreeNodeParentNode(name,parent),
   mailBoxList(this),
   contactList(this)
{
  _children.push_back(&mailBoxList);
  _children.push_back(&contactList);
}

/** Tree node that manages the list of user identities (e.g. for email and chat) in the profile
*/
class TIdentityListNode : public ATreeNodeParentNode
{
public:
               TIdentityListNode(ITreeNode* parent)
                  : ATreeNodeParentNode("Identities",parent) 
                 {
                 //TODO: remove sample data
                 addChild(new TIdentityNode("Dan1",this));
                 addChild(new TIdentityNode("Dan2",this));
                 addChild(new TIdentityNode("Dan3",this));
                 }
};


/** Modes of the GUI are defined here */
TTreeRoot::TTreeRoot()
 : ATreeNodeParentNode("invisibleRoot",nullptr)
{
    addChild(new TIdentityListNode(this));
    addChild(new TContactListNode(this));
}


BitSharesTreeModel::BitSharesTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
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
    //if parent not valid, must be at tree root node
    if (!parent.isValid())
        return gTreeRoot.childCount();
    //internalPointer points to item's parent item
    ITreeNode* parentParentItem = static_cast<ITreeNode*>(parent.internalPointer());
    ITreeNode* parentItem = parentParentItem->child(parent.row());
    //if item is not a leaf item (non-null), ask for it's child count
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
        parentItem = &gTreeRoot;
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
