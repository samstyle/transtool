#include "main.h"

#include <QDockWidget>

#include <QMenuBar>

extern dNode dict;
extern QString formpath;

XWindow::XWindow() {
	thrd = new QThread;

	twid = new TWindow;
	dwid = new DWindow;
	DictMain();

	setCentralWidget(twid);
	QDockWidget* dokw = new QDockWidget;
	dokw->setWidget(dwid);
	dokw->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::TopDockWidgetArea, dokw);

	connect(twid, &TWindow::textChanged, dwid, &DWindow::textForTranslate);

	resize(1280,720);
	setWindowTitle("Translator");
	setWindowIcon(QIcon(":/Japan.png"));

	QMenuBar* mbar = new QMenuBar;
	QMenu* mfile = mbar->addMenu("File");
	mfile->addAction(twid->ui.actNewProj);
	mfile->addSeparator();
	mfile->addAction(twid->ui.actOpen);
	mfile->addAction(twid->ui.actMerge);
	mfile->addSeparator();
	mfile->addAction(twid->ui.actSave);
	mfile->addAction(twid->ui.actSaveAs);
	QMenu* mpage = mbar->addMenu("Page");
	mpage->addAction(twid->ui.actNewPage);
	mpage->addSeparator();
	mpage->addAction(twid->ui.actOpenSrc);
	mpage->addAction(twid->ui.actInsertSrc);
	mpage->addSeparator();
	mpage->addAction(twid->ui.actSaveSrc);
	QMenu* mopt = mbar->addMenu("Options");
	mopt->addAction(twid->ui.actGrabCbrd);
	mopt->addAction(twid->ui.actNameRepeat);
	mopt->addSeparator();
	mopt->addAction(twid->ui.actBookmarks);
	mopt->addAction(twid->ui.actPlayerFont);
	setMenuBar(mbar);
}

void XWindow::openPrj(QString path) {
	twid->openPrj(path);
}

void XWindow::keyPressEvent(QKeyEvent* ev) {
	twid->keyPress(ev);
}

void XWindow::closeEvent(QCloseEvent* ev) {
	if (twid->saveChanged())
		ev->accept();
	else
		ev->ignore();;
}

int main(int ac, char** av) {
	QApplication app(ac,av);
	app.setOrganizationName("samstyle");
	app.setApplicationName("transtext");

	XWindow xwin;
	xwin.show();
	if (ac > 1)
		xwin.openPrj(QString(av[1]));
	app.exec();

	return 0;
}
