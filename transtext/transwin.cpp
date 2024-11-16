#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QMenu>

#include <string.h>

#include "mainwin.h"
#include "filetypes.h"

#define NEW_LOADER 1
#define NEW_SAVER 1

QColor blkcol;
TPage* curPage = nullptr;
int changed = 0;

TWindow::TWindow() {
	curPage = nullptr;
	curItem = nullptr;
	prjPath.clear();

	clip = QApplication::clipboard();
	connect(clip, &QClipboard::dataChanged, this, &TWindow::cbrdChanged);

	ui.setupUi(this);
	model = new TBModel;
	ui.table->setModel(model);

	fdial.setOption(QFileDialog::DontUseNativeDialog, true);

	QTableView* tab = ui.table;
	tab->setColumnWidth(0,20);
	tab->setColumnWidth(1,50);
	tab->setColumnWidth(2,200);
	tab->setColumnWidth(3,50);
	tab->setColumnWidth(4,200);
	tab->horizontalHeader()->setStretchLastSection(true);
	tab->setSelectionBehavior(QTableWidget::SelectRows);
	tab->verticalHeader()->setDefaultSectionSize(17);
	tab->verticalHeader()->setVisible(false);

	ftw = new xFileTreeWidget();
	ftw->setDir("/");
	connect(ftw, &xFileTreeWidget::s_selected, this, &TWindow::insertImgText);

	ui.widFind->hide();

	ui.actAddBookmark->setData(-1);
	ui.actRmBookmark->setData(-1);

	ui.srcline->addAction(ui.actSplitLine);
	ui.srcline->addAction(ui.actSplitName);

	connect(ui.actNewProj,SIGNAL(triggered()),this,SLOT(newPrj()));
	connect(ui.actOpen,SIGNAL(triggered()),this,SLOT(openPrj()));
	connect(ui.actMergePage,SIGNAL(triggered()),this,SLOT(mergePrj()));
	connect(ui.actSave,SIGNAL(triggered()),this,SLOT(saveIt()));
	connect(ui.actSaveAs,SIGNAL(triggered()),this,SLOT(savePrj()));

	connect(ui.actNewPage,SIGNAL(triggered()),this,SLOT(newPage()));
	connect(ui.actOpenSrc,SIGNAL(triggered()),this,SLOT(openSrc()));
	connect(ui.actInsertSrc,SIGNAL(triggered()),this,SLOT(insertSrc()));
	connect(ui.actSaveSrc,SIGNAL(triggered()),this,SLOT(saveSrc()));
	connect(ui.actPlayerFont,SIGNAL(triggered()),this,SLOT(fontSelect()));

	connect(ui.actSplitLine,SIGNAL(triggered()),this,SLOT(splitLine()));
	connect(ui.actSplitName,SIGNAL(triggered()),this,SLOT(splitName()));

	connect(ui.srcline,SIGNAL(textChanged(QString)),this,SLOT(changeSrc(QString)));
	connect(ui.trnline,SIGNAL(textChanged(QString)),this,SLOT(changeTrn(QString)));
	connect(ui.srcname,SIGNAL(textChanged(QString)),this,SLOT(changeSNm(QString)));
	connect(ui.trnname,SIGNAL(textChanged(QString)),this,SLOT(changeTNm(QString)));

	connect(ui.leFind,SIGNAL(returnPressed()),this,SLOT(findNext()));

	connect(ui.tree->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(changePage()));
	connect(ui.tree, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(pageInfo()));
	connect(ui.tree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),this,SLOT(treeItemChanged(QTreeWidgetItem*)));

	connect(ui.table->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(changeRow(QItemSelection)));
	connect(ui.table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(play()));

	connect(ui.tbInsertAbove, &QToolButton::released, this, &TWindow::rowInsertAbove);
	connect(ui.tbInsertBelow, &QToolButton::released, this, &TWindow::rowInsertBelow);
	connect(ui.tbDeleteLine, &QToolButton::released, this, &TWindow::rowDelete);
	connect(ui.tbMergeNext, &QToolButton::released, this, &TWindow::joinLine);

	treeMenu = new QMenu();
	treeMenu->addAction(ui.actNewDir);
	treeMenu->addAction(ui.actNewPage);
	treeMenu->addSeparator();
	treeMenu->addAction(ui.actSaveBranch);
	treeMenu->addSeparator();
	treeMenu->addAction(ui.actIcon);
	treeMenu->addAction(ui.actSort);
	treeMenu->addSeparator();
	treeMenu->addAction(ui.actSetImgDir);
	treeMenu->addAction(ui.actRmImgDir); ui.actRmImgDir->setVisible(false);
	treeMenu->addSeparator();
	treeMenu->addAction(ui.actMerge);
	treeMenu->addSeparator();
	treeMenu->addAction(ui.actDelPage);

	connect(ui.tree,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(treeContextMenu()));
	connect(ui.actNewDir,SIGNAL(triggered()),this,SLOT(newDir()));
	connect(ui.actSaveBranch,SIGNAL(triggered()),this,SLOT(saveBranch()));
	connect(ui.actDelPage,SIGNAL(triggered()),this,SLOT(delPage()));
	connect(ui.actSort,SIGNAL(triggered()),this,SLOT(sortTree()));
	connect(ui.actMerge, SIGNAL(triggered()), this, SLOT(mergePages()));
	connect(ui.actIcon, SIGNAL(triggered()), this, SLOT(changeIcon()));
	connect(ui.actSetImgDir,SIGNAL(triggered()),this,SLOT(setImgDir()));
	connect(ui.actRmImgDir,SIGNAL(triggered()),this,SLOT(rmImgDir()));

	connect(ui.actGrabCbrd, &QAction::toggled, ui.tbRec, &QToolButton::setChecked);
	connect(ui.tbRec, &QToolButton::toggled, ui.actGrabCbrd, &QAction::setChecked);

	tbMenu = new QMenu();
	sjMenu = tbMenu->addMenu("Labels");
	bmMenu = tbMenu->addMenu(QIcon(":/bookmark.png"),"Bookmarks");
	tbMenu->addAction(ui.actFindUntrn);
	tbMenu->addAction(ui.actSplitName);
	tbMenu->addAction(ui.actJoinLine);
	tbMenu->addAction(ui.actSplit);
	tbMenu->addSeparator();
	tbMenu->addAction(ui.actInsertBG);
	tbMenu->addSeparator();
	tbMenu->addAction(ui.actDelRows);
	tbMenu->addAction(ui.actClearTrn);

	connect(ui.table,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(tbContextMenu()));
	connect(sjMenu,SIGNAL(triggered(QAction*)),this,SLOT(jumpLine(QAction*)));
	connect(bmMenu,SIGNAL(triggered(QAction*)),this,SLOT(jumpLine(QAction*)));
	connect(ui.actClearTrn,SIGNAL(triggered()),this,SLOT(clearTrn()));
	connect(ui.actAddBookmark,SIGNAL(triggered()),this,SLOT(askBookmark()));
	connect(ui.actRmBookmark,SIGNAL(triggered()),this,SLOT(askRmBookmark()));
	connect(ui.actJoinLine,SIGNAL(triggered()),this,SLOT(joinLine()));
	connect(ui.actInsertBG,SIGNAL(triggered()),this,SLOT(insertImgLine()));

	connect(ui.actSplit,SIGNAL(triggered()),this,SLOT(pageSplit()));
	connect(ui.actDelRows,SIGNAL(triggered()),this,SLOT(rowDelete()));
	connect(ui.actFindUntrn,SIGNAL(triggered()),this,SLOT(findUntrn()));

	connect(ui.leFind, SIGNAL(textChanged(QString)), this, SLOT(findStr(QString)));

	connect(ui.tbScroll,SIGNAL(clicked(bool)),this,SLOT(findUntrn()));
	connect(ui.tbImages,SIGNAL(clicked(bool)),this,SLOT(insertImgLine()));
	connect(ui.tbBookmark,SIGNAL(released()),this,SLOT(bmList()));

	icowin = new QDialog(this);
	icoui.setupUi(icowin);
	connect(icoui.list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(setIcon(QListWidgetItem*)));
	connect(icoui.tbAdd, SIGNAL(released()), this, SLOT(loadIcon()));
	connect(icoui.tbDel, SIGNAL(released()), this, SLOT(delIcon()));

	bmwin = new QDialog(this);
	bmui.setupUi(bmwin);
	connect(bmui.pbAdd, SIGNAL(released()), this, SLOT(newBookmark()));

	blwin = new QDialog(this);
	blui.setupUi(blwin);
	blmod = new BMLModel;
	blui.table->setModel(blmod);
	connect(ui.actBookmarks, SIGNAL(triggered()), this, SLOT(bmList()));
	connect(blui.table, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(goToBookmark(const QModelIndex&)));

	player = new xPlayer();
	player->fnt.fromString(opt.value("player/font").toString());
	connect(player, SIGNAL(clicked()), this, SLOT(playNext()));
	connect(player, SIGNAL(clicked_r()), this, SLOT(playPrev()));

	rpl = new Replacer(this);
	QObject::connect(this, &TWindow::rqReplace, rpl, &Replacer::show);
	QObject::connect(rpl, &Replacer::confirm, this, &TWindow::replace);
}

bool askSure(QString text) {
	QMessageBox box(QMessageBox::Question,"Question",text,QMessageBox::Yes | QMessageBox::No);
	// int res = box.exec();
	return (box.exec() == QMessageBox::Yes);
}

void TWindow::clearTrn() {
	if (!curPage) return;

	QMessageBox box(QMessageBox::Question,"Question","Clear translation?",QMessageBox::YesToAll | QMessageBox::Yes | QMessageBox::No);
	int res = box.exec();
	if (res == QMessageBox::No) return;
	if (res == QMessageBox::YesToAll) {
		for (int i = 0; i < curPage->text.size(); i++) {
			//curPage->text[i].trn.name.clear();
			curPage->text[i].trn.text.clear();
		}
		changed = 1;
	} else if (res == QMessageBox::Yes) {
		QModelIndexList lst = ui.table->selectionModel()->selectedRows();
		QModelIndex idx;
		int row;
		foreach (idx, lst) {
			row = idx.row();
			//curPage->text[row].trn.name.clear();
			curPage->text[row].trn.text.clear();
		}
		changed = 1;
	}
	setProgress();
	model->update();
}

void TWindow::findUntrn() {
	if (!curPage) return;
	int start, state, nst;
	if (curRow < 0) {
		start = -1;
		state = LS_NONE;
	} else {
		start = curRow;
		state = getLineStatus(curPage->text.at(curRow));
	}
	for (int i = start + 1; i < curPage->text.size(); i++) {
		nst = getLineStatus(curPage->text.at(i));
		if (((nst == LS_TRN) || (nst == LS_UNTRN)) && (nst != state)) {
			ui.table->selectRow(i);
			break;
		}
	}
}

void TWindow::treeContextMenu() {
	if (curItem) {
		QUuid id(curItem->data(0, roleId).toByteArray());
		ui.actIcon->setEnabled(id.isNull());
	} else {
		ui.actIcon->setEnabled(false);
	}
	treeMenu->exec(QCursor::pos());
}

void TWindow::tbContextMenu() {
	fillSJMenu();
	tbMenu->exec(QCursor::pos());
}

void TWindow::jumpLine(QAction *act) {
	int newRow = act->data().toInt();
	if (newRow < 0) return;
	ui.table->selectRow(newRow);
}

void TWindow::fillSJMenu() {
	if (curPage) {

		sjMenu->clear();
		bmMenu->clear();

		bmMenu->addAction(ui.actAddBookmark);
		bmMenu->addAction(ui.actRmBookmark);
		bmMenu->addSeparator();

		QString txt;
		TLine* line;
		TBookmark* bm;
		int i;
		if ((curRow < 0) || (curRow >= curPage->text.size())) {
			ui.actAddBookmark->setEnabled(false);
			ui.actRmBookmark->setEnabled(false);
		} else {
			ui.actAddBookmark->setEnabled(true);
			ui.actRmBookmark->setDisabled(curPage->text[curRow].bmrkId.isNull());
		}
		for (i = 0; i < curPage->text.size(); i++) {
			line = &curPage->text[i];

			if (!line->bmrkId.isNull()) {
				bm = findBookmark(line->bmrkId);
				if (bm != nullptr) {
					bmMenu->addAction(bm->name)->setData(i);
				}
			}
			if (line->src.text.startsWith("==") || line->trn.text.startsWith("==")) {
				if (line->trn.text.isEmpty()) {
					txt = line->src.text;
				} else {
					txt = line->trn.text;
				}
				txt.remove("==");
				txt = txt.trimmed();
				if (!txt.isEmpty()) {
					txt.prepend(QString("%0 : ").arg(i));
					sjMenu->addAction(txt)->setData(i);
				}
			} else if (line->src.text.startsWith("[")) {
				txt = line->src.text;
				txt.prepend(QString("%0 : ").arg(i));
				sjMenu->addAction(txt)->setData(i);
			}
		}
		QString imgdir = getImgDir(curItem);
		ui.actRmImgDir->setVisible(!imgdir.isEmpty());
		ui.actInsertBG->setVisible(!imgdir.isEmpty());
//	} else {
//		ui.actInsertBG->setVisible(false);
	}
}

void TWindow::sortTree() {
	ui.tree->sortItems(0,Qt::AscendingOrder);
}

void TWindow::setPage(QUuid id) {
	curPage = findPage(id);
	model->update();
}

void TWindow::setProgress() {
	if (!curItem || !curPage) {
		ui.progress->setMaximum(1);
		ui.progress->setValue(0);
	} else {
		int tot,trn;
		if (curItem->data(0, roleId) == 0) {
			trn = 0;
			tot = 1;
		} else {
			getCounts(curPage,trn,tot);
			curItem->setIcon(0,getPageIcon(curPage));
		}
		ui.progress->setMaximum(tot);
		ui.progress->setValue(trn);
	}
}

QString getString(TLine line, int wut) {
	QString res;
	switch (wut) {
		case TL_SRC:
			res = line.src.text;
			if (line.src.name.isEmpty()) break;
			res.prepend("\t").prepend(line.src.name);
			break;
		case TL_TRN:
			res = line.trn.text;
			if (line.trn.name.isEmpty()) break;
			res.prepend("\t").prepend(line.trn.name);
			break;
	}
	return res;
}

int TWindow::getCurrentRow() {
	int res = -1;
	QModelIndexList idxs = ui.table->selectionModel()->selectedRows();
	if (idxs.size() == 1) res = idxs.first().row();
	return res;
}

// search

void TWindow::replace(QString fstr, QString rstr) {
	if (!curPage) return;
	if (fstr.isEmpty()) return;
	for (int i = 0; i < curPage->text.size(); i++) {
		if (curPage->text[i].src.name.contains(fstr)) {
			curPage->text[i].src.name.replace(fstr, rstr);
			changed = 1;
		}
		if (curPage->text[i].src.text.contains(fstr)) {
			curPage->text[i].src.text.replace(fstr, rstr);
			changed = 1;
		}
		if (curPage->text[i].trn.name.contains(fstr)) {
			curPage->text[i].trn.name.replace(fstr, rstr);
			changed = 1;
		}
		if (curPage->text[i].trn.text.contains(fstr)) {
			curPage->text[i].trn.text.replace(fstr, rstr);
			changed = 1;
		}
		model->updateLine(i);
	}
	changeRow(QItemSelection());
}

void TWindow::findStr(QString str) {
	ui.leFind->setStyleSheet("");
	if (!curPage) return;
	if (str.isEmpty()) return;
	int row = (curRow < 0) ? 0 : curRow;
	int match;
	int pass = 0;
	while (1) {
		if (row >= curPage->text.size()) {
			if (pass) {
				ui.leFind->setStyleSheet("QLineEdit {background-color: #ffc0c0;}");
				break;
			}
			row = 0;
			pass = 1;
		}
		match = curPage->text[row].trn.text.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].trn.name.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].src.text.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].src.name.contains(str, Qt::CaseInsensitive);
		if (match) {
			curRow = row;
			ui.table->selectRow(row);
			break;
		}
		row++;
	}
}

void TWindow::findNext() {
	if (ui.leFind->isHidden()) return;
	lineDown();
	findStr(ui.leFind->text());
}

void TWindow::findPrev() {
	ui.leFind->setStyleSheet("");
	if (ui.leFind->isHidden()) return;
	lineUp();
	QString str = ui.leFind->text();
	if (str.isEmpty()) return;
	int row = curRow - 1;
	int match;
	int pass = 0;
	while (1) {
		if (row < 0) {
			if (pass) {
				ui.leFind->setStyleSheet("QLineEdit {background-color: #ffc0c0;}");
				break;
			}
			row = model->rowCount() - 1;
			pass = 1;
		}
		match = curPage->text[row].trn.text.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].trn.name.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].src.text.contains(str, Qt::CaseInsensitive) || \
				curPage->text[row].src.name.contains(str, Qt::CaseInsensitive);
		if (match) {
			curRow = row;
			ui.table->selectRow(row);
			break;
		}
		row--;
	}
}

void TWindow::keyPress(QKeyEvent* ev) {
	keyPressEvent(ev);
}

// protected

void TWindow::keyPressEvent(QKeyEvent* ev) {
	if (ev->modifiers() & Qt::ControlModifier) {
		switch (ev->key()) {
			case Qt::Key_S:
				savePrj(prjPath);
				break;
			case Qt::Key_O:
				openPrj();
				break;
			case Qt::Key_F:
				if (ui.widFind->isHidden()) {
					ui.leFind->clear();
					ui.widFind->show();
					ui.leFind->setFocus();
				} else {
					ui.widFind->hide();
				}
				break;
			case Qt::Key_R:
				emit rqReplace();
				break;
			case Qt::Key_D:
				if (ui.srcname->isVisible()) break;
				splitName();
				break;
			case Qt::Key_Z:
				askBookmark();
				break;
			case Qt::Key_B:
				bmList();
				break;
			case Qt::Key_Delete:
				rowDelete();
				break;
		}
	} else if (ev->modifiers() & Qt::AltModifier) {
		switch (ev->key()) {
			case Qt::Key_Insert:
				rowInsert(true);
				break;
		}
	} else {
		switch (ev->key()) {
			case Qt::Key_Up:
				if (ui.leFind->isVisible()) {
					findPrev();
				} else {
					lineUp();
				}
				break;
			case Qt::Key_Down:
				if (ui.leFind->isVisible()) {
					findNext();
				} else {
					lineDown();
				}
				break;
			case Qt::Key_Escape:
				if (ui.widFind->isVisible()) {
					ui.widFind->hide();
					ui.table->setFocus();
				}
				break;
			case Qt::Key_Insert:
				rowInsert(false);
				break;
		}
	}
}

void TWindow::splitName() {
	QString tx = ui.srcline->text();
	int pos = tx.indexOf("（");
	if (pos > 0) {
		if (tx.endsWith("）")) {
			// tx.remove(tx.size() - 1, 1);
			ui.srcname->setText(tx.left(pos));
			ui.srcline->setText(tx.mid(pos));
		}
	} else {
		pos = tx.indexOf("「");
		if (pos > 0) {
			if (tx.endsWith("」")) {
				tx.remove(tx.size() - 1, 1);
				ui.srcname->setText(tx.left(pos));
				ui.srcline->setText(tx.mid(pos+1));
			}
		}
	}
	ui.srcname->setVisible(true);
	ui.trnname->setVisible(true);
}

void TWindow::lineUp() {
	do {
		curRow--;
	} while ((curRow > 0) && ui.table->isRowHidden(curRow));
	if (curRow < 0) curRow = 0;
	ui.table->selectRow(curRow);
}

void TWindow::lineDown() {
	if (curRow < 0) return;
	do {
		curRow++;
	} while ((curRow < model->rowCount()) && ui.table->isRowHidden(curRow));
	if (curRow >= model->rowCount()) {
		curRow = model->rowCount() - 1;
	}
	if (curRow < 0) return;
	ui.table->selectRow(curRow);
}

int TWindow::saveChanged() {
	if (!changed) return 1;
	int res = 0;
	QMessageBox msg(QMessageBox::Question,"Question","Save @ exit?",QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,this);
	switch (msg.exec()) {
		case QMessageBox::Yes:
			if (savePrj(prjPath)) {
				res = 1;
			} else {
				res = 0;
			}
			break;
		case QMessageBox::No:
			res = 1;
			break;
		case QMessageBox::Cancel:
			res = 0;
			break;
	}
	return res;
}

// icon

void TWindow::fillIconList() {
	icoui.list->clear();
	TIcon ico;
	QListWidgetItem* itm;
	itm = new QListWidgetItem();
	itm->setToolTip("default");
	itm->setIcon(QIcon(":/folder.png"));
	itm->setData(roleId, 0);
	icoui.list->addItem(itm);
	foreach (ico, icons) {
		itm = new QListWidgetItem();
		itm->setIcon(ico.icon);
		itm->setToolTip(ico.name);
		itm->setData(roleId, ico.id.toByteArray());
		icoui.list->addItem(itm);
	}
}

void TWindow::changeIcon() {
	fillIconList();
	icowin->show();
}

void TWindow::loadIcon() {
	QStringList paths = fdial.getOpenFileNames(icowin, "Select icon(s)","","",nullptr,QFileDialog::DontUseNativeDialog);
	if (paths.isEmpty()) return;
	QString path;
	TIcon ico;
	foreach(path, paths) {
		ico.id = QUuid::createUuid();
		ico.icon = QIcon(path);
		addIcon(ico);
	}
	changed = 1;
	fillIconList();
}

void rmIconFromTree(QTreeWidgetItem* par, QUuid icoid) {
	int cnt = par->childCount();
	QTreeWidgetItem* itm;
	QUuid id;
	for (int i = 0; i < cnt; i++) {
		itm = par->child(i);
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (id.isNull()) {
			id = QUuid(itm->data(0, roleIcon).toByteArray());
			if (id == icoid) {
				itm->setData(0, roleIcon, QUuid(0).toByteArray());
				itm->setIcon(0, QIcon(":/folder.png"));
			}
			rmIconFromTree(itm, icoid);
		}
	}
}

void TWindow::delIcon() {
	QList<QListWidgetItem*> itms = icoui.list->selectedItems();
	if (itms.size() > 0) changed = 1;
	QListWidgetItem* itm;
	QUuid id;
	foreach(itm, itms) {
		id = QUuid(itm->data(roleId).toByteArray());
		if (!id.isNull()) {
			rmIcon(id);
			rmIconFromTree(ui.tree->invisibleRootItem(), id);
		}
	}
	fillIconList();
}

void TWindow::setIcon(QListWidgetItem* itm) {
	if (!curItem) return;
	QUuid id = QUuid(itm->data(roleId).toByteArray());
	QUuid oldid = QUuid(curItem->data(0, roleIcon).toByteArray());
	if (id != oldid) {
		changed = 1;
		curItem->setData(0, roleIcon, id.toByteArray());
		curItem->setIcon(0, getIcon(id));
	}
	icowin->close();
}

// bookmark

void TWindow::askBookmark() {
	if (!curPage) return;
	if (curRow < 0) return;
	QUuid id = curPage->text.at(curRow).bmrkId;
	TBookmark* bm = findBookmark(id);
	if (!bm) {
		bmui.leid->clear();
		bmui.lename->clear();
		bmui.tedescr->clear();
	} else {
		bmui.leid->setText(bm->id.toString());
		bmui.lename->setText(bm->name);
		bmui.tedescr->setPlainText(bm->descr);
	}
	bmui.lepage->setText(curPage->id.toString());
	bmui.leline->setText(QString::number(curRow));
	bmwin->show();
}

void TWindow::newBookmark() {
	TBookmark bm;
	bm.id = QUuid::fromString(bmui.leid->text());
	bm.name = bmui.lename->text();
	bm.descr = bmui.tedescr->toPlainText();
	bm.pgid = curPage->id;
	bm.row = curPage->curRow;
	if (bm.name.isEmpty()) return;
	bmwin->close();
	curPage->text[curRow].bmrkId = addBookmark(bm);
	model->updateLine(curRow);
	changed = 1;
}

void TWindow::askRmBookmark() {
	if (!curPage) return;
	if (curRow < 0) return;
	QUuid id = curPage->text.at(curRow).bmrkId;
	if (id.isNull()) return;
	rmBookmark(id);
	curPage->text[curRow].bmrkId = QUuid(0);
	model->updateCell(curRow, 0);
	changed = 1;
}

void TWindow::bmList() {
	blmod->update();
	blwin->show();
}

QTreeWidgetItem* searchItem(QTreeWidgetItem* par, QUuid id) {
	QTreeWidgetItem* res = nullptr;
	QTreeWidgetItem* itm;
	QUuid uid;
	int i;
	for (i = 0; (i < par->childCount()) && (res == nullptr); i++) {
		itm = par->child(i);
		uid = itm->data(0, roleId).toUuid();
		if (uid.isNull()) {
			res = searchItem(itm, id);
		} else if (itm->data(0, roleId).toUuid() == id) {
			res = itm;
		}
	}
	return res;
}

int TWindow::selectItemByPageID(QUuid id) {
	QTreeWidgetItem* itm = searchItem(ui.tree->invisibleRootItem(), id);
	int res = 0;
	if (itm != nullptr) {
		ui.tree->setCurrentItem(itm);
		res = 1;
	}
	return res;
}

void TWindow::goToBookmark(const QModelIndex& idx) {
	int row = idx.row();
	TBookmark bm = bookmarks.at(row);
	TPage* pg = findPage(bm.pgid);
	if (selectItemByPageID(bm.pgid)) {
		pg->curRow = bm.row;
		curRow = bm.row;
		ui.table->selectRow(bm.row);
		ui.table->scrollTo(ui.table->model()->index(bm.row, 0));
	}
	blwin->close();
}

// page

void TWindow::delPage() {
	QList<QTreeWidgetItem*> items = ui.tree->selectedItems();
	if (items.isEmpty()) return;
	if (!askSure("Delete selected element(s)?")) return;
	ui.tree->selectionModel()->clear();
	foreach(QTreeWidgetItem* itm, items) {
		delItem(itm);
	}
}

void TWindow::mergePages() {
	QList<QTreeWidgetItem*> items = ui.tree->selectedItems();
	if (items.size() < 2) return;
	QUuid pid(items.first()->data(0, roleId).toByteArray());
	if (pid.isNull()) return;
	TPage* par = findPage(pid);
	TPage* pg;
	TLine elin, lin;
	elin.type = TL_TEXT;
	lin.type = TL_TEXT;
	lin.flag = 0;
	elin.flag = 0;
	items.removeFirst();
	QTreeWidgetItem* itm;
	QUuid id;
	foreach(itm, items) {
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (!id.isNull()) {
			pg = findPage(id);
			if (pg) {
				lin.src.text = QString("== Joined : %0").arg(itm->text(0));
				par->text.append(elin);
				par->text.append(lin);
				par->text.append(elin);
				par->text.append(pg->text);
			}
		}
	}
	foreach(itm, items) {
		delItem(itm);
	}
	model->update();
	setProgress();
}

void TWindow::delItem(QTreeWidgetItem* item) {
	QUuid id(item->data(0, roleId).toByteArray());
	if (id.isNull()) {
		for (int i = 0; i < item->childCount(); i++) {
			delItem(item->child(i));
		}
	} else {
		removePage(id);
	}
	QTreeWidgetItem* par = item->parent();
	if (par == nullptr) par = ui.tree->invisibleRootItem();
	if (par->indexOfChild(item) >= 0)
		par->removeChild(item);
	changed = 1;
}

void TWindow::pageInfo() {
	if (curItem == nullptr) return;		// never?
	if (curPage == nullptr) return;
	if (curPage->id.isNull()) return;	// dir
}

void TWindow::pageSplit() {
	if (curItem == nullptr) return;
	int idx = getCurrentRow();
	if (idx < 0) return;
	if (idx >= ui.table->model()->rowCount()) return;
	QList<TLine> tx1 = curPage->text.mid(0,idx);
	QList<TLine> tx2 = curPage->text.mid(idx);
	TPage* newp = newPage();
	newp->text = tx2;
	curPage->text = tx1;
	model->update();
	setProgress();
}

// pix

// rows actions

bool checkOrder(QModelIndex& idx1, QModelIndex& idx2) {
	return idx1.row() > idx2.row();
}

void TWindow::rowDelete() {
	if (!ui.table->isEnabled()) return;
	QModelIndexList list = ui.table->selectionModel()->selectedRows();
	if (list.size() == 0) return;
	std::sort(list.begin(), list.end(), checkOrder);
	int row;
	QUuid id;
	int zRow = list.last().row();
	foreach(QModelIndex idx, list) {
		row = idx.row();
		id = curPage->text.at(row).bmrkId;
		if (!id.isNull()) {
			rmBookmark(id);
		}
		curPage->text.removeAt(row);
		model->removeRow(row);
	}
	curRow = zRow;
	setProgress();
	ui.table->selectRow(curRow);
	changed = 1;
}

void TWindow::rowInsert(bool before) {
	if (!ui.table->isEnabled()) return;
	TLine line;
	line.type = TL_TEXT;
	line.flag = 0;
	int row;
	if (curRow < 0) {
		row = 0;
	} else if (before) {
		row = curRow;
	} else {
		row = curRow + 1;
	}
	curPage->text.insert(row,line);
	model->insertRow(row);
	setProgress();
	changed = 1;
}

void TWindow::rowInsertAbove() {rowInsert(true);}
void TWindow::rowInsertBelow() {rowInsert(false);}

void TWindow::joinLine() {
	if (!curPage) return;
	if (curRow < 0) return;
	if (curRow > curPage->text.size() - 2) return;
	TLine tlin = curPage->text[curRow];
	TLine nlin = curPage->text[curRow + 1];
	tlin.src.text.append(nlin.src.text);
	tlin.trn.text.append(nlin.trn.text);
	curPage->text[curRow] = tlin;
	curPage->text.removeAt(curRow + 1);
	model->update();
	ui.table->selectRow(curRow);
}

void TWindow::splitLine() {
	if (!curPage) return;
	if (curRow < 0) return;
	QString oldtxt = ui.srcline->text();
	int pos = ui.srcline->cursorPosition();
	if (pos < 0) return;
	rowInsert(false);
	curPage->text[curRow].src.text = oldtxt.left(pos);
	curPage->text[curRow+1].src.text = oldtxt.mid(pos);
	ui.srcline->setText(curPage->text[curRow].src.text);
	model->updateLine(curRow);
	model->updateLine(curRow+1);
	ui.table->selectRow(curRow);
	setProgress();
}

// edit block

void TWindow::setEdit(bool on) {
	ui.editGrid->setEnabled(on);
	if (!on) {
		ui.srcname->clear();
		ui.srcline->clear();
		ui.trnname->clear();
		ui.trnline->clear();
	}
}

void TWindow::disableTab() {
	curPage = nullptr;
	curRow = -1;
	ui.tabs->setEnabled(false);
	setEdit(false);
	ui.table->update();
}

void TWindow::treeItemChanged(QTreeWidgetItem* itm) {
	QUuid id = itm->data(0, roleId).toUuid();
	TPage* pg;
	if (!id.isNull()) {
		pg = findPage(id);
		if (pg != NULL) {
			pg->name = itm->text(0);
			itm->setToolTip(0, pg->name);
		}
	}
}

void TWindow::changePage() {
	if (curPage) {
		// curPage->curRow = ui.table->currentIndex().row();
		curPage = nullptr;
	}
	if (ui.tree->selectedItems().size() != 1) {
		disableTab();
	} else {
		QTreeWidgetItem* itm = ui.tree->currentItem();
		if (!itm) {
			disableTab();
		} else {
			curItem = itm;
			QUuid id(itm->data(0, roleId).toByteArray());
			if (id != 0) {
				if (!curPage || (curPage->id != id)) {
					setPage(id);
					ui.tabs->setEnabled(true);
					setEdit(true);
					ui.table->scrollToTop();
					ui.table->selectionModel()->clear();
					setEdit(false);
				}
			} else {
				disableTab();
			}
		}
	}
	model->update();
	setProgress();
	if (curPage) {
		curRow = curPage->curRow;
		if (curRow > -1) {
			ui.table->selectRow(curRow);
			ui.table->scrollTo(ui.table->model()->index(curRow, 0));
		}
	}
	ui.widFind->hide();
	fillSJMenu();
}

void TWindow::changeRow(QItemSelection) {
	int row = getCurrentRow();
	curRow = row;
	curPage->curRow = row;
	QString text;
	if (row < 0) {
		setEdit(false);
		ui.labInfo->clear();
	} else {
		if ((curPage->text[row].type == TL_TEXT) || (curPage->text[row].type == TL_SELECT)) {
			ui.srcname->setText(curPage->text[row].src.name);
			ui.srcline->setText(curPage->text[row].src.text);
			ui.trnname->setText(curPage->text[row].trn.name);
			ui.trnline->setText(curPage->text[row].trn.text);
			ui.srcline->setCursorPosition(0);
			ui.trnline->setCursorPosition(0);
			text = ui.srcline->text();
			setEdit(true);
			if (curPage->text[row].src.name.isEmpty() && curPage->text[row].trn.name.isEmpty()) {
				ui.trnname->setVisible(false);
				ui.srcname->setVisible(false);
				if (ui.trnname->hasFocus()) ui.trnline->setFocus();
			} else {
				ui.trnname->setVisible(true);
				ui.srcname->setVisible(true);
				if (ui.trnline->hasFocus()) ui.trnname->setFocus();
			}
			if (!ui.srcname->text().isEmpty()) {
				text.prepend("「").prepend(ui.srcname->text()).append("」");
			}
			if (!ui.actGrabCbrd->isChecked() && !ui.tbRec->isChecked()) {
				emit textChanged(text.remove(" "));
				// clip->setText(text.remove(" "));
			}
			ui.labInfo->setText(QString("%0 / %1").arg(curRow+1).arg(curPage->text.size()));
		} else {
			setEdit(false);
			ui.labInfo->clear();
		}
	}
}

void TWindow::changeSrc(QString text) {
	if (!ui.editGrid->isEnabled()) return;
	if (!curPage || (curRow < 0)) return;
	if (curPage->text[curRow].src.text == text) return;
	QString oldtext = curPage->text[curRow].src.text;
	curPage->text[curRow].src.text = text;
	model->updateCell(curRow,2);
	if (oldtext.isEmpty() ^ text.isEmpty()) {
		setProgress();
	}
	changed = 1;
}

void TWindow::changeTrn(QString text) {
	if (!ui.editGrid->isEnabled()) return;
	if (!curPage || (curRow < 0)) return;
	if (curPage->text[curRow].trn.text == text) return;
	QString oldtext = curPage->text[curRow].trn.text;
	curPage->text[curRow].trn.text = text;
	model->updateCell(curRow,4);
	if (oldtext.isEmpty() ^ text.isEmpty()) {
		setProgress();
	}
	changed = 1;
}

void TWindow::changeSNm(QString text) {
	if (!ui.editGrid->isEnabled()) return;
	if (!curPage || (curRow < 0)) return;
	if (curPage->text[curRow].src.name == text) return;
	curPage->text[curRow].src.name = text;
	model->updateCell(curRow,1);
	// setProgress();
	changed = 1;
}

void TWindow::changeTNm(QString text) {
	if (!ui.editGrid->isEnabled()) return;
	if (!curPage || (curRow < 0)) return;
	if (curPage->text[curRow].trn.name == text) return;
	// curPage->text[curRow].trn.name = text;
	// model->updateCell(curRow,3);

	QString srcNm = curPage->text[curRow].src.name;
	for (int i = 0; i < curPage->text.size(); i++) {
		if ((curPage->text[i].src.name == srcNm) && (curPage->text[i].trn.name != text)) {
			curPage->text[i].trn.name = text;
			model->updateCell(i, 3);
		}
	}
	// setProgress();
	changed = 1;
}

// menu

void TWindow::newPrj() {
	prjInit();
	disableTab();
	ui.tree->clear();
	prjPath = "";
}

// load-save

/*
void idError(int type, int subtype) {
	QMessageBox msg(QMessageBox::Critical, QString("Error"), QString(), QMessageBox::Ok);
	switch(type) {
		case T7_ICON:
			msg.setText(QString("Unknown ICON id %0").arg(subtype));
			break;
		case T7_PAGE:
			msg.setText(QString("Unknown PAGE id %0").arg(subtype));
			break;
		case T7_TREE:
			msg.setText(QString("Unknown TREE id %0").arg(subtype));
			break;
		case TP_LINE:
			msg.setText(QString("Unknown LINE id %0").arg(subtype));
			break;
		case TT_DIR:
			msg.setText(QString("Unknown TREE.DIR id %0").arg(subtype));
			break;
		case TT_PAGE:
			msg.setText(QString("Unknown TREE.PAGE id %0").arg(subtype));
			break;
		default:
			msg.setText(QString("Unknown GLOBAL id %0:%1").arg(type).arg(subtype));
			break;
	}
	msg.exec();
}
*/

#if !NEW_LOADER
void idError(int type, int subtype);
void MWindow::loadVer78(QByteArray& data, QTreeWidgetItem* par) {
	QUuid id;
	QUuid iconid;
	QIcon icon;
	int oldid;
	int type;
	TIcon ico;
	TBookmark bm;
	TPage page;
	page.curRow = -1;
	TLine lin;
	QString name;
	QBuffer buf;
	QDataStream strm;
	QMap<int, QUuid> map;
	map.clear();
	data.remove(0,4);		// header (TRB7)
	data = qUncompress(data);	// unpack data
	buf.setBuffer(&data);
	buf.open(QIODevice::ReadOnly);
	strm.setDevice(&buf);
	strm >> type;
	int err = 0;
	while (!strm.atEnd() && (type != T7_END)) {
		switch(type) {
			case T7_ICON:
				strm >> type;
				while (type != T7_END) {
					switch (type) {
						case TI_ID: strm >> ico.id; break;
						case TI_ICO: strm >> ico.icon; break;
						case TI_NAME: strm >> ico.name; break;
						case TI_END: addIcon(ico); break;
						default:
							idError(T7_ICON, type);
							err = 1;
							break;
					}
					if (err) break;
					strm >> type;
				}
				break;
			case T7_BMRK:
				strm >> type;
				while (type != T7_END) {
					switch(type) {
						case TB_ID: strm >> bm.id; break;
						case TB_NAME: strm >> bm.name; break;
						case TB_DSC: strm >> bm.descr; break;
						case TB_END: addBookmark(bm); break;
						default:
							idError(T7_BMRK, type);
							err = 1;
							break;
					}
					if (err) break;
					strm >> type;
				}
				break;
			case T7_PAGE:
				page.text.clear();
				strm >> type;
				while (type != T7_END) {
					switch(type) {
						case TP_ID:
							strm >> oldid;
							if (!map.contains(oldid)) {
								map[oldid] = QUuid::createUuid();
							}
							page.id = map[oldid];
							break;
						case TP_UUID:
							strm >> page.id;
							break;
						case TP_FLAG:
							strm >> page.flag;
							break;
						case TP_LINE:
							lin.type = TL_TEXT;
							lin.flag = 0;
							lin.bmrkId = QUuid();
							lin.src.name.clear();
							lin.src.text.clear();
							lin.trn.name.clear();
							lin.trn.text.clear();
							strm >> type;
							while (type != T7_END) {
								switch(type) {
									case TL_SN: strm >> lin.src.name; break;
									case TL_ST: strm >> lin.src.text; break;
									case TL_TN: strm >> lin.trn.name; break;
									case TL_TT: strm >> lin.trn.text; break;
									case TL_TYPE: strm >> lin.type; break;
									case TL_FLAG: strm >> lin.flag; break;
									case TL_BMID: strm >> lin.bmrkId; break;
									default:
										idError(TP_LINE, type);
										err = 1;
										break;
								}
								if (err) break;
								strm >> type;
							}
							if (err) break;
							lin.src.text.remove(QObject::trUtf8("　"));
							normLine(lin);
							page.text.append(lin);
							break;
						default:
							idError(T7_PAGE, type);
							err = 1;
							break;
					}
					if (err) break;
					strm >> type;
				}
				if (err) break;
				putPage(page);
				break;
			case T7_TREE:
				strm >> type;
				while (type != T7_END) {
					switch (type) {
						case TT_DIR:
							strm >> type;
							while (type != TT_END) {
								switch(type) {
									case TT_NAME:
										strm >> name;
										break;
									case TT_ICON:
										strm >> icon;
										break;
									case TT_ICONID:
										strm >> iconid;
										break;
									default:
										idError(TT_DIR, type);
										err = 1;
										break;
								}
								if (err) break;
								strm >> type;
							}
							if (err) break;
							par = addItem(par,name,0,iconid);
							iconid = 0;
							break;
						case TT_PAGE:
							strm >> type;
							while (type != TT_END) {
								switch(type) {
									case TT_NAME:
										strm >> name;
										break;
									case TT_PID:
										strm >> oldid;
										if (!map.contains(oldid)) {
											map[oldid] = QUuid::createUuid();
										}
										id = map[oldid];
										break;
									case TT_UUID:
										strm >> id;
										break;
									case TT_ICON:
										strm >> icon;
										break;
									default:
										idError(TT_PAGE, type);
										err = 1;
										break;
								}
								if (err) break;
								strm >> type;
							}
							if (err) break;
							addItem(par,name,id);
							break;
						case TT_END:
							par = par->parent();
							if (par == nullptr) par = ui.tree->invisibleRootItem();
							break;
						default:
							idError(T7_TREE, type);
							err = 1;
							break;
					}
					if (err) break;
					strm >> type;
				}
				break;
			default:
				idError(type, 0);
				err = 1;
				break;
		}
		if (err) break;
		strm >> type;
	}
	if (err) {
		prjInit();
	}
}

QByteArray loadPrjData(QString path) {
	QByteArray arr;
	QFile file(path);
	QMessageBox box(QMessageBox::Critical, "Error", "", QMessageBox::Ok);
	if (file.open(QFile::ReadOnly)) {
		arr = file.readAll();
		file.close();
		if (QDialog::trUtf8(arr.left(3)) != "TRB") {
			box.setText("Signature error");
			box.exec();
			arr.clear();
		}
	} else {
		box.setText("Can't open file");
		box.exec();
	}
	return arr;
}
#endif

void TWindow::openPrj(QString path) {
	if (!saveChanged()) return;
	if (path == "") path = fdial.getOpenFileName(this,"Open book",opt.value("lastdir","").toString(),"Book files (*.trb)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path == "") return;
	opt.setValue("lastdir",QFileInfo(path).dir().absolutePath());
#if NEW_LOADER
	prjInit();
	ui.tree->clear();
	if (trb.load(path, ui.tree->invisibleRootItem())) {
		prjPath = path;
		changed = 0;
	}
#else
	QByteArray data = loadPrjData(path);
	if (!data.isEmpty()) {
		prjPath = path;
		ui.tree->clear();
		book.clear();
		icons.clear();
		bookmarks.clear();
		QTreeWidgetItem* par = ui.tree->invisibleRootItem();
		if (data.left(4) == "TRB7") {
			prjInit();
			loadVer78(data, par);
			changed = 0;
//		} else if (data.left(4) == "TRB8") {
//			prjInit();
//			loadVer8(data, par);
//			changed = 0;
		} else {
			prjPath.clear();
			QMessageBox::critical(this, "Error", "TRB version mismatch");
		}
	}
#endif
}

void TWindow::mergePrj(QString path) {
	if (path == "") path = fdial.getOpenFileName(this,"Open book",opt.value("lastdir","").toString(),"Book files (*.trb)",nullptr,QFileDialog::DontUseNativeDialog);
	if (path == "") return;
	opt.setValue("lastdir",QFileInfo(path).dir().absolutePath());
#if NEW_LOADER
	trb.load(path, getCurrentParent());
	changed = 1;
#else
	QByteArray data = loadPrjData(path);
	if (!data.isEmpty()) {
		QTreeWidgetItem* par = getCurrentParent();
		if (data.left(4) == "TRB7") {
			loadVer78(data, par);
			changed = 1;
//		} else if (data.left(4) == "TRB8") {
//			loadVer8(data, par);
//			changed = 1;
		} else {
			QMessageBox::critical(this, "Error", "TRB version mismatch");
		}
	}
#endif
}

void TWindow::saveIt() {
	savePrj(prjPath);
}

#if !NEW_SAVER

void saveLeaf7(QDataStream& strm, QTreeWidgetItem* par) {
	int i;
	QUuid id;
	QTreeWidgetItem* itm;
	for (i = 0; i < par->childCount(); i++) {
		itm = par->child(i);
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (id.isNull()) {
			strm << TT_DIR;
			strm << TT_NAME << itm->text(0);
			strm << TT_ICONID << QUuid(itm->data(0,roleIcon).toByteArray());
			strm << TT_END;
			saveLeaf7(strm, itm);
		} else {
			strm << TT_PAGE;
			strm << TT_NAME << itm->text(0);
			strm << TT_UUID << id;
			strm << TT_END;
		}
	}
	strm << TT_END;
}

void savePage(QDataStream& strm, QUuid id) {
	TPage* page = findPage(id);
	TLine line;
	if (!page) return;
	strm << T7_PAGE;
	strm << TP_UUID << page->id;
	strm << TP_FLAG << page->flag;
	foreach(line, page->text) {
		strm << TP_LINE;
		strm << TL_TYPE << line.type;
		strm << TL_FLAG << line.flag;
		strm << TL_BMID << line.bmrkId;
		strm << TL_SN << line.src.name;
		strm << TL_ST << line.src.text;
		strm << TL_TN << line.trn.name;
		strm << TL_TT << line.trn.text;
		strm << T7_END;
	}
	strm << T7_END;
}

void saveTree(QDataStream& strm, QTreeWidgetItem* root, int saveroot = 0) {
	strm << T7_TREE;
	if (saveroot) {
		strm << TT_DIR;
		strm << TT_NAME << root->text(0);
		strm << TT_ICONID << QUuid(root->data(0,roleIcon).toByteArray());
		strm << TT_END;
	}
	saveLeaf7(strm, root);
	strm << T7_END;
}

#endif

QList<QUuid> getTreeIds(QTreeWidgetItem* root) {
	QList<QUuid> res;
	QTreeWidgetItem* itm;
	QUuid id;
	for (int i = 0; i < root->childCount(); i++) {
		itm = root->child(i);
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (id.isNull()) {
			res.append(getTreeIds(itm));
		} else {
			res.append(id);
		}
	}
	return res;
}

QList<QUuid> getPageBMrk(QUuid id) {
	QList<QUuid> res;
	TPage* page = findPage(id);
	TLine ln;
	if (page) {
		foreach(ln, page->text) {
			if (!ln.bmrkId.isNull())
				res.append(ln.bmrkId);
		}
	}
	return res;
}

QList<QUuid> getTreeBMrk(QTreeWidgetItem* root) {
	QList<QUuid> res;
	QTreeWidgetItem* itm;
	QUuid id;
	for (int i = 0; i < root->childCount(); i++) {
		itm = root->child(i);
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (id.isNull()) {
			res.append(getTreeBMrk(itm));
		} else {
			res.append(getPageBMrk(id));
		}
	}
	return res;
}

QList<QUuid> getTreeIcons(QTreeWidgetItem* root) {
	QList<QUuid> res;
	QTreeWidgetItem* itm;
	QUuid id;
	for (int i = 0; i < root->childCount(); i++) {
		itm = root->child(i);
		id = QUuid(itm->data(0, roleIcon).toByteArray());
		if (!id.isNull())
			res.append(id);
		id = QUuid(itm->data(0, roleId).toByteArray());
		if (id.isNull())
			res.append(getTreeIcons(itm));
	}
	return res;
}

// TODO:save current folder
void TWindow::saveBranch() {
	if (!curItem) return;
	QString tmpath = prjPath;
	int tmpcha = changed;
	savePrj(QString(""), curItem);
	prjPath = tmpath;
	changed = tmpcha;
}

bool TWindow::savePrj(QString path, QTreeWidgetItem* root) {
#if NEW_SAVER
	if (path.isEmpty())
		path = fdial.getSaveFileName(this,"Save book",opt.value("lastdir","").toString(),"Book files (*.trb)", nullptr, QFileDialog::DontUseNativeDialog);		// prjPath
	if (path.isEmpty())
		return false;
	if (!path.endsWith(".trb",Qt::CaseInsensitive))
		path.append(".trb");
	opt.setValue("lastdir",QFileInfo(path).dir().absolutePath());
	if (!root)
		root = ui.tree->invisibleRootItem();
	trb.save(path, root);
	prjPath = path;
	changed = 0;
	return true;
#else
	QByteArray data;	// all data
	QBuffer buf;
	QDataStream strm;
	QList<QUuid> idlist;
	QUuid id;
	TIcon* ico;
	TBookmark* bm;
	int saveroot = root ? 1 : 0;

	if (!root)
		root = ui.tree->invisibleRootItem();

	buf.setBuffer(&data);
	buf.open(QIODevice::WriteOnly);

//	TPage page;
	strm.setDevice(&buf);

// collect pages for root
	idlist = getTreeIds(root);
	foreach(id, idlist) {
		savePage(strm, id);
	}
// collect icons for root
	strm << T7_ICON;
	idlist = getTreeIcons(root);
	qDebug() << idlist.size() << "icons";
	foreach(id, idlist) {
		ico = findIcon(id);
		if (ico) {
			strm << TI_ID << ico->id;
			strm << TI_NAME << ico->name;
			strm << TI_ICO << ico->icon;
			strm << TI_END;
		}
	}
	strm << T7_END;
// collect bookmarks for root
	idlist = getTreeBMrk(root);
	strm << T7_BMRK;
	foreach(id, idlist) {
		bm = findBookmark(id);
		if (bm) {
			strm << TB_ID << bm->id;
			strm << TB_NAME << bm->name;
			strm << TB_DSC << bm->descr;
			strm << TB_END;
		}
	}
	strm << T7_END;

// save tree from root
	saveTree(strm, root, saveroot);

	buf.close();
	data = qCompress(data);

	if (path.isEmpty())
		path = fdial.getSaveFileName(this,"Save book","","Book files (*.trb)", nullptr, QFileDialog::DontUseNativeDialog);		// prjPath
	if (path.isEmpty())
		return false;
	if (!path.endsWith(".trb",Qt::CaseInsensitive))
		path.append(".trb");
	QFile file(path);
	if (!file.open(QFile::WriteOnly)) return false;
	file.write(QString("TRB7").toUtf8());	// signature
	file.write(data);
	file.close();
	prjPath = path;
	changed = 0;
	return true;
#endif
}

void TWindow::saveSrc() {
	if (curPage == nullptr) return;
	QString path = fdial.getSaveFileName(this,"Save src text","","All files (*)");
	if (path == "") return;
	QFile file(path);
	if (file.open(QFile::WriteOnly)) {
		for (int i = 0; i < curPage->text.size(); i++) {
			if (curPage->text.at(i).src.name.isEmpty()) {
				file.write(curPage->text.at(i).src.text.toUtf8());
			} else {
				file.write(curPage->text.at(i).src.name.toUtf8());
				file.write("「");
				file.write(curPage->text.at(i).src.text.toUtf8());
				file.write("」");
			}
			file.write("\r\n");
		}
		file.close();
	}
}

QList<TPage> openFiles(QFileDialog::FileMode mode) {
	QList<TPage> res;
	QFileDialog qfd;
	QStringList filters;
	qfd.setOption(QFileDialog::DontUseNativeDialog, true);
	filters << "Text files (*)"
		<< "EAGLS script(*.txt)"
		<< "KS files (*.ks)"
		<< "KS files UCS2 (*.ks)"
		<< "KS files UTF8 (*.ks)"
		<< "Abelsoft script ADV (*.adv)"
		<< "Enmon script ENM (*.enm)"
		<< "SNX engine (*.snx)";
	qfd.setNameFilters(filters);
	qfd.setWindowTitle("Open sources");
	qfd.setFileMode(mode);
	if (!qfd.exec())
		return res;
	QStringList paths = qfd.selectedFiles();
	QString path = qfd.selectedNameFilter();
	TPage page;
	int cpage = CP_SJIS;
	if (path.contains("UCS2")) cpage = CP_UCS2;
	if (path.contains("UTF8")) cpage = CP_UTF8;
	TPage(*callback)(QString,int) = nullptr;
	if (path.contains("Abelsoft")) callback = &loadAbelsoft;
	if (path.contains("Enmon")) callback = &loadEnmon;
	if (path.contains("KS files")) callback = &loadKS;
	if (path.contains("EAGLS")) callback = &loadEAGLS;
	if (path.contains("SNX")) callback = &loadSNX;
	foreach(path, paths) {
		if (callback) {
			page = callback(path, cpage);
		} else {
			page = loadPage(path, TL_SRC);
		}
		if (page.text.size() != 0) {
			page.id = QUuid::createUuid();
			page.name = QFileInfo(path).baseName();
			res.append(page);
		}
	}
	return res;
}

void TWindow::insertSrc() {
	if (!ui.editGrid->isEnabled()) return;
	if (!curPage || (curRow < 0)) return;
	QList<TPage> pages = openFiles(QFileDialog::ExistingFile);
	if (pages.size() != 1) return;
	TPage page = pages.first();
	int row = curRow;
	TLine lin;
	foreach(lin, page.text) {
		curPage->text.insert(row, lin);
		model->insertRow(row);
		row++;
	}
	setProgress();
}

void TWindow::openSrc() {
	QList<TPage> pages = openFiles(QFileDialog::ExistingFiles);
	if (pages.size() == 0) return;
	TPage page;
	TPage* pg;
	QTreeWidgetItem* par = getCurrentParent();
	foreach(page, pages) {
		pg = addPage(page);
		addItem(par, page.name, pg->id);
	}
}

QTreeWidgetItem* TWindow::getCurrentParent() {
	QModelIndexList rws = ui.tree->selectionModel()->selectedRows(0);
	QTreeWidgetItem* par = (rws.size() == 1) ? ui.tree->currentItem() : nullptr;
	QUuid id;
	if (par) {
		id = QUuid(par->data(0, roleId).toByteArray());
		if (!id.isNull())
			par = par->parent();
	}
	return par;
}

void TWindow::newDir() {
//	if (name.isEmpty()) name = QTime::currentTime().toString("hhmmss");
	QTreeWidgetItem* par = getCurrentParent();
	addItem(par,QUuid::createUuid().toString(),0);
}

// pages

TPage* TWindow::newPage() {
	TPage* pg = createPage();
	QTreeWidgetItem* par = getCurrentParent();
	addItem(par,"",pg->id);
	return pg;
}

QTreeWidgetItem* TWindow::addItem(QTreeWidgetItem* par, QString nam, QUuid id, QUuid iconid) {
	TPage* page;
	QString tip;
	QIcon ico;
	QTreeWidgetItem* itm = new QTreeWidgetItem();
	itm->setData(0, roleId, id.toByteArray());
	if (nam == "") {
		nam = id.toString();
	}
	itm->setText(0,nam);
	itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable); // | Qt::ItemIsUserCheckable);
	if (id.isNull()) {
		itm->setFlags(itm->flags() | Qt::ItemIsDropEnabled);
		ico = getIcon(iconid);
		itm->setIcon(0, ico.isNull() ? QIcon(":/folder.png") : ico);
		itm->setData(0, roleIcon, iconid.toByteArray());
	} else {
		page = findPage(id);
		if (page) {
			//tip = QString("id : %0").arg(page->id.toString());
			itm->setToolTip(0, nam);
			itm->setIcon(0, getPageIcon(page));
		}
	}
	if (par == nullptr) par = ui.tree->invisibleRootItem();
	int idx = par->indexOfChild(curItem);
	if (idx < 0) {
		par->addChild(itm);
	} else {
		par->insertChild(idx+1, itm);
	}
	changed = 1;
	return itm;
}

// images

void TWindow::setImgDir() {
	if (curItem == nullptr) return;
	QString imgdir = curItem->data(0, roleImgDir).toString();
	QString path = QFileDialog::getExistingDirectory(this, "Select images directory", imgdir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) return;
	curItem->setData(0, roleImgDir, path);
	ui.actRmImgDir->setVisible(true);
//	curPage->imgdir = path;
	changed = 1;
}

void TWindow::rmImgDir() {
	if (curItem == nullptr) return;
	curItem->setData(0, roleImgDir, "");
	ui.actRmImgDir->setVisible(false);
}

void TWindow::insertImgLine() {
	if (curPage == nullptr) return;
	QString imgdir = getImgDir(curItem);
	if (imgdir.isEmpty()) return;
	ftw->setDir(imgdir);
	ftw->show();
}

void TWindow::insertImgText(QString rpath) {
	TLine lin;
	int pos = rpath.lastIndexOf(".");
	if (pos > 0) rpath = rpath.left(pos);
	lin.type = TL_TEXT;
	lin.src.text = QString("[%0]").arg(rpath);
	curPage->text.insert(curRow, lin);
	model->insertRow(curRow);
	setProgress();
	changed = 1;
}

// files tree

xFileTreeWidget::xFileTreeWidget(QWidget* p):QWidget(p) {
	QHBoxLayout* lay = new QHBoxLayout;
	tree = new QTreeView;
	view = new QLabel;
	lay->addWidget(tree, 10);
	lay->addWidget(view);
	setLayout(lay);
	model = new QFileSystemModel;
	model->setNameFilters(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif");
	model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
	model->setNameFilterDisables(true);
	model->setReadOnly(true);
	tree->setModel(model);
	tree->setColumnHidden(1, true);
	tree->setColumnHidden(2, true);
	tree->setColumnHidden(3, true);
	tree->setMinimumWidth(400);
	view->setMinimumWidth(400);
	tree->setSelectionBehavior(QTreeView::SelectRows);
	tree->setSelectionMode(QTreeView::SingleSelection);
	tree->setHeaderHidden(true);
	setWindowTitle("Select image file");

	connect(tree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &xFileTreeWidget::itemClick);
	connect(tree, &QTreeView::doubleClicked, this, &xFileTreeWidget::itemChosed);
}

void xFileTreeWidget::setDir(QString dir) {
	model->setRootPath(dir);
	tree->setRootIndex(model->index(dir));
}

void xFileTreeWidget::itemClick(const QModelIndex& idx, const QModelIndex&) {
	QFileInfo inf = model->fileInfo(idx);
	view->setPixmap(QPixmap(inf.filePath()).scaled(400, 300, Qt::KeepAspectRatio));
}

void xFileTreeWidget::itemChosed(const QModelIndex& idx) {
	QFileInfo inf = model->fileInfo(idx);
	// QString relpath = model->rootDirectory().relativeFilePath(inf.filePath());
	QString relpath = inf.fileName();
	emit s_selected(relpath);
	close();
}

// cbrd

void TWindow::cbrdChanged() {
	if (clip->property("busy").toInt()) return;
	if (!ui.tbRec->isChecked()) return;
	if (!curPage) return;
	if (!clip->mimeData()->hasText()) return;
	QString txt = clip->text();
	if (txt.isEmpty()) return;
	clip->setProperty("busy", 1);
	clip->clear();
	TLine lin;
	int posa = txt.indexOf("「");
	int posb = txt.indexOf("（");
	if (posa >= 0) {
		lin.src.name = txt.left(posa);
		lin.src.text = txt.mid(posa + 1);
		lin.src.text.remove("」");
	} else if (posb >= 0) {
		lin.src.name = txt.left(posb);
		lin.src.text = txt.mid(posb);
	} else {
		lin.src.name.clear();
		lin.src.text = txt;
	}
	if (!lin.src.name.isEmpty() && ui.actNameRepeat->isChecked()) {
		posa = lin.src.name.size();
		posb = posa >> 1;
		if (!(posa & 1) && (lin.src.name.left(posb) == lin.src.name.right(posb))) {
			lin.src.name = lin.src.name.left(posb);
		}
	}
	lin.type = TL_TEXT;
	curPage->text.append(lin);
	model->insertRow(curPage->text.size() - 1);
	ui.table->scrollToBottom();
	setProgress();
	clip->setProperty("busy", 0);
}

// player

QStringList fExistsR(QString imgdir, QString str) {
	QStringList flt;
	QStringList lst;
	flt << str;
	flt << str + ".jpg";
	flt << str + ".png";
	flt << str + ".bmp";
	flt << str + ".gif";
	QDirIterator it(imgdir, flt, QDir::NoFilter, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		lst << it.next();
	}
	return lst;
}

void fillImages(TPage* pg, QString imgdir) {
	if (pg == nullptr) return;
	QString img;
	int cnt = pg->text.size();
	int i;
	QString txt;
	QStringList ximglist;
	for (i = 0; i < cnt; i++) {
		txt = pg->text[i].src.text;
		txt.remove(" ");
		if (imgdir.isEmpty()) {
			pg->text[i].imgpath.clear();
		} else if (txt.startsWith("[") && !(pg->text[i].flag & TF_SELECT)) {
			if (txt.startsWith("[BG:")) {
				img = txt.mid(4);
			} else if (txt.startsWith("[BGX:") || (txt.startsWith("[MOV:"))) {
				img = txt.mid(5);
			} else if (txt.startsWith("[BigBG:")) {
				img = txt.mid(7);
			} else {
				img = txt.mid(1);
			}
			img.remove("]");
			ximglist = fExistsR(imgdir, img);
			if (!ximglist.isEmpty()) {
				img = ximglist.first();
			} else {
				img.clear();
			}
			pg->text[i].imgpath = img;
		} else {
			pg->text[i].imgpath = img;
		}
	}
}

QString TWindow::getImgDir(QTreeWidgetItem* itm) {
	QString imgdir;
	do {
		imgdir = itm->data(0, roleImgDir).toString();
		if (imgdir.isEmpty())
			itm = itm->parent();
	} while (imgdir.isEmpty() && (itm != nullptr));
	return imgdir;
}

void TWindow::playLine() {
//	bool r = player->playLine(curPage->text[curRow]);
//	while (r) {
//		r = playNext();
//	}
}

void TWindow::play() {
	if (curItem == nullptr) return;
	if (curPage == nullptr) return;
	int row = curPage->curRow;
	if (row < 0) return;
	QTreeWidgetItem* itm = curItem;
	QString imgdir;
	QString nam;
	while (itm != nullptr) {
		if (!nam.isEmpty()) nam.prepend("/");
		nam.prepend(itm->text(0));
		itm = itm->parent();
	}
	player->setWindowTitle(nam);
	imgdir = getImgDir(curItem);
	fillImages(curPage, imgdir);
	player->reset();
	player->playLine(curPage, curRow); // curPage->text[curRow]);
	player->show();
}

bool TWindow::playNext() {
	TLine lin;
	do {
		lineDown();
		lin = curPage->text[curRow];
	} while ((curRow < (curPage->text.size() - 1)) && (lin.src.text.isEmpty() || (lin.src.text.startsWith("[") && !(lin.flag & TF_SELECT)) || (lin.flag & TF_SELITEM)));
	return player->playLine(curPage, curRow); // lin);
}

void TWindow::playPrev() {
	TLine lin;
	do {
		lineUp();
		lin = curPage->text[curRow];
	} while ((curRow > 0) && (lin.src.text.isEmpty() || (lin.src.text.startsWith("[") && !(lin.flag & TF_SELECT)) || (lin.flag & TF_SELITEM)));
	player->playLine(curPage, curRow); // lin);
}

void TWindow::fontSelect() {
	bool f;
	QFont fnt = QFontDialog::getFont(&f, player->fnt, this);
	if (f) {
		player->fnt = fnt;
		opt.setValue("player/font", fnt.toString());
	}
}

