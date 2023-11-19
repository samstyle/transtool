#include <QtGui>

#include "classes.h"
#include "vars.h"

QList<dictfind> words;

WordWin::WordWin(QWidget* parent, dNode* ptr):QDialog(parent) {
	dicton=ptr;
	ui.setupUi(this);
	ui.list->weight[0] = .2;
	ui.list->weight[1] = .2;
	ui.list->weight[2] = .2;
	ui.list->weight[3] = .4;

	QObject::connect(ui.pbOk,SIGNAL(released()),this,SLOT(saveword()));
	QObject::connect(ui.pbCancel,SIGNAL(released()),this,SLOT(reject()));
	QObject::connect(ui.leWord,SIGNAL(textChanged(QString)),this,SLOT(searchword(QString)));
	QObject::connect(ui.list,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(recallword(QModelIndex)));
}

void WordWin::recallword(QModelIndex idx) {
	int row = idx.row();
	if (row==-1) return;
	goeditword(words[row].word);
}

void WordWin::saveword() {
	QString sword = ui.leWord->text();
	if (sword.isEmpty()) return;

	dWord neword;
	neword.word = sword;
	neword.read = ui.leRead->text();
	neword.type = ui.leType->text();
	neword.trans = ui.leTrans->text();

	if (eword.word.isEmpty()) {
		addWord(neword);
	} else {
		updWord(eword, neword);
	}
	saveDict();
	hide();
	emit wantranslate();
}

void WordWin::searchword(QString line) {
	QStringList lst;
	MTableModel *model = (MTableModel*)ui.list->model();
	model->setColumnCount(4);
	model->clear();
	if (line != "") {
		formfind formza;
		formza.form=line;
		formza.type="*";
		words = scanWords(formza, false);
		dictfind dfnd;
		foreach(dfnd, words) {
			lst.clear();
			lst.append(dfnd.word.word);
			lst.append(dfnd.word.read);
			lst.append(dfnd.word.type);
			lst.append(dfnd.word.trans);
			model->addrow(lst);
		}
	}
	model->update();
}

void WordWin::goeditword(dWord wrd) {
	eword = wrd;
	ui.leWord->setText(wrd.word);
	ui.leRead->setText(wrd.read);
	ui.leType->setText(wrd.type);
	ui.leTrans->setText(wrd.trans);
	show();
}
