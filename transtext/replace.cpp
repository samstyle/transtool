#include "replace.h"

Replacer::Replacer(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	connect(ui.pbCancel, &QPushButton::released, this, &Replacer::hide);
	connect(ui.pbOk, &QPushButton::released, this, &Replacer::onOk);
}

void Replacer::onOk() {
	QString fstr = ui.leStrF->text();
	QString rstr = ui.leStrR->text();
	if (fstr.isEmpty()) return;
	if (rstr.isEmpty()) return;
	emit confirm(fstr, rstr);
	hide();
}
