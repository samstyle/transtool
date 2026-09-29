#include "mainwin.h"

#define OVLCOUNT 5
#define BGOVL	ovrlist[0]
#define CHAROVL ovrlist[1]
#define MENUOVL ovrlist[2]
#define TEXTOVL ovrlist[3]
#define TOPOVL	ovrlist[4]

xPlayer::xPlayer(QWidget* p):QLabel(p) {
	setWindowModality(Qt::ApplicationModal);
	fnt.setPixelSize(32);
	fnt.setFamily("Buxton Sketch");
	setFixedSize(1280,720);
	ovrlist.clear();
	for(int i = 0; i < OVLCOUNT; i++) {
		ovrlist.append(QPixmap(1920,1080));
		ovrlist.last().fill(Qt::transparent);
	}
	mov = new QMovie;
	mov->setCacheMode(QMovie::CacheAll);
	picpos = QPoint(0,0);
	setMouseTracking(true);

	connect(mov, SIGNAL(frameChanged(int)),this,SLOT(redrawFrame()));
}

void xPlayer::closeEvent(QCloseEvent*) {
	emit closed();
}

void xPlayer::resizeEvent(QResizeEvent* ev) {
	QSize sz = ev->size();
	MENUOVL=MENUOVL.scaled(sz,Qt::IgnoreAspectRatio);
	TEXTOVL	= TEXTOVL.scaled(sz,Qt::IgnoreAspectRatio);
	TOPOVL = TOPOVL.scaled(sz,Qt::IgnoreAspectRatio);
	drawOverlays();
}

void xPlayer::reset() {
	bglist.clear();
}

void xPlayer::drawChars() {
	QPainter pnt;
	QImage img;
	xImage ximg;
	CHAROVL.fill(Qt::transparent);
	pnt.begin(&CHAROVL);
	foreach(ximg, lin.ovlimages) {
		img.load(ximg.path);
		pnt.drawImage(ximg.xpos, ximg.ypos, img);
	}
	pnt.end();
}

void xPlayer::drawOverlays() {
	QPainter pnt;
	QString txt;
	selabs.clear();
	zones.clear();
	curzone = -1;
	MENUOVL.fill(Qt::transparent);
	TEXTOVL.fill(Qt::transparent);
	int wwid = TEXTOVL.width();
	int whig = TEXTOVL.height();
	if (lin.flag & TF_SELECT) {
		// select
		pnt.begin(&MENUOVL);
		pnt.setFont(fnt);
		pnt.setPen(Qt::white);
		int _ln = curline;
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
		int xs = 20; // width() >> 2;
		int w = wwid - 40; // >> 1;
		int h = 45;
		QRect rct;
		while (cnt > 0) {
			rct.setRect(xs, ys, w, h);
			zones.append(rct);
			pnt.fillRect(rct, QColor(0, 0, 0, 160));
			pnt.drawText(xs+2, ys+2, w-4, h-4, Qt::AlignHCenter | Qt::AlignVCenter, variants.takeFirst());
			ys += (h + 5);
			cnt--;
		}
		pnt.end();
	} else {
		// text ovl
		int flag = Qt::TextWordWrap;
		if (lin.trn.text.isEmpty()) {
			txt = lin.src.text;
			flag |= Qt::TextWrapAnywhere;
		} else {
			txt = lin.trn.text;
		}
		pnt.begin(&TEXTOVL);
		pnt.setFont(fnt);
		pnt.setPen(Qt::white);
		QRect rct(5, whig-195, wwid-10, 190);
		QRect nrc(5, whig-235, 300, 80);
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
		pnt.end();
	}
}

bool xPlayer::playLine(TPage* _pg, int ln) {
	int res = false;
	if ((pg == _pg) && (ln == curline)) return res;		// same line
	pg = _pg;
	curline = ln;
	lin = pg->text.at(ln);
	// draw chars
	drawChars();
	// text/menu+top
	drawOverlays();
	// if image(s) is changed, restart
	if (bglist != lin.bgimages) {
		bglist = lin.bgimages;
		curbglist = bglist;
		if (curbglist.isEmpty()) {	// no images left
			reset();
		} else {
			nextImage();
		}
	} else if (mov->state() != QMovie::Running) {
		mov->start();
	}
	// combine all layers TOGEZZZA
	redrawFrame();
	return true;
}

void xPlayer::nextImage() {
	if (curbglist.isEmpty()) return;
	xImageList imgs = curbglist.takeFirst();
	BGOVL.fill(Qt::transparent);
	QPainter pnt;
	QPixmap pxm;
	if (imgs.size() < 1) {
		mov->stop();
	} else {
		xImage bgimg = imgs.takeFirst();
		if (QFile::exists(bgimg.path)) {
			mov->stop();
			mov->setFileName(bgimg.path);
			mov->start();
			// NOTE: picsize is scaled, must use original frame size
			QSize frmsz = mov->frameRect().size();
			if (frmsz != BGOVL.size()) {
				BGOVL=BGOVL.scaled(frmsz);
				BGOVL=CHAROVL.scaled(frmsz);
			}
			// load & bg layers
			pnt.begin(&BGOVL);
			foreach(bgimg, imgs) {
				if (pxm.load(bgimg.path)) {
					// qDebug() << "ovl size " << pxm.size() << "over canvas" << BGOVL.size();
					pnt.drawPixmap(bgimg.xpos, bgimg.ypos, pxm);
				}
			}
			pnt.end();
			if (mov->frameCount() < 2) {
				timer.singleShot(3000, this, &xPlayer::nextImage);
			}
		} else {
			nextImage();
		}
	}
}

// TODO: overlays not drawing properly
void xPlayer::redrawFrame() {
	QPainter pnt;
	// frame
	QPixmap pxm = mov->currentPixmap();		// bg only (w/o layers)
	if (pxm.isNull() || bglist.isEmpty()) {
		picsize = size();
		pxm = QPixmap(picsize);
		pxm.fill(Qt::black);
		picpos = QPoint(0,0);
	} else {
		picsize = pxm.size();
		// draw overlays & chars over bg
		pnt.begin(&pxm);
		pnt.drawPixmap(0,0,BGOVL);
		pnt.drawPixmap(0,0,CHAROVL);
		pnt.end();
		// resize
		if ((picsize.width() > 1280) || (picsize.height() > 768)) {
			picsize.scale(1280,720,Qt::KeepAspectRatioByExpanding);		// scaled to 1280x720);
			pxm = pxm.scaled(1280,720,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
		}
		int w = picsize.width();
		int h = picsize.height();
		// cut
		if (w * 9 > h * 16) {		// wide
			pxm = pxm.copy(picpos.x(), picpos.y(), h * 16 / 9, h);
		} else if (w * 3 < h * 4) {	// tall
			pxm = pxm.copy(picpos.x(), picpos.y(), w, w * 9 / 16);
		} else {
			// pxm = pxm.scaled(1280,720,Qt::KeepAspectRatio,Qt::SmoothTransformation);
		}
	}
	// result is scaled to 1280x720 (if bigger) and drawed into window
	setFixedSize(pxm.size());
	// text,menu & top overlays drawed after scaling
	pnt.begin(&pxm);
	pnt.drawPixmap(0,0,TEXTOVL);
	pnt.drawPixmap(0,0,MENUOVL);
	pnt.drawPixmap(0,0,TOPOVL);
	pnt.end();

	setPixmap(pxm);

	if ((mov->frameCount() > 1) && (mov->currentFrameNumber() == (mov->frameCount() - 1)) && !curbglist.isEmpty()) {
		nextImage();
	}
}

void xPlayer::mousePressEvent(QMouseEvent *ev) {
	if (moved) return;
	if (ev->button() == Qt::LeftButton) {
		if (zones.size() == 0) {
			// emit clicked();
		} else if (curzone >= 0) {
			if (!selabs.at(curzone).isEmpty()) {
				TOPOVL.fill(Qt::transparent);
				emit selected(selabs.at(curzone));
			}
		}
	}
}

void xPlayer::mouseReleaseEvent(QMouseEvent *ev) {
	if (ev->button() == Qt::LeftButton) {
		if (moved) {
			moved = 0;
		} else {
			emit clicked();
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
			redrawFrame();
		}
	} else {
		// check cursor above one of QRect in zones
		QRect rct;
		int newcurzone = getZone(ev->pos());
		if (newcurzone != curzone) {
			TOPOVL.fill(Qt::transparent);
			QPainter pnt;
			pnt.begin(&TOPOVL);
			curzone = newcurzone;
			if (curzone >= 0) {
				rct = zones.at(curzone);
				pnt.fillRect(rct.left(), rct.top(), 5, rct.height(), Qt::green);
				pnt.fillRect(rct.right() - 5, rct.top(), 5, rct.height(), Qt::green);
			}
			pnt.end();
			redrawFrame();
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
