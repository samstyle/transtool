#pragma once

#include <QMainWindow>
#include <QThread>

#include "jrdict/classes.h"
#include "transtext/mainwin.h"

class XWindow : public QMainWindow {
	Q_OBJECT
	public:
		XWindow();
		void openPrj(QString);
	private:
		TWindow* twid;
		DWindow* dwid;
		QThread* thrd;
	protected:
		void keyPressEvent(QKeyEvent*);
		void closeEvent(QCloseEvent*);
};

void DictMain();
// void TransMain(TWindow*);
