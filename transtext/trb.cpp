#include <stdlib.h>
#include <stdio.h>

#include <QList>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "mainwin.h"

// ver 7

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
		case T7_IMGS:
			msg.setText(QString("Unknown IMAGE id %0").arg(subtype));
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

QTreeWidgetItem* TRBLoader::add_item(QTreeWidgetItem* par, QString name, QUuid id, QUuid iconid) {
	TPage* page;
	QString tip;
	QIcon ico;
	QTreeWidgetItem* itm = new QTreeWidgetItem();
	itm->setData(0, roleId, id.toByteArray());
	if (name == "") {
		name = id.toString();
	}
	itm->setText(0, name);
	itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable); // | Qt::ItemIsUserCheckable);
	if (id.isNull()) {						// directory
		itm->setFlags(itm->flags() | Qt::ItemIsDropEnabled);
		ico = getIcon(iconid);
		itm->setIcon(0, ico.isNull() ? QIcon(":/folder.png") : ico);
		itm->setData(0, roleIcon, iconid.toByteArray());
	} else {							// page
		page = findPage(id);
		if (page) {
			tip = QString("id : %0").arg(page->id.toString());
			itm->setToolTip(0,tip);
			itm->setIcon(0, getPageIcon(page));
		}
	}
	par->addChild(itm);
	return itm;
}

int TRBLoader::v7_load_icons() {
	TIcon ico;
	int type;
	int err = 0;
	strm >> type;
	while ((type != T7_END) && !err) {
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
		if (!err) strm >> type;
	}
	return err;
}

int TRBLoader::v7_load_bookmarks() {
	int type;
	int err = 0;
	TBookmark bm;
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
	return err;
}

void filter_bookmarks() {
	QList<TBookmark>::iterator it;
	QList<QUuid> lst;
	for (it = bookmarks.begin(); it != bookmarks.end(); it++) {
		if (findPage(it->pgid) == NULL)
			lst.append(it->id);
	}
	while(!lst.isEmpty())
		rmBookmark(lst.takeFirst());
}

int TRBLoader::v7_load_page() {
	int type;
	int err = 0;
	TPage page;
	page.curRow = -1;
	TLine lin;
	TImage img;
	TBookmark bm;
	QString tmpstr;
	QUuid id;
	QUuid tmpid;
	bool issel = false;
	strm >> type;
	while (type != T7_END) {
		switch(type) {
			case TP_UUID:
				strm >> page.id;
				break;
			case TP_FLAG:
				strm >> page.flag;
				break;
			case TP_CURL:
				strm >> page.curRow;
				break;
			case TP_DIR:
				strm >> tmpstr;
				break;
			case TP_IMG:
				do {
					strm >> type;
					switch(type) {
						case TI_ID: strm >> id; break;
						case TI_NAME: strm >> img.name; break;
						case TI_ICO: strm >> img.img; break;
						case TI_END:
							if (!img.name.isEmpty()) {
								if (id.isNull())
									id = QUuid::createUuid();
								page.imgs[id] = img;
								img.name.clear();
							}
							break;
						default:
							err = 1;
							type = TI_END;
							break;
					}
				} while (type != TI_END);
				break;
			case TP_LINE:
				lin.type = TL_TEXT;
				lin.flag = 0;
				lin.bmrkId = QUuid();
				// lin.picId = QUuid();
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
						case TL_PIC: strm >> tmpid; break;
						default:
							idError(TP_LINE, type);
							err = 1;
							break;
					}
					if (err) break;
					strm >> type;
				}
				if (!err) {
					if (!lin.bmrkId.isNull()) {
						bm.id = lin.bmrkId;
						bm.pgid = page.id;
						bm.row = page.text.size();
						addBookmark(bm);
					}
					if (lin.src.text.startsWith("[BigBG")) lin.type = TL_TEXT;
					lin.src.text.remove("　");
					normLine(lin);
					if ((lin.type == TL_TEXT) && (lin.src.text.toLower() == "[select]")) {
						issel = true;
						lin.type = TL_SELECT;
						lin.flag |= TF_SELECT;
						page.text.append(lin);
					} else if (issel) {
						if (lin.src.text.isEmpty()) {
							issel = false;
							lin.flag &= ~(TF_SELECT & TF_SELITEM);
							lin.type = TL_TEXT;
						} else {
							lin.type = TL_SELECT;
							lin.flag |= TF_SELITEM;
						}
						page.text.append(lin);
					} else {
						page.text.append(lin);
					}
				}
				break;
			default:
				idError(T7_PAGE, type);
				err = 1;
				break;
		}
		if (!err) strm >> type;
	}
	if (!err) putPage(page);
	return err;
}

int TRBLoader:: v7_load_tree(QTreeWidgetItem* root) {
	int err = 0;
	int type;
	QTreeWidgetItem* par = root;
	QTreeWidgetItem* itm;
	QUuid id;
	QUuid iconid;
	QIcon icon;
	QString name;
	QString imgdir;
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
						case TT_IMG:
							strm >> imgdir;
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
				par = add_item(par,name,0,iconid);
				par->setData(0, roleImgDir, imgdir);
				imgdir.clear();
				iconid = 0;
				break;
			case TT_PAGE:
				strm >> type;
				while (type != TT_END) {
					switch(type) {
						case TT_NAME:
							strm >> name;
							break;
						case TT_UUID:
							strm >> id;
							break;
						case TT_ICON:
							strm >> icon;
							break;
						case TT_IMG:
							strm >> imgdir;
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
				itm = add_item(par, name, id);
				itm->setData(0, roleImgDir, imgdir);
				imgdir.clear();
				break;
			case TT_END:
				par = par->parent();
				if (par == nullptr)
					par = root;
				break;
			default:
				idError(T7_TREE, type);
				err = 1;
				break;
		}
		if (err) break;
		strm >> type;
	}
	return err;
}

void v7_sort_names(QTreeWidgetItem* root) {
	QTreeWidgetItem* itm;
	QUuid id;
	TPage* pg;
	int i;
	for (i = 0; i < root->childCount(); i++) {
		itm = root->child(i);
		id = itm->data(0, roleId).toUuid();
		if (id.isNull()) {
			v7_sort_names(itm);
		} else {
			pg = findPage(id);
			if (pg != NULL) {
				pg->name = itm->text(0);
			}
		}
	}
}

void TRBLoader::v7_load(QTreeWidgetItem* root) {
	int type;
	strm >> type;
	int err = 0;
	while (!strm.atEnd() && (type != T7_END)) {
		switch(type) {
			case T7_ICON:
				v7_load_icons();
				break;
			case T7_BMRK:
				err = v7_load_bookmarks();
				break;
			case T7_PAGE:
				err = v7_load_page();
				break;
			case T7_TREE:
				err = v7_load_tree(root);
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
	} else {
		v7_sort_names(root);
		filter_bookmarks();
	}
}

// load file

int TRBLoader::load(QString path, QTreeWidgetItem* root) {
	QFile file(path);
	QByteArray sig;
	QByteArray data;
	int res = 1;
	if (file.open(QFile::ReadOnly)) {
		sig = file.read(4);
		data = file.readAll();
		data = qUncompress(data);
		buf.setBuffer(&data);
		buf.open(QBuffer::ReadOnly);
		strm.setVersion(QDataStream::Qt_5_0);
		strm.setDevice(&buf);
		if (sig == "TRB7") {
			v7_load(root);
		} else {
			QMessageBox::critical(nullptr, "Error", "File signature or version mismatch");
			res = 0;
		}
		buf.close();
		strm.resetStatus();
		strm.setDevice(nullptr);
	} else {
		QMessageBox::critical(nullptr, "Error", "Can't open file");
		res = 0;
	}
	return res;
}

// v7 save

void TRBLoader::v7_save_leaf(QTreeWidgetItem* par) {
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
			strm << TT_IMG << itm->data(0, roleImgDir).toString();
			strm << TT_END;
			v7_save_leaf(itm);
		} else {
			strm << TT_PAGE;
			strm << TT_NAME << itm->text(0);
			strm << TT_UUID << id;
			strm << TT_IMG << itm->data(0, roleImgDir).toString();
			strm << TT_END;
		}
	}
	strm << TT_END;
}

void TRBLoader::v7_save_tree(QTreeWidgetItem* root) {
	strm << T7_TREE;
// TODO: option to save root element (for save-branch)
#if 0
	if (saveroot) {
		strm << TT_DIR;
		strm << TT_NAME << root->text(0);
		strm << TT_ICONID << QUuid(root->data(0,roleIcon).toByteArray());
		strm << TT_END;
	}
#endif
	v7_save_leaf(root);
	strm << T7_END;
}

QList<QUuid> getTreeIds(QTreeWidgetItem*);
QList<QUuid> getTreeIcons(QTreeWidgetItem*);
QList<QUuid> getTreeBMrk(QTreeWidgetItem*);
QList<QUuid> getTreeImgs(QTreeWidgetItem*);

int TRBLoader::v7_save(QTreeWidgetItem* par) {
//	qDebug() << "v7_save";
	int res = 1;
	QList<QUuid> idlist;
	QUuid id;
	TIcon* ico;
	TPage* pg;
	TLine line;
	TBookmark* bm;
//	TImage img;
// pages
//	qDebug() << "begin (pages): " << buf.pos();
	idlist = getTreeIds(par);
	foreach(id, idlist) {
		pg = findPage(id);
		if (pg != nullptr) {
			strm << T7_PAGE;
			strm << TP_UUID << pg->id;
			strm << TP_FLAG << pg->flag;
			strm << TP_CURL << pg->curRow;
//			strm << TP_DIR << pg->imgdir;
			foreach(line, pg->text) {
				strm << TP_LINE;
				strm << TL_TYPE << line.type;
				strm << TL_FLAG << line.flag;
				strm << TL_BMID << line.bmrkId;
				// strm << TL_PIC << line.picId;
				strm << TL_SN << line.src.name;
				strm << TL_ST << line.src.text;
				strm << TL_TN << line.trn.name;
				strm << TL_TT << line.trn.text;
				strm << T7_END;
			}
			/*
			foreach(id, pg->imgs.keys()) {
				img = pg->imgs[id];
				strm << TP_IMG;
				strm << TI_ID << id;
				strm << TI_NAME << img.name;
				strm << TI_ICO << img.img;
				strm << TI_END;
			}
			*/
			strm << T7_END;
		}
	}
// icons
//	qDebug() << "treeicons: " << buf.pos();
	strm << T7_ICON;
	idlist = getTreeIcons(par);
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
// bookmarks
	strm << T7_BMRK;
	idlist = getTreeBMrk(par);
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
// tree
	v7_save_tree(par);
	return res;
}

int TRBLoader::save(QString path, QTreeWidgetItem* root) {
	int res = 1;
	QFile file(path);
	QByteArray data;
	strm.setVersion(QDataStream::Qt_5_0);
	buf.setBuffer(&data);
	buf.open(QBuffer::WriteOnly);
	strm.setDevice(&buf);

	v7_save(root);

	buf.close();
	strm.resetStatus();
	strm.setDevice(nullptr);
	data = qCompress(data);
	if (file.open(QFile::WriteOnly)) {
		file.write("TRB7");
		file.write(data);
		file.close();
	} else {
		res = 0;
	}
	return res;
}
