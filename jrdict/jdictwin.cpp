#include <QtGui>

#include "vars.h"
#include "classes.h"

extern QString basedir;
QList<dictfind> findres;
extern QMap<QString, kanjitem> kanji;
extern dNode dict;

DWindow::DWindow(QWidget *parent):QWidget(parent) {
	ui.setupUi(this);

	QString bd = QDir::homePath().append("/.config/samstyle/jrdict/");
	QString fp = bd + "forms4.jrf";

	wordwin = new WordWin(this, &dict);
	formwin = new FormWin(this, fp);

	ui.result->weight[0] = .1;
	ui.result->weight[1] = .1;
	ui.result->weight[2] = .1;
	ui.result->weight[3] = .1;
	ui.result->weight[4] = .40;
	ui.result->weight[5] = .20;

	connect(ui.leText,SIGNAL(textChanged(QString)),this,SLOT(gotranslate()));
	connect(ui.tbNew,SIGNAL(released()),this,SLOT(waneword()));
	connect(ui.tbReload,SIGNAL(released()),this,SLOT(reload()));
	connect(ui.tbForms,SIGNAL(released()),this,SIGNAL(wannaform()));
	connect(ui.tbDelete,SIGNAL(released()),this,SLOT(delword()));
	connect(ui.result,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(wanoldword(QModelIndex)));
	connect(ui.result,SIGNAL(indexchanged(int)),this,SLOT(reselect(int)));

	connect(this, &DWindow::wannaeditword, wordwin, &WordWin::goeditword);
	connect(wordwin, &WordWin::wantranslate, this, &DWindow::gotranslate);
	connect(this, &DWindow::wannaform, formwin, &FormWin::goforms);
	connect(formwin, &FormWin::wantranslate, this, &DWindow::gotranslate);

	opt.setDefaultFormat(QSettings::IniFormat);
	opt.setValue("global/test", 1);

	objtrn = new xTranslator;
	thrd = new QThread;
	objtrn->moveToThread(thrd);
	connect(this, &DWindow::wannatrans, objtrn, &xTranslator::goTranslate);
	connect(objtrn, &xTranslator::translationReady, this, &DWindow::showtranslation);
	thrd->start();
}

DWindow::~DWindow() {
	thrd->quit();
	thrd->wait();
}

void DWindow::keyPressEvent(QKeyEvent* ev) {
	switch(ev->key()) {
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			QWidget::keyPressEvent(ev);
			ev->accept();
			break;
		default:
			ev->ignore();
			break;
	}
}

void DWindow::delword() {
	int row = ui.result->currentIndex().row();
	if (row < 0) return;
	delWord(findres[row].word);
	saveDict();
	gotranslate();
}

void DWindow::reselect(int idx) {
	ui.leText->setSelection(findres[idx].begin,findres[idx].len);
}

void DWindow::wanoldword(QModelIndex idx) {
	int row = idx.row();
//	wordwin->goeditword(findres[row].word);
	emit wannaeditword(findres[row].word);
}

void DWindow::waneword() {
	dWord newWord;
	emit wannaeditword(newWord);
}

void DWindow::reload() {
	reloadAll();
	gotranslate();
}

void addfindkanji(MTableModel* model, QString kanjisrc, QString kanjird, int pos) {
	dictfind df;
	QStringList lst;
	df.src = kanjisrc;
	df.len = kanjisrc.size();
	df.begin = pos - df.len;
	findres.append(df);
	lst.append(kanjisrc);
	lst.append("");
	lst.append(kanjird);
	lst.append("");
	lst.append("");
	model->addrow(lst);
}

void DWindow::textForTranslate(QString txt) {
	ui.leText->setText(txt);
}

void DWindow::gotranslate() {
	emit wannatrans(ui.leText->text(), (MTableModel*)ui.result->model());
}

void DWindow::showtranslation(MTableModel* model) {
	model->update();
}

void xTranslator::goTranslate(QString string, MTableModel* model) {
	QString current;
	QString ch;
	QStringList lst;
	QString kanjisrc;
	QString kanjird;
	QList<dictfind> curr,burr;
	int maxlen = (string.size()<10)?string.size():10;
	int i,j,k,m,len,pos=0;
	bool kap;
	QList<formfind> finded;
	model->setColumnCount(6);
	model->clear();
	findres.clear();
	while (string.size()>0) {
		burr.clear();
		for(i=maxlen;(i>0 && burr.size()==0);i--) {
			current = string.left(i);
			finded=getbackforms(current,"*",0,"");
			curr.clear();
			for (j=0;j<finded.size();j++) {
				curr = scanWords(finded[j],true);
				if (curr.size()==0)
					curr = scanWords(katatohira(finded[j]),true);
				for (k = 0; k < curr.size(); k++) {
					kap = true;
					for(m = 0; m < burr.size(); m++) {
						if (wrdCompare(burr[m].word, curr[k].word))
							kap=false;
					}
					if (kap) {
						curr[k].src = current;
						curr[k].begin=pos;
						curr[k].len=current.size();
						burr.append(curr[k]);
					}
				}
			}
		}
		if (burr.size() > 0) {
			if (kanjisrc.size() > 0) {
				addfindkanji(model, kanjisrc, kanjird, pos);
				kanjisrc.clear();
				kanjird.clear();
			}
			findres.append(burr);
			for(i = 0;i < burr.size(); i++) {
				lst.clear();
				lst.append(burr[i].src);
				if (burr[i].src == burr[i].word.word) {
					lst.append("");
				} else {
					lst.append(burr[i].word.word);
				}
				lst.append(burr[i].word.read);
				lst.append(burr[i].word.type);
				lst.append(burr[i].word.trans);
				lst.append(burr[i].comment);
				model->addrow(lst);
			}
			len=current.size();
		} else {
			len = 2;
			ch = string.left(2);
			if (!kanji.contains(ch)) {
				len = 1;
				ch = string.left(1);
				if (!kanji.contains(ch))
					len = 0;
			}
			if (len == 0) {
				if (kanjisrc.size() > 0) {
					addfindkanji(model, kanjisrc, kanjird, pos);
					kanjisrc.clear();
					kanjird.clear();
				}
				len = 1;
			} else {
				kanjisrc.append(ch);
				kanjird.append(kanji[ch].rd_on);
			}
		}
		pos += len;
		string.remove(0,len);
	}
	if (!kanjisrc.isEmpty()) {
		addfindkanji(model, kanjisrc, kanjird, pos);
	}
	emit translationReady(model);
//	model->update();
//	qDebug() << ">" <<__FUNCTION__;
}
