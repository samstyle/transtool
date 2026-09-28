#pragma once

#include <QtGui>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTreeWidgetItem>

//#include "ui_mainwin.h"
#include "ui_transwidget.h"
#include "ui_iconwindow.h"
#include "ui_bookmark.h"
#include "ui_bookmarks.h"

#include "models.h"
#include "base.h"
#include "replace.h"

extern TPage* curPage;

enum {
	roleId = Qt::UserRole,
	roleIcon,
	roleImgDir
};

class TRBLoader {
	public:
		TRBLoader() {}
		int load(QString, QTreeWidgetItem*);
		int save(QString, QTreeWidgetItem*);
	private:
		QBuffer buf;
		QDataStream strm;

		QList<int> getlist();
		void putlist(QList<int>&);
		QTreeWidgetItem* add_item(QTreeWidgetItem*, QString, QUuid, QUuid=0);

		void v7_load(QTreeWidgetItem*);
		int v7_load_icons();
		int v7_load_bookmarks();
		int v7_load_page();
		int v7_load_tree(QTreeWidgetItem*);
		int v7_load_imgs();

		int v7_save(QTreeWidgetItem*);
		void v7_save_tree(QTreeWidgetItem*);
		void v7_save_leaf(QTreeWidgetItem*);
};

class xFileTreeWidget : public QWidget {
	Q_OBJECT
	public:
		xFileTreeWidget(QWidget* = nullptr);
		void setDir(QString);
	signals:
		void s_selected(QString);
	private:
		QTreeView* tree;
		QLabel* view;
		QFileSystemModel* model;
		void keyPressEvent(QKeyEvent*);
	private slots:
		void itemClick(const QModelIndex&, const QModelIndex&);
		void itemChosed(const QModelIndex&);
};

class xPlayer : public QLabel {
	Q_OBJECT
	public:
		xPlayer(QWidget* = nullptr);
		bool playLine(TPage*, int); // TLine);
		void reset();
		QFont fnt;
	signals:
		void clicked();
		void clicked_r();
		void selected(QString);
		void closed();
	private:
		int moved;
		int curline;
		QPoint mousepos;
		QPoint picpos;
		QSize picsize;
		QTimer timer;

		QMovie* mov;		// player for bg image
		QList<QPixmap> ovrlist;		// overlay with text
		QList<xImageList> bglist;	// current line image sequence (stable)
		QList<xImageList> curbglist;	// current line image sequence (runtime, 1st element deleted every img change)
		TLine lin;
		QList<QRect> zones;
		QStringList selabs;
		int curzone;
		int getZone(QPoint);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void keyPressEvent(QKeyEvent*);
		void closeEvent(QCloseEvent*);
		void resizeEvent(QResizeEvent*);
	private slots:
		void redrawFrame();
		void nextImage();
};

class TWindow : public QWidget {
	Q_OBJECT
	public:
		TWindow();
		void keyPress(QKeyEvent*);
		int saveChanged();
		Ui::TTWidget ui;
	public slots:
		void openPrj(QString path = "");
		void replace(QString, QString);
	signals:
		void rqReplace();
		void textChanged(QString);
	private:
		// Ui::MainWin ui;
		Ui::IconWin icoui;
		Ui::AddBookmark bmui;
		Ui::Bookmarks blui;

		QDialog* icowin;
		QDialog* bmwin;
		xPlayer* player;
		Replacer* rpl;
		QFileDialog fdial;
		xFileTreeWidget* ftw;

		QDialog* blwin;
		BMLModel* blmod;

		int curRow;
		QTreeWidgetItem* curItem;
		QString prjPath;
		QClipboard* clip;
		TBModel* model;
		QSettings opt;

		QMenu* tbMenu;
		QMenu* sjMenu;
		QMenu* bmMenu;
		QMenu* treeMenu;

		void fillBlock(const QList<TLine>*);
		void disableTab();
		QTreeWidgetItem* addItem(QTreeWidgetItem*,QString,QUuid,QUuid = QUuid(0));

		void setPage(QUuid);
		void setProgress();
		void setEdit(bool);

		void lineUp();
		void lineDown();

		QString getImgDir(QTreeWidgetItem*);
		int getCurrentRow();
		QTreeWidgetItem* getCurrentParent();
		int selectItemByPageID(QUuid);

		TRBLoader trb;

		void loadVer78(QByteArray&, QTreeWidgetItem*);
	private slots:
		void treeContextMenu();
		void tbContextMenu();
		void jumpLine(QAction*);
		void findUntrn();
		void cbrdChanged();

		void play();
		void playLine();
		bool playNext();
		void playPrev();
		void jmpToLabel(QString);
		void fontSelect();

		void findStr(QString);
		void findNext();
		void findPrev();

		void changePage();
		void changeRow(QItemSelection);
		void changeSrc(QString);
		void changeTrn(QString);
		void changeSNm(QString);
		void changeTNm(QString);

		void pageInfo();
		void bmList();
		void goToBookmark(const QModelIndex&);
		void treeItemChanged(QTreeWidgetItem*);

		void setImgDir();
		void rmImgDir();

		void saveBranch();

		void newDir();
		void delPage();
		void delItem(QTreeWidgetItem*);
		void sortTree();
		void mergePages();
		void clearTrn();

		void joinLine();
		void splitLine();
		void splitName();

		void rowDelete();
		void rowInsert(bool);
		void rowInsertAbove();
		void rowInsertBelow();
		void pageSplit();

		void changeIcon();
		void fillIconList();
		void setIcon(QListWidgetItem*);
		void loadIcon();
		void delIcon();

		void askBookmark();
		void askRmBookmark();
		void newBookmark();
		void fillSJMenu();

		void insertImgLine();
		void insertImgText(QString);

		void newPrj();
		void mergePrj(QString path = "");
		bool savePrj(QString path = "", QTreeWidgetItem* = NULL);
		void saveIt();
		QList<TPage> openFiles(QFileDialog::FileMode);

		TPage* newPage();
		void openSrc();
		void insertSrc();
		void saveSrc();
	protected:
		void keyPressEvent(QKeyEvent*);
};
