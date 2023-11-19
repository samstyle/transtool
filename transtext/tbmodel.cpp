#include "mainwin.h"

// TBModel

void TBModel::update() {
	endResetModel();
}

void TBModel::updateCell(int row, int col) {
	QModelIndex idx = index(row,col);
	emit dataChanged(idx,idx);
}

void TBModel::updateLine(int row) {
	QModelIndex bidx = index(row, 0);
	QModelIndex eidx = index(row, columnCount() - 1);
	emit dataChanged(bidx, eidx);
}

TBModel::TBModel(QObject* p):QAbstractTableModel(p) {}

int TBModel::columnCount(const QModelIndex&) const {
	return 5;
}

int TBModel::rowCount(const QModelIndex&) const {
	if (curPage == NULL) return 0;
	return curPage->text.size();
}

QVariant TBModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	if ((idx.row() < 0) || (idx.row() > rowCount(idx))) return res;
	if ((idx.column() < 0) || (idx.column() > columnCount(idx))) return res;
	switch(role) {
		case Qt::DecorationRole:
			if ((idx.column() == 0) && !curPage->text.at(idx.row()).bmrkId.isNull())
				res = res = QIcon(":/bookmark.png");
			break;
		case Qt::DisplayRole:
			switch (idx.column()) {
				case 1: res = curPage->text.at(idx.row()).src.name; break;
				case 2: res = curPage->text.at(idx.row()).src.text; break;
				case 3: res = curPage->text.at(idx.row()).trn.name; break;
				case 4: res = curPage->text.at(idx.row()).trn.text; break;
			}
			break;
		default:
			break;
	}
	return res;
}

void TBModel::insertRow(int row, const QModelIndex& idx) {
	emit beginInsertRows(idx,row,row);
	emit endInsertRows();
}

void TBModel::removeRow(int row, const QModelIndex& idx) {
	emit beginRemoveRows(idx,row,row);
	emit endRemoveRows();
}

QVariant TBModel::headerData(int sect,Qt::Orientation orien, int role) const {
	if (role != Qt::DisplayRole) return QVariant();
	if (orien == Qt::Vertical) return QString::number(sect+1);
	QVariant res;
	if (orien == Qt::Horizontal) {
		switch (sect) {
			case 1: res = "Name"; break;
			case 2: res = "Text"; break;
			case 3: res = "Имя"; break;
			case 4: res = "Текст"; break;
		}
	}
	return res;
}

QModelIndex TBModel::index(int row, int column, const QModelIndex&) const {
	return createIndex(row, column, (void*)this);
}
