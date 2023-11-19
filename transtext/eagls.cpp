// eagls script

#include "filetypes.h"

QStringList parseEAGLCom(QString& line) {
	QStringList res;
	int pos1 = line.indexOf("(");
	int pos2 = line.indexOf(")");
	if ((pos1 < 0) || (pos2 < pos1)) {
		res << line;
		line.clear();
	} else {
		QString pars = line.mid(pos1 + 1, pos2 - pos1 - 1);
		res = pars.split(",",Qt::SkipEmptyParts);
		for (int i = 0; i < res.size(); i++) {
			res[i] = res[i].remove("\"");
		}
		res.prepend(line.left(pos1));
		line = line.mid(pos2 + 1);
	}
	return res;
}

QMap<QString,QString> eamap;

QString eagIfPars(QStringList prs) {
	QString ifpar;
	if (prs.size() > 3) {
		ifpar = QString("(%1 %3 %2)").arg(prs[1]).arg(prs[2]).arg(prs[3]);
		prs.removeFirst();
		prs.removeFirst();
		prs.removeFirst();
		while (prs.size() > 3) {
			ifpar.append(QString(" %4 (%1 %3 %2)").arg(prs[1]).arg(prs[2]).arg(prs[3]).arg(prs.last()));
			prs.removeFirst();
			prs.removeFirst();
			prs.removeFirst();
		}
	} else {
		ifpar = prs.join(",");
	}
	return ifpar;
}

void parseEAGLine(QString line, TPage* page, QString* name) {
	if (line.isEmpty()) return;
	QStringList prs;
	QStringList ddz;
	QString num;
	TLine tlin;
	tlin.flag = 0;
	int pos;
	if (line.startsWith("$")) {			// $label
		tlin.src.text = QString("== ").append(line);
		tlin.type = TL_LABEL;
		page->text.append(tlin);
	} else if (line.startsWith("#")) {
		*name = line.split(QRegularExpression("[#=]"),Qt::SkipEmptyParts).first();
	} else if (QString("123456789").contains(line.at(0))) {
		tlin.type = TL_COM;
		num = line;
		prs = parseEAGLCom(line);
		if (prs[0] == "52") {
			//tlin.src.text = QString("[%1 = %2]").arg(prs[1]).arg(prs[2]);
			//page->text.append(tlin);
			eamap[prs[1]] = prs[2];
		} else if (prs[0] == "41") {
			tlin.type = TL_TEXT;
			tlin.src.text = QString("[ jp %1 # %2]").arg(prs[2]).arg(prs[1]);
			page->text.append(tlin);
		} else if (prs[0] == "42") {
			if (prs[2] == "00Haikei.dat") {
				if (prs[1] == "Black") num = "black"; else num = eamap["_Haikei"];
				tlin.type = TL_TEXT;
				tlin.src.text = QString("[BG : %1]").arg(num);
				page->text.append(tlin);
			} else if (((prs[1] == "DrawBG") || (prs[1] == "DrawCG")) && (prs[2] == "00Draw.dat")) {
				tlin.type = TL_TEXT;
				tlin.src.text = QString("[BG : %1]").arg(eamap["_GRPName"]);
				page->text.append(tlin);
			} else if (((prs[1] == "Cg") || (prs[1] == "Bg")) && (prs[2] == "00Draw.dat")) {
				tlin.type = TL_TEXT;
				tlin.src.text = QString("[BG : %1]").arg(eamap["_DrawName"]);
				page->text.append(tlin);
/*			} else if ((prs[1] == "DrawChar") && (prs[2] == "00DrawChar.dat")) {
				tlin.type = TL_TEXT;
				if (eamap.count("_CharName") > 0) {
					tlin.src.text = QString("[CH : %1]").arg(eamap["_CharName"]);
				} else {
					tlin.src.text = QString("[CH : %1]").arg(eamap["_GRPName"]);
				}
				page->text.append(tlin);
*/
			} else if ((prs[2] == "00CGIcon.dat") && (prs[1] == "CGIcon")) {
				tlin.type = TL_TEXT;
				tlin.src.text = QString("[BG : %1]").arg(eamap["_Haikei"]);
				page->text.append(tlin);
			} else if ((prs[2] == "Choices.dat") && (prs[1].startsWith("Choices"))) {
				tlin.type = TL_TEXT;
				tlin.src.text.clear();
				page->text.append(tlin);
				tlin.src.text = "[select]";
				page->text.append(tlin);
				ddz.clear();
				ddz << "_Character" << "_Character2" << "_Character3" << "_Character4" << "_Character5" << "_Character6";
				foreach (num, ddz) {
					if (eamap.count(num) > 0) {
						if (!eamap[num].isEmpty()) {
							tlin.src.text = QString(eamap[num]);
							page->text.append(tlin);
						}
					}
				}
				tlin.src.text.clear();
				page->text.append(tlin);
				eamap.clear();
			} else if (prs[2].endsWith("Select.dat") || (prs[2] == "95SelectH.dat")) {
				tlin.type = TL_TEXT;
				tlin.src.text.clear();
				page->text.append(tlin);
				tlin.src.text = "[select]";
				page->text.append(tlin);
				ddz.clear();
				ddz << "_SelStr0" << "_SelStr1" << "_SelStr2" << "_SelStr3" << "_SelStr4" << "_SelStr5";
				ddz << "_SelChar0" << "_SelChar1" << "_SelChar2" << "_SelChar3" << "_SelChar4" << "_SelChar5";
				foreach (num, ddz) {
					if (eamap.count(num) > 0) {
						if (!eamap[num].isEmpty()) {
							tlin.src.text = QString(eamap[num]);
							page->text.append(tlin);
						}
					}
				}
				tlin.src.text.clear();
				page->text.append(tlin);
				eamap.clear();
			} else if ((prs[2] == "00MovPlay.dat") || (prs[2] == "00HMovPlay.dat") || (prs[2] == "02Move_BG.dat")) {
				tlin.type = TL_TEXT;
				if (eamap.count("_MOVName")) tlin.src.text = QString("[MOV : %1]").arg(eamap["_MOVName"]);
				else if (eamap.count("_MovieFileName")) tlin.src.text = QString("[MOV : %1]").arg(eamap["_MovieFileName"]);
				else if (eamap.count("_GRPName")) tlin.src.text = QString("[BG : %1]").arg(eamap["_GRPName"]);
				page->text.append(tlin);
			} else if ((prs[1] == "DrawCGEX") && (eamap.count("_GRPName") > 0)) {
				tlin.src.text = QString("[BigBG : %1]").arg(eamap["_GRPName"]);
				page->text.append(tlin);
			} else if ((prs[2] == "90examine.dat") && (prs[1] == "examine")) {
				tlin.type = TL_TEXT;
				tlin.src.text.clear();
				page->text.append(tlin);
				tlin.src.text = "[select]";
				page->text.append(tlin);
				ddz.clear();
				ddz << "_SQ_Sel01Text" << "_SQ_Sel02Text" << "_SQ_Sel03Text" << "_SQ_Sel04Text" << "_SQ_Sel05Text";
				foreach(num,ddz) {
					if (eamap.count(num) > 0) {
						if (!eamap[num].isEmpty()) {
							tlin.src.text = QString(eamap[num]);
							page->text.append(tlin);
							eamap[num].clear();
						}
					}
				}
				eamap.clear();
			}
		} else if (prs[0].startsWith("120,")) {
			pos = num.indexOf("(");
			num = num.mid(4,pos-4);
			tlin.src.text = QString("[ if.%1 %2 ]").arg(num).arg(eagIfPars(prs));
			tlin.type = TL_TEXT;
			page->text.append(tlin);
			pos = line.indexOf("{");
			line = line.mid(pos + 1);
		} else if (prs[0].startsWith("122,")) {
			pos = num.indexOf("(");
			num = num.mid(4,pos-4);
			tlin.type = TL_TEXT;
			tlin.src.text = QString("[ elseif.%1 %2 ]").arg(num).arg(eagIfPars(prs));
			page->text.append(tlin);
			pos = line.indexOf("{");
			line = line.mid(pos + 1);
		} else if (prs[0].startsWith("121,")) {
			pos = line.indexOf("{");
			num = num.mid(4,pos-4);
			tlin.type = TL_TEXT;
			tlin.src.text = QString("[ else.%1 ]").arg(num);
			page->text.append(tlin);
			line = line.mid(pos + 1);
		}
		parseEAGLine(line,page,name);
	} else if (line.startsWith("}")) {
		pos = line.indexOf(";");
		if (pos < 0) {
			num = line.mid(1);
		} else {
			num = line.mid(1,pos-1);
		}
		tlin.src.text = QString("[ endif.%1 ]").arg(num);
		tlin.type = TL_TEXT;
		page->text.append(tlin);
		if (pos < 0) {
			line.clear();
		} else {
			line = line.mid(pos+1);
			parseEAGLine(line,page,name);
		}
	} else if (line.startsWith("_")) {

	} else if (line.startsWith("&")) {
		int pos = line.indexOf("\"");
		if (pos > 0) line = line.mid(pos + 1);
		tlin.src.name = *name;
		tlin.src.name.remove("　");
		tlin.src.name.replace(":NameSuffix", "俺");
		tlin.src.name = tlin.src.name.split(",").first();
		name->clear();
		pos = line.indexOf("\"");
		if (pos > 0) {
			tlin.src.text = line.left(pos);
			if (tlin.src.text.startsWith("「") && tlin.src.text.endsWith("」")) {
				tlin.src.text.remove(0,1);
				tlin.src.text.remove(tlin.src.text.size()-1,1);
			}
			tlin.src.text.remove("(e)");
			tlin.type = TL_TEXT;
			tlin.src.text.remove("　");
			page->text.append(tlin);
			line = line.mid(pos + 1);
		}
		parseEAGLine(line,page,name);
	}
}

TPage loadEAGLS(QString fnam, int) {
	TPage page;
	QFile file(fnam);
	if (!file.open(QFile::ReadOnly)) return page;
	QString line;
	QString name;
	QTextCodec* codec = QTextCodec::codecForName("Shift-JIS");
	name.clear();
	while (!file.atEnd()) {
		line = codec->toUnicode(file.readLine());
		line.remove("\r");
		line.remove("\n");
		line.remove("\t");
		line.remove("(E)");
		line.remove("(e)");
		parseEAGLine(line,&page,&name);
	}
	return page;
}
