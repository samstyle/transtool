#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "base.h"

int get4(QFile& file) {
	char ch;
	file.getChar(&ch);
	int res = ch & 0xff;
	file.getChar(&ch);
	res |= (ch << 8);
	file.getChar(&ch);
	res |= (ch << 16);
	file.getChar(&ch);
	res |= (ch << 24);
	return res;
}

TPage loadSNX(QString fname, int) {
	TPage page;
	QFile file(fname);
	if (!file.open(QFile::ReadOnly)) return page;
	TLine line;
	line.type = TL_TEXT;
	QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");

	int count = get4(file);
	int p1 = get4(file);
	int p2,p3;
	int len;

	long dataPos = count * 12 + 8;
	long curPos = 0;
//	long adr = 0;
	char buf[2000];
	char cbuf[2010];
	char* sptr;
	char* dptr;

//	if (dump) printf("; bytecode\n\n");

	while (count > 0) {
		p1 = get4(file);
		p2 = get4(file);
		p3 = get4(file);
		if ((p1 == 0x11) && (p2 == 0x02)) {
			curPos = file.pos();
			file.seek(dataPos + p3);
			len = get4(file);
			file.read(buf,len);
			file.seek(curPos);
			sptr = buf;
			dptr = buf;
			if (*sptr == 0x01) {
				sptr += 3;
			}
			while(1) {
				if ((*sptr > 0x00) && (*sptr < 0x04)) {
					sptr++;
				} else {
					*dptr = *sptr;
					if (*dptr == 0x00) break;
					dptr++;
					sptr++;
				}
			}
		}
		if (p1 == 0x0d) {
			if (p2 == 0x012) {
				sprintf(cbuf,"[ %s ]",buf);
				//fwrite(cbuf,strlen(cbuf),1,ofile);
				line.src.name.clear();
				line.src.text = codec->toUnicode(cbuf);
				page.text.append(line);
			} else if (p2 == 0x2c) {
				sprintf(cbuf,"%s\n",buf);
				//fwrite(cbuf,strlen(cbuf),1,ofile);
				line.src.name.clear();
				line.src.text = codec->toUnicode(buf);
				normLine(line);
				page.text.append(line);
			}
		}
//		if (dump) printf("\n");
		count--;
	}

	return page;
}
