#pragma once

#include <QtCore>
#include <QTableWidget>

enum {
	CP_SJIS = 0,
	CP_UCS2,
	CP_UTF8
};

enum {
	TL_SRC = 0,
	TL_TRN
};

enum {
	TL_TEXT = 1,
	TL_SELECT,
	TL_NOTE,
	TL_SEPARATOR,
	TL_LABEL,
	TL_COM
};

enum {
	LS_NONE = 0,
	LS_UNTRN,
	LS_TRN
};

// line flags

#define TF_SELECT	1
#define TF_SELITEM	(1<<1)

// ver7 tags

#define TPART	1
#define TPAGE	2
#define TNOTE	3
#define TPIXX	4
#define TEND	-1

#define	TBOOK	10
#define TTREE	11

#define T7_PAGE	0x3F		// pages
#define	T7_TREE	0x3E		// book tree
#define T7_ICON	0x3D		// icons
#define T7_BMRK	0x3C		// bookmarks
#define T7_IMGS 0x3B		// images
#define T7_END	0x00

#define TI_ID	0x40		// uuid
#define TI_NAME	0x80		// name
#define TI_ICO	0xc0		// icon
#define TI_END	0x01

#define TB_ID	0x40		// id
#define	TB_NAME	0x41		// name
#define TB_DSC	0x42		// description
#define TB_END	0x01

#define TP_ID	0x41		// page id (old)
#define	TP_FLAG	0x42		// page flag
#define TP_UUID	0x43		// page uuid
#define TP_IMG	0x44		// image id
#define TP_CURL	0x45		// current line (int)
#define TP_DIR	0x46		// images dir

#define	TP_LINE	0x03
#define	TL_SN	0x84		// src name
#define	TL_ST	0x85		// src text
#define	TL_TN	0x86		// trn name
#define	TL_TT	0x87		// trn text
#define TL_BMID	0x88		// bookmark id
#define TL_PIC	0x89		// picture
#define	TL_TYPE	TP_ID
#define	TL_FLAG	TP_FLAG

#define	TT_DIR	0x08
#define	TT_PAGE	0x09
#define	TT_NAME	0x8A		// name of (dir/page)
#define TT_ICON 0xCC		// icon (old)
#define TT_ICONID 0x4a		// icon uuid
#define	TT_PID	0x4B		// assotiated page id (int)
#define	TT_UUID	0x4C		// ...or uuid
#define TT_IMG	0x4D		// images dir
#define	TT_END	0x0D		// end of current dir list

struct TPhrase {
	QString name;
	QString text;
};

struct TLine {
	int type = TL_TEXT;
	int flag = 0;
	QUuid bmrkId;
//	QUuid picId;
	QString imgpath;
	TPhrase src;
	TPhrase trn;
};

struct TIcon {
	QUuid id;
	QString name;
	QIcon icon;
};

struct TImage {
	QString name;
	QImage img;
};

struct TPage {
	QUuid id;
	int flag;
	int curRow;
	QString name;
//	QString imgdir;
	QMap<QUuid,TImage> imgs;
	QList<TLine> text;
};

struct TBookmark {
	QUuid id;
	QUuid pgid;
	int row;
	QString name;
	QString descr;
};

void prjInit();

TPage* createPage();
TPage* addPage(TPage);
TPage* putPage(TPage);
TPage* findPage(QUuid);
TPage loadPage(QString, int);
void removePage(QUuid);

extern QList<TPage> book;

TIcon* findIcon(QUuid);
QIcon getIcon(QUuid);
QIcon getPageIcon(TPage*);
int addIcon(TIcon);
void rmIcon(QUuid);

extern QList<TIcon> icons;

TBookmark* findBookmark(QUuid);
QUuid addBookmark(TBookmark);
void rmBookmark(QUuid);
QList<TBookmark>* get_bmlist_ptr();

extern QList<TBookmark> bookmarks;

void getCounts(TPage*,int&,int&);
int getProgress(TPage*);
void normLine(TLine&);

void normLine(TLine&);
int getLineStatus(TLine);
