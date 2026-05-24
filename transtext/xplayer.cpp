#include "mainwin.h"

xPlayer::xPlayer(QWidget* p):QLabel(p) {
	setWindowModality(Qt::ApplicationModal);
	fnt.setPixelSize(32);
	fnt.setFamily("Buxton Sketch");
	setFixedSize(1280,720);
	cnt = 0;
	mov = new QMovie;
	picpos = QPoint(0,0);
	setMouseTracking(true);

	connect(mov, SIGNAL(frameChanged(int)),this,SLOT(frameChanged()));
}

void xPlayer::closeEvent(QCloseEvent*) {
	emit closed();
}

void xPlayer::reset() {
	curimgpath.clear();
}

bool xPlayer::playLine(TPage* pg, int ln) { // TLine _l) {
	int res = false;
	// lin = _l;
	lin = pg->text.at(ln);
	QPainter pnt;
	// text overlay
	QString txt;
	QString path;
	QImage img;
	int px,py;
	int flag = Qt::TextWordWrap;
	if (lin.trn.text.isEmpty()) {
		txt = lin.src.text;
		flag |= Qt::TextWrapAnywhere;
	} else {
		txt = lin.trn.text;
	}
	ovr = QPixmap(size());
	ovr.fill(Qt::transparent);
	pnt.begin(&ovr);
	foreach(path, lin.ovlpath) {
		img.load(path);
		px = (width() - img.width()) / 2;	// center
		py = height() - img.height();		// bottom
		if (px < 0) px = 0;
		if (py < 0) py = 0;
		pnt.drawImage(px, py, img);
	}
	pnt.setFont(fnt);
	pnt.setPen(Qt::white);
	selabs.clear();
	zones.clear();
	curzone = -1;
	if (lin.flag & TF_SELECT) {
		int _ln = ln;
		QStringList variants;
		TLine slin;
		do {
			_ln++;
			slin = pg->text.at(_ln);
			if (slin.flag & TF_SELITEM) {
				if (slin.trn.text.isEmpty()) {
					variants.append(slin.src.text);
				} else {
					variants.append(slin.trn.text);
				}
				selabs.append(slin.src.name);
			}
		} while (slin.flag & TF_SELITEM);
		int cnt = variants.size();
		int ys = (height() >> 1) - (cnt * 50);
		int xs = width() >> 2;
		int w = width() >> 1;
		int h = 45;
		QRect rct;
		while (cnt > 0) {
			rct.setRect(xs, ys, w, h);
			zones.append(rct);
			pnt.fillRect(rct, QColor(0, 0, 0, 200));
			pnt.drawText(xs+2, ys+2, w-4, h-4, Qt::AlignHCenter | Qt::AlignVCenter, variants.takeFirst());
			ys += (h + 5);
			cnt--;
		}
	} else {
		QRect rct(5, height()-195, width()-10, 190);
		QRect nrc(5, height()-235, 300, 80);
		QLinearGradient grd(rct.topLeft(),rct.bottomLeft());
		QPainterPath pth;
		pth.setFillRule(Qt::WindingFill);
		pth.addRoundedRect(rct, 10, 10);
		grd.setColorAt(0, QColor(0,0,0,200));
		grd.setColorAt(1, QColor(0,0,0,120));
		if (!lin.src.name.isEmpty()) {
			pth.addRoundedRect(nrc, 10, 10);
			pnt.fillPath(pth, grd);
			if (lin.trn.name.isEmpty()) {
				pnt.drawText(nrc.adjusted(5,2,2,5),0,lin.src.name);
			} else {
				pnt.drawText(nrc.adjusted(5,2,2,5),0,lin.trn.name);
			}
		} else {
			pnt.fillPath(pth, grd);
		}
		pnt.drawText(rct.adjusted(20,10,-20,-10), flag, txt);
	}
	pnt.end();

	if (curimgpath != lin.imgpath) {
		curimgpath = lin.imgpath;
		if (!curimgpath.isEmpty() && QFile::exists(curimgpath)) {
			mov->stop();
			mov->setFileName(curimgpath);
			mov->start();
		} else {
			reset();
		}
	} else if (mov->state() != QMovie::Running) {
		mov->start();
	}

	frameChanged();
	return res;
}

void xPlayer::frameChanged() {
	QPainter pnt;
	// frame
	QPixmap pxm = mov->currentPixmap();
	if (pxm.isNull() || curimgpath.isEmpty()) {
		pxm = QPixmap(size());
		pxm.fill(Qt::black);
		picpos = QPoint(0,0);
	} else {
		pxm = pxm.scaled(1280,720,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
		picsize = pxm.size();
		if (pxm.width() * 9 > pxm.height() * 16) {		// wide
			pxm = pxm.copy(picpos.x(), picpos.y(), pxm.height() * 16 / 9, pxm.height());
		} else if (pxm.width() * 3 < pxm.height() * 4) {	// tall
			pxm = pxm.copy(picpos.x(), picpos.y(), pxm.width(), pxm.width() * 9 / 16);
		} else {
			pxm = pxm.scaled(1280,720,Qt::KeepAspectRatio,Qt::SmoothTransformation);
		}
	}
	setFixedSize(pxm.size());
	// draw overlay
	pnt.begin(&pxm);
	pnt.drawPixmap(0,0,ovr);
	pnt.end();
	setPixmap(pxm);
}

void xPlayer::mousePressEvent(QMouseEvent *ev) {
	if (moved) return;
	if (ev->button() == Qt::LeftButton) {
		if (zones.size() == 0) {
			emit clicked();
		} else if (curzone >= 0) {
			if (!selabs.at(curzone).isEmpty()) {
				emit selected(selabs.at(curzone));
			}
		}
	}
}

void xPlayer::mouseReleaseEvent(QMouseEvent *ev) {
	if (ev->button() == Qt::LeftButton) {
		if (moved) {
			moved = 0;
		}
	}
}

int xPlayer::getZone(QPoint pt) {
	int idx = 0;
	int zone = -1;
	foreach(QRect rct, zones) {
		if (rct.contains(pt)) {
			zone = idx;
		}
		idx++;
	}
	return zone;
}

void xPlayer::mouseMoveEvent(QMouseEvent* ev) {
	if (ev->buttons() & Qt::LeftButton) {
		QPoint delta = ev->pos() - mousepos;
		QPoint newpos = picpos - delta;
		if (newpos.x() < 0) newpos.setX(0);
		else if (newpos.x() + width() > picsize.width()) newpos.setX(picsize.width() - width());
		if (newpos.y() < 0) newpos.setY(0);
		else if (newpos.y() + height() > picsize.height()) newpos.setY(picsize.height() - height());
		if (newpos != picpos) {
			moved = 1;
			picpos = newpos;
			frameChanged();
		}
	} else {
		// check cursor above one of QRect in zones
		QRect rct;
		int newcurzone = getZone(ev->pos());
		if (newcurzone != curzone) {
			QPainter pnt(&ovr);
			if (curzone >= 0) {
				rct = zones.at(curzone);
				pnt.fillRect(rct.left() - 5, rct.top(), 5, rct.height(), qRgba(0,0,0,255));
			}
			curzone = newcurzone;
			if (curzone >= 0) {
				rct = zones.at(curzone);
				pnt.fillRect(rct.left() - 5, rct.top(), 5, rct.height(), Qt::green);
			}
			pnt.end();
			frameChanged();
		}
	}
	mousepos = ev->pos();
}

void xPlayer::wheelEvent(QWheelEvent* ev) {
	if (ev->angleDelta().y() < 0) {
		if (zones.isEmpty()) {
			emit clicked();
		}
	} else if (ev->angleDelta().y() > 0) {
		emit clicked_r();
	}
}

void xPlayer::keyPressEvent(QKeyEvent *ev) {
	switch(ev->key()) {
		case Qt::Key_Return: emit clicked(); break;
		case Qt::Key_Escape: mov->stop(); close(); break;
	}
}
