#pragma once

#include <QResizeEvent>
#include <QTableView>
#include <QHeaderView>
#include <QDialog>

class MTableModel : public QAbstractTableModel {
	public:
		MTableModel(QWidget *parent) : QAbstractTableModel(parent) {
			colz = 0;
			matrix.clear();
		}
		int rowCount(const QModelIndex&) const {
			return matrix.size();
		}
		int columnCount(const QModelIndex&) const {
			return colz;
		}
		void setColumnCount(int clz) {
			colz = clz;
		}
		QVariant data(const QModelIndex& idx, int role) const {
			if (!idx.isValid()) return QVariant();
			if (idx.row()>=matrix.size()) return QVariant();
			if (idx.column()>=colz) return QVariant();
			if (role==Qt::DisplayRole) {
				return matrix[idx.row()][idx.column()];
			} else {
				return QVariant();
			}
		}
		void clear() {
			matrix.clear();
		}
		void addrow(QStringList lst) {
			while (lst.size() < colz)
				lst.append("");
			matrix.append(lst);
		}
		void update() {
			endResetModel();
		}
	private:
		QList<QStringList> matrix;
		int colz;
};

class MTableView : public QTableView {
	Q_OBJECT
	public:
		MTableView(QWidget *parent):QTableView(parent) {
			MTableModel *model = new MTableModel(this);
			setModel(model);
			horizontalHeader()->setStretchLastSection(true);
			verticalHeader()->setDefaultSectionSize(18);
			for (int i = 0; i < 10; i++) {
				weight[i] = 0;
			}
			setSelectionBehavior(QAbstractItemView::SelectRows);
//			horizontalHeader()->setResizeMode(QHeaderView::Fixed);
			verticalHeader()->setVisible(false);
		}
		float weight[10];
	signals:
		void indexchanged(int);
	private slots:
		void currentChanged(const QModelIndex &current, const QModelIndex &prev) {
			QAbstractItemView::currentChanged(current, prev);
			emit indexchanged(current.row());
		}
	protected:
		void resizeEvent(QResizeEvent *event) {
			int wid = event->size().width();
			QHeaderView *header = horizontalHeader();
			for(int i = 0; i < header->count(); i++) {
				header->resizeSection(i, wid * weight[i]);
			}
			QTableView::resizeEvent(event);
		}
};
