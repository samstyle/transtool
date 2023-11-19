#include "base.h"

TPage loadSRP(QString name, int) {
	TPage page;
	QFile file(name);
	if (!file.open(QFile::ReadOnly)) return page;


	return page;
}
