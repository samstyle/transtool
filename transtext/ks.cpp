#include <QDialog>
#include <exception>

#include "base.h"

struct ParLine {
	QString com;
	QMap<QString,QString> pars;
};

QMap<QString,QString> parseKSArgs(QString& line) {
	bool subw = true;
	bool quote = false;
	bool wrk = true;
	int pos;
	QString pnam;
	QString pval;
	QMap<QString,QString> map;
	while ((line.size() > 0) && wrk) {
		pos = line.indexOf("=");
		if (pos > 0) {
			pnam = line.left(pos).trimmed();
			line.remove(0,pos+1);
			while (line.startsWith(" ")) line.remove(0,1);
			pval.clear();
			subw = true;
			while (subw && (line.size() > 0)) {
				if (line.startsWith("\"") || line.startsWith("'")) {
					quote = !quote;
				} else {
					if (quote) {
						pval.append(line.at(0));
					} else if (line.startsWith(" ")) {
						subw = false;
					} else {
						pval.append(line.at(0));
					}
				}
				line.remove(0,1);
			}
			map[pnam] = pval;
		} else {
			wrk = false;
		}
	}
	return map;
}

ParLine parseKS(QString& line) {
	bool wrk = true;
	int pos;
	ParLine res;
	pos = line.indexOf(" ");
	if (pos < 0) {			// no params
		wrk = false;
		pos = line.size();
	}
	res.com = line.left(pos);
	line.remove(0,pos+1);
	if (wrk) {
		res.pars = parseKSArgs(line);
	} else {
		res.pars.clear();
	}
	return res;
}

/*
QString getAttribute(ParLine par, QString name) {
	QString res;
	for (int i = 0; i < par.pars.size(); i++) {
		if (par.pars[i].name == name) res = par.pars[i].value;
	}
	return res;
}
*/

TPage loadKS(QString fnam, int cpage) {
	TPage page;
	QFile file(fnam);
	if (!file.open(QFile::ReadOnly)) return page;
	page.id = QUuid::createUuid();
	TLine elin;
	elin.type = TL_TEXT;
	elin.flag = 0;
	TLine nlin = elin;
	TLine tlin = elin;
	int pos;
	QString line;
	QString comline;
	QString str;
	ParLine param;
	QMap<QString,QString> submap;
	QTextCodec* codec;
	int name = 0;
	QTextStream strm(&file);
	switch (cpage) {
		case CP_UCS2:
			strm.read(2);		// skip BOM (FF FE)
			codec = QTextCodec::codecForName("UTF-16LE");
			break;
		case CP_UTF8:
			file.read(3);		// skip BOM
			codec = QTextCodec::codecForName("UTF-8");
			break;
		default:
			codec = QTextCodec::codecForName("Shift-JIS");
			break;
	}
	strm.setCodec(codec);
	// to skip bom if present

	while (!strm.atEnd()) {
		line = strm.readLine();
		line.remove("\r");
		line.remove("\n");
		line.remove("\t");
		line.remove("　");
		tlin.src.text.clear();
		while(line.size() > 0) {
			if (line.startsWith(";")) {
				line.clear();
			} else if (line.startsWith("[")) {
				pos = line.indexOf("]");
				if (pos < 0) {
					line.clear();
				} else {
					comline = line.mid(1, pos-1);
					line = line.mid(pos+1);
					param = parseKS(comline);
					if (param.com == "name") {
						tlin.src.name = param.pars["text"];
					} else if (param.com == "eval") {
						str = param.pars["exp"];
						submap = parseKSArgs(str);
						if (submap.contains("f.speaker")) {
							tlin.src.name = submap["f.speaker"];
						} else if (submap.contains("f.bgNow")) {
							page.text.append(elin);
							nlin.src.text = QString("[BG:%0]").arg(submap["f.bgNow"]);
							page.text.append(nlin);
						}
					} else if (param.com == "名前") {
						tlin.src.name = param.pars["id"];
					} else if ((param.com == "cg_i") || (param.com == "cg_a") || (param.com == "image")) {
						page.text.append(elin);
						nlin.src.text = QString("[BG:%0]").arg(param.pars["storage"]);
						page.text.append(nlin);
					} else if (param.com == "cg_x") {
						page.text.append(elin);
						nlin.src.text = QString("[BGX:%0]").arg(param.pars["storage"]);
						page.text.append(nlin);
					} else if (param.com == "イベント") {
						nlin.src.text = QString("[BGX:%0]").arg(param.pars["file"]);
						page.text.append(elin);
						page.text.append(nlin);
					} else if (param.com == "cg_a_cyara_lr_on") {
						page.text.append(elin);
						nlin.src.text = QString("[BG:%0]").arg(param.pars["haikei"]);
						page.text.append(nlin);
					} else if ((param.com == "haikei") || (param.com == "bg") || (param.com == "ev") || (param.com == "evcg")) {
						str = param.pars["file"];
						if (str.isEmpty())
							str = param.pars["storage"];
						if (!str.isEmpty()) {
							nlin.src.text = QString("[BG:%0]").arg(str);
							page.text.append(elin);
							page.text.append(nlin);
						}
					} else if (param.com == "背景") {
						nlin.src.text = QString("[BG:%0]").arg(param.pars["file"]);
						page.text.append(elin);
						page.text.append(nlin);
					} else if (param.com == "jump") {
						nlin.src.text = QString("[jump %0:%1]").arg(param.pars["storage"]).arg(param.pars["target"]);
						page.text.append(nlin);
					} else if (param.com == "link") {
						nlin.src.name = param.pars["target"];
					} else if (param.com.startsWith("CH_NAME") || (param.com == "cn")) {
						tlin.src.name = param.pars["name"];
						if (tlin.src.name == "ト書き") tlin.src.name.clear();
					} else if (param.com == "NAME_M") {
						tlin.src.name = param.pars["n"];
					} else if ((param.com == "FAID_IN") || (param.com == "TR")) {
						page.text.append(elin);
						nlin.src.text = QString("[BG:%0]").arg(param.pars["bmp"]);
						page.text.append(nlin);
					} else if ((param.com == "FAID_IN_CG") || (param.com == "ALL_OFF")) {
						page.text.append(elin);
						nlin.src.text = QString("[BG:%0]").arg(param.pars["back_cg"]);
						page.text.append(nlin);
					} else if (param.com == "FLASH") {
						page.text.append(elin);
						nlin.src.text = QString("[FLASH:%0]").arg(param.pars["in_bmp"]);
						page.text.append(nlin);
					} else if (param.com == "SELECT") {
						page.text.append(elin);
						nlin.src.text = QString("[select]");
						page.text.append(nlin);
						nlin.src.name = QString("%0 : %1").arg(param.pars["file1"]).arg(param.pars["tag1"]);
						nlin.src.text = param.pars["sel1"];
						if (!nlin.src.name.isEmpty()) page.text.append(nlin);
						nlin.src.name = QString("%0 : %1").arg(param.pars["file2"]).arg(param.pars["tag2"]);
						nlin.src.text = param.pars["sel2"];
						if (!nlin.src.name.isEmpty()) page.text.append(nlin);
						nlin.src.name = QString("%0 : %1").arg(param.pars["file3"]).arg(param.pars["tag3"]);
						nlin.src.text = param.pars["sel3"];
						if (!nlin.src.name.isEmpty()) page.text.append(nlin);
						nlin.src.name.clear();
					} else if (param.com == "select") {
						if (param.pars.isEmpty()) {
							nlin.src.text = "[SELECT]";
							page.text.append(elin);
							page.text.append(nlin);
						} else {
							nlin.src.name = param.pars["target"];
							nlin.src.text = param.pars["text"];
							page.text.append(nlin);
							nlin.src.name.clear();
						}
					} else if (param.com == "msg") {
						nlin.src.name = param.pars["name"];
					} else if (param.com == "lcg") {
						// page.text.append(elin);
						nlin.src.name.clear();
						nlin.src.text = QString("[BG:%0]").arg(param.pars["storage"]);
						page.text.append(nlin);
					} else if (param.com == "「") {
						line.prepend("「");
					} else if (param.com == "（") {
						line.prepend("（");
					} else if (param.com.startsWith("m") && !param.com.startsWith("mw") && (param.pars.size() == 0)) {
						tlin.src.name = param.com.mid(1);
					} else if (param.com == "ns") {
						name = 1;
					} else if ((param.com == "r") || (param.com == "nse")) {
						if (param.com == "nse") name = 0;
						do {
							str = strm.readLine();
						} while (str.startsWith(";") && !strm.atEnd());
						if (!strm.atEnd()) {
							line.append(str);
							line.remove("\r");
							line.remove("\n");
							line.remove("\t");
							line.remove("　");
						}
					//} else if (param.pars.isEmpty() && !line.isEmpty()) {
					//	if (param.com.contains(QDialog::trUtf8("/"))) {
					//		param.com = param.com.split(QDialog::trUtf8("/")).first();
					//	}
					//	tlin.src.name = param.com;
					} else if (param.com == "src_end") {
						page.text.append(elin);
						nlin.src.text = QString("[JUMP %0]").arg(param.pars["src"]);
						page.text.append(nlin);
					} else if (param.com == "mruby") {
						tlin.src.text.append(param.pars["text"]);
					}
				}
			} else if (line.startsWith("@")) {
				comline = line.mid(1);
				param = parseKS(comline);
				if ((param.com == "bg_FI") || (param.com == "E_FI")) {
					page.text.append(elin);
					nlin.src.text = QString("[BG:%0]").arg(param.pars["storage"]);
					page.text.append(nlin);
				} else if (param.com.startsWith("BG_")) {
					page.text.append(elin);
					nlin.src.text = QString("[BG:%0]").arg(param.pars["storage"]);
					page.text.append(nlin);
				} else if ((param.com == "bg_") || (param.com == "bg")) {
					page.text.append(elin);
					nlin.src.text = QString("[BG:%0]").arg(param.pars["f"]);
					page.text.append(nlin);
				} else if  (param.com == "bgf") {
					page.text.append(elin);
					nlin.src.text = QString("[BG:%0]").arg(param.pars["bg"]);
					page.text.append(nlin);
				} else if (param.com == "FLASH") {
					page.text.append(elin);
					nlin.src.text = QString("[FLASH BG:%0]").arg(param.pars["storage"]);
					page.text.append(nlin);
				} else if (param.com == "name") {
					tlin.src.name = param.pars["chara"];
					if (tlin.src.name == "地") tlin.src.name.clear();
				} else if (param.com == "jump") {
					nlin.src.text = QString("[jump %0:%1]").arg(param.pars["storage"]).arg(param.pars["target"]);
					page.text.append(nlin);
				} else if (param.com == "Msg") {
					tlin.src.name = param.pars["name"];
				} else if (param.com == "select") {
					page.text.append(elin);
					nlin.src.text = QString("[select]");
					page.text.append(nlin);
					nlin.src.name = param.pars["target1"];
					nlin.src.text = param.pars["text1"];
					if (!nlin.src.name.isEmpty()) page.text.append(nlin);
					nlin.src.name = param.pars["target2"];
					nlin.src.text = param.pars["text2"];
					if (!nlin.src.name.isEmpty()) page.text.append(nlin);
					nlin.src.name = param.pars["target3"];
					nlin.src.text = param.pars["text3"];
					if (!nlin.src.name.isEmpty()) page.text.append(nlin);
					nlin.src.name = param.pars["target4"];
					nlin.src.text = param.pars["text4"];
					if (!nlin.src.name.isEmpty()) page.text.append(nlin);
					nlin.src.name.clear();
				}
				nlin.src.text.clear();
				line.clear();
			} else if (line.startsWith("*")) {
				if (!line.contains("|")) {
					nlin.src.text = QString("== %0").arg(line.mid(1));
					page.text.append(elin);
					page.text.append(nlin);
					page.text.append(elin);
				}
				nlin.src.text.clear();
				line.clear();
			} else {
				pos = line.indexOf("[");
				if (pos < 0) {
					comline = line;
					line.clear();
				} else {
					comline = line.left(pos);
					line = line.mid(pos);
				}
				if (name) {
					tlin.src.name.append(comline);
				} else {
					tlin.src.text.append(comline);
				}
			}
		}
		if (!tlin.src.text.isEmpty()) {
			if (tlin.src.text.startsWith("「") && tlin.src.text.endsWith("」")) {
				tlin.src.text = tlin.src.text.mid(1, tlin.src.text.size() - 2);
			}
			tlin.src.name.remove("【");
			tlin.src.name.remove("】");
			page.text.append(tlin);
			tlin.src.name.clear();
		}
	}
	return page;
}
