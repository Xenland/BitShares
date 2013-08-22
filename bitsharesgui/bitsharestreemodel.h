#ifndef BITSHARESTREEMODEL_H
#define BITSHARESTREEMODEL_H

#include <QAbstractItemModel>

class BitSharesTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit BitSharesTreeModel(QObject *parent = 0);
    
            int columnCount(const QModelIndex & parent = QModelIndex()) const;
            int rowCount(const QModelIndex & parent = QModelIndex()) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
       QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
signals:
    
public slots:
    
};

#endif // BITSHARESTREEMODEL_H
