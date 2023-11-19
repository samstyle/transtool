#pragma once

#include <QTextEdit>
#include <QSettings>
#include <QThread>

#include "vars.h"

//#include "ui_mainwindow.h"
#include "ui_jrdwidget.h"
#include "ui_wordwin.h"

class xTranslator : public QObject {
	Q_OBJECT
	public:
		xTranslator() {}
	public slots:
		void goTranslate(QString, MTableModel*);
	signals:
		void translationReady(MTableModel*);
};

class WordWin : public QDialog {
	Q_OBJECT
	public:
		WordWin(QWidget*, dNode*);
	public slots:
		void goeditword(dWord);
		void searchword(QString);
		void saveword();
	signals:
		void wantranslate();
	private slots:
		void recallword(QModelIndex);
	private:
		Ui::WordForm ui;
		dNode *dicton;
		dWord eword;
};

class FormWin : public QDialog {
	Q_OBJECT
	public:
		FormWin(QWidget*, QString);
	public slots:
		void goforms();
		void saveforms();
	signals:
		void wantranslate();
	private:
		QTextEdit *editor;
		QPushButton *okbut,*canbut;
		QString formpath;
};

class DWindow : public QWidget {
	Q_OBJECT
	public:
		DWindow(QWidget* = nullptr);
		~DWindow();
	public slots:
		void showtranslation(MTableModel*);
		void waneword();
		void reload();
		void wanoldword(QModelIndex);
		void reselect(int);
		void delword();
		void textForTranslate(QString);
	signals:
		void wannaeditword(dWord);
		void wannaform();
		void wannatrans(QString, MTableModel*);
	private slots:
		void gotranslate();
	private:
		Ui::JDWidget ui;
		QSettings opt;
		WordWin* wordwin;
		FormWin* formwin;
		xTranslator* objtrn;
		QThread* thrd;
		void keyPressEvent(QKeyEvent*);
};
