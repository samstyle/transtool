#include <QtGui>

#include "classes.h"
#include "vars.h"

FormWin::FormWin(QWidget *parent, QString frmpath):QDialog(parent) {
	formpath = frmpath;
	QVBoxLayout *mainlay = new QVBoxLayout;
	editor = new QTextEdit; editor->setLineWrapMode(QTextEdit::NoWrap);
	mainlay->addWidget(editor);
	QHBoxLayout *dnlay = new QHBoxLayout;
	okbut = new QPushButton("OK");
	canbut = new QPushButton("Cancel");
	dnlay->addStretch();
	dnlay->addWidget(okbut);
	dnlay->addWidget(canbut);
	mainlay->addLayout(dnlay);
	setLayout(mainlay);
	setMinimumSize(500,300);
	setWindowIcon(QIcon(":/jrtrandict.png"));
	setModal(true);
	resize(800,300);

	QObject::connect(canbut,SIGNAL(released()),this,SLOT(reject()));
	QObject::connect(okbut,SIGNAL(released()),this,SLOT(saveforms()));
}

void loadForms();
void FormWin::saveforms() {
	QFile file(formpath);
	file.open(QFile::WriteOnly);
	file.write(editor->toPlainText().toUtf8()); file.close();
	hide();
	loadForms();
	emit wantranslate();
}

void FormWin::goforms() {
	QFile file(formpath);
	file.open(QFile::ReadOnly);
	editor->setText(QDialog::trUtf8(file.readAll())); file.close();
	show();
}
