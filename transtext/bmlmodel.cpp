#include "models.h"
#include "base.h"

BMLModel::BMLModel(QObject* p):QAbstractTableModel(p) {}

int BMLModel::columnCount(const QModelIndex&) const {
	return 2;
}

int BMLModel::rowCount(const QModelIndex&) const {
	return bookmarks.size();
}

QVariant BMLModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (col < 0)) return res;
	if (row >= rowCount()) return res;
	if (col >= columnCount()) return res;
	if (row >= bookmarks.size()) return res;
	TBookmark bm = bookmarks.at(row);
	TPage* pg = findPage(bm.pgid);
	switch (role) {
		case Qt::DisplayRole:
			switch (col) {
				case 0:
					res = bm.name;
					break;
				case 1:
					if (pg) res = QString("%0:%1").arg(pg->name).arg(bm.row);
					break;
				//case 2:
				//	res = bm.descr;
				//	break;
			}
			break;
	}
	return res;
}

QModelIndex BMLModel::index(int row, int column, const QModelIndex&) const {
	QModelIndex res = createIndex(row, column, (void*)this);
	return res;
}

void BMLModel::update() {
	endResetModel();
}
