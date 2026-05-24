#include "base.h"

typedef struct {
	int version;
	int cmdcount;
	int codepos;
	int argpos;
	int datapos;
	int linepos;
} YBNHeader;

typedef struct {
	unsigned char com;
	unsigned char argcnt;
	unsigned char b1;
	unsigned char b2;
} YBNCom;

typedef struct {
	unsigned char id;
	QByteArray data;
} YBNData;

typedef struct {
	unsigned short id;
	unsigned short type;
	QByteArray data;
	QList<YBNData> args;
} YBNArg;

static YBNHeader hd;

YBNArg ybnGetArg(QFile& file, int nr, int raw) {
	YBNArg arg;
	YBNData data;
	size_t fpos = file.pos();
	file.seek(hd.argpos + 12 * nr);
	arg.id = fgetw(file);
	arg.type = fgetw(file);
	int size = fgeti(file);
	int offset = hd.datapos + fgeti(file);
	file.seek(offset);
	arg.data = file.read(size);
	arg.args.clear();
	if (!raw) {
		int pos = 0;
		while (pos < arg.data.size()) {
			data.id = arg.data[pos++];
			size = arg.data[pos++] & 0xff;
			size |= (arg.data[pos++] & 0xff) << 8;
			data.data = arg.data.mid(pos, size);
			pos += size;
			arg.args.append(data);
		}
	}
	file.seek(fpos);
	return arg;
}

TPage loadYBN(QString path, int) {
	TPage pg;
	QFile file(path);
	qDebug() << "path = " << path;
	if (file.open(QFile::ReadOnly)) {
		int code = fgeti(file);
		if (code == 0x42545359) {		// YSTB
			hd.version = fgeti(file);
			hd.cmdcount = fgeti(file);
			hd.codepos = 0x20;
			hd.argpos = hd.codepos + fgeti(file);
			hd.datapos = hd.argpos + fgeti(file);
			hd.linepos = hd.datapos + fgeti(file);
			file.seek(0x20);
			code = fgeti(file);
			if (code < 0x1000000) {
				file.seek(0x20);
				int cnt = hd.cmdcount;
				TLine ln;
				ln.type = TL_TEXT;
				QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
				YBNCom com;
				YBNArg arg;
				QString str;
				int argn = 0;
				int n;
				while (cnt > 0) {
					com.com = fgetb(file);
					com.argcnt = fgetb(file);
					com.b1 = fgetb(file);
					com.b2 = fgetb(file);
					switch(com.com) {
						// text
						case 0x5b:
						case 0x6a:
							arg = ybnGetArg(file, argn, 1);
							ln.src.name.clear();
							ln.src.text = codec->toUnicode(arg.data);
							normLine(ln);
							pg.text.append(ln);
							break;
						// command
						case 0x1d:
						case 0x2c:
							arg = ybnGetArg(file, argn, 0);
							str = arg.args.isEmpty() ? "" : codec->toUnicode(arg.args.at(0).data).toLower();	// com name
							str.remove("\"");
							qDebug() << str;
							if ((str == "mac.bg") || (str == "mac.ev")) {	// BG image
								arg = ybnGetArg(file, argn + 1, 0);
								str = codec->toUnicode(arg.args.at(0).data);
								str.remove("\"");
								ln.src.name.clear();
								ln.src.text = QString("[%0]").arg(str);
								pg.text.append(ln);
							} else if (str == "es.sel.set") {
								ln.src.name.clear();
								ln.src.text = "[select]";
								pg.text.append(ln);
								for (n = 1; n < com.argcnt; n++) {
									arg = ybnGetArg(file, argn + n, 0);
									ln.src.text = codec->toUnicode(arg.args.at(0).data);
									ln.src.text.remove("\"");
									if (!ln.src.text.isEmpty()) {
										pg.text.append(ln);
									}
								}
							} else if (str == "es.stexec") {
								/*
								arg = ybnGetArg(file, argn + 2, 0);
								str = codec->toUnicode(arg.args.at(0).data);
								str.remove("\"");
								ln.src.name.clear();
								ln.src.text = QString("[CH:%0]").arg(str);
								pg.text.append(ln);
								*/
							}
							break;
					}
					argn += com.argcnt;
					cnt--;
				}
			} else {
				qDebug() << "not xored: " << code;
				// file must be xored
			}
		} else {
			qDebug() << "signature error";
		}
	} else {
		qDebug() << "Can't open file";
	}
	return pg;
}
