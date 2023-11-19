#pragma once

#include <QAbstractTableModel>
#include <QTableView>

class TBModel : public QAbstractTableModel {
	public:
		TBModel(QObject* p = NULL);
		int rowCount(const QModelIndex& idx = QModelIndex()) const;
		int columnCount(const QModelIndex& idx = QModelIndex()) const;
		QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;
		QVariant headerData(int,Qt::Orientation, int role = Qt::DisplayRole) const;
		void update();
		void updateLine(int);
		void updateCell(int,int);
		void insertRow(int, const QModelIndex& idx = QModelIndex());
		void removeRow(int, const QModelIndex& idx = QModelIndex());
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
};

class BMLModel : public QAbstractTableModel {
	public:
		BMLModel(QObject* = NULL);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QVariant data(const QModelIndex&, int = Qt::DisplayRole) const;
		void update();
		QModelIndex index(int row, int column, const QModelIndex& = QModelIndex()) const;
};
