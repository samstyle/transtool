#include <QtGui>

#include "vars.h"
#include "classes.h"

//QString basedir=".";
QString basedir;

dNode dict;

//dict sysdict;
QList<formitem> forms;
QMap<QString, kanjitem> kanji;
QString dictpath;
QString formpath;

// tree dict

dNode* findChild(dNode* par, QChar ch) {
	dNode* res = NULL;
	if (par->childs.contains(ch)) {
		res = &par->childs[ch];
	}
/*
	int i;
	dNode* res = NULL;
	for (i = 0; i < par->childs.size(); i++) {
		if (par->childs[i].id == ch) {
			res = &par->childs[i];
			break;
		}
	}
*/
	return res;
}

dNode* findNode(QString word, int mkPath) {
	dNode* res = NULL;
	dNode* par = &dict;
	QChar ch;
	dNode newNode;
	newNode.childs.clear();
	newNode.words.clear();
	foreach(ch, word) {
		if (par->childs.contains(ch)) {
			par = &par->childs[ch];
			res = par;
		} else if (mkPath) {
			par->childs[ch] = newNode;
			par = &par->childs[ch];
			res = par;
		} else {
			word.clear();
			res = NULL;
		}
	}
	return res;
}

bool wrdCompare(dWord src, dWord dst) {
	if (src.word != dst.word) return false;
	if (src.read != dst.read) return false;
	if (src.type != dst.type) return false;
	return true;
}

int findWord(dNode* node, dWord wrd) {
	int idx = -1;
	for (int i = 0; i < node->words.size(); i++) {
		if (wrdCompare(node->words[i], wrd))
			idx = i;
	}
	return idx;
}

void addWord(dWord wrd) {
	dNode* nod = findNode(wrd.word, 1);
	if (findWord(nod, wrd) < 0) {
		nod->words.append(wrd);
	}
}

void updWord(dWord wold, dWord wnew) {
	dNode* nod = findNode(wold.word, 0);
	if (nod == NULL) return;
	int idx = findWord(nod, wold);
	if (idx < 0) return;
	nod->words[idx] = wnew;
}

void delWord(dWord wrd) {
	dNode* nod = findNode(wrd.word, 0);
	if (nod == NULL) return;
	for (int i = 0; i < nod->words.size(); i++) {
		if (wrdCompare(nod->words[i], wrd)) {
			nod->words.removeAt(i);
			i--;
		}
	}
}

// old

QString hiragana = QObject::trUtf8("あえういおんばべぶびぼだでづぢどがげぐぎごはへふひほかけくきこまめむみもなねぬにのぱぺぷぴぽられるりろさせすしそたてつちとやゆよざぜずじぞわをぁぇぅぃぉゃゅょっ");
QString katakana = QObject::trUtf8("アエウイオンバベブビボダデヅヂドガゲグギゴハヘフヒホカケクキコマメムミモナネヌニノパペプピポラレルリロサセスシソタテツチトヤユヨザゼズジゾワヲァェゥィォャュョッ");

bool iskana(QChar symbol) {
	return ((katakana.indexOf(symbol)!=-1) or (hiragana.indexOf(symbol)!=-1));
}

QString tohira(QString wrd) {
	for (int i=0; i<katakana.size(); i++) {
		wrd.replace(katakana.at(i),hiragana.at(i));
	}
	return wrd;
}

formfind katatohira(formfind ffn) {
	ffn.form = tohira(ffn.form);
	return ffn;
}

QList<dictfind> getChildsWords(dNode* nod) {
	dWord wrd;
	dNode* chld;
	QList<dictfind> res;
	dictfind df;
	QChar ch;
	foreach (ch, nod->childs.keys()) {
		chld = &nod->childs[ch];
		foreach(wrd, chld->words) {
			df.word = wrd;
			res.append(df);
		}
		res.append(getChildsWords(chld));
	}
	return res;
}

// TODO: if full = false, collect all words from leaf
QList<dictfind> scanWords(formfind nani, bool full) {
	QList<dictfind> res;
	QStringList frmz = nani.type.split(" ");
	QString srcWord = tohira(nani.form);
	QString dicWord;
	dictfind newitem;
	bool oka;
	int m;

	dNode* nod = findNode(nani.form, 0);

	if (nod == NULL) return res;
	for (int i = 0; i < nod->words.size(); i++) {
		dicWord = tohira(nod->words[i].word);
		if (srcWord == dicWord) {
			oka = frmz.contains("*");
			for (m = 0; m < frmz.size(); m++) {
				oka = oka || nod->words[i].type.contains(frmz[m]);
			}
			if (oka || !full) {
				newitem.word = nod->words[i];
				newitem.comment = nani.comment;
				res.append(newitem);
			}
		}
	}
	if (!full) {
		res.append(getChildsWords(nod));
	}
	return res;
}

QList<formfind> getbackforms(QString string,QString typz,int rec=0,QString lcom="") {
	QList<formfind> res,curfind;
	QStringList frmz = typz.split(" ",Qt::SkipEmptyParts); int m; bool oka,only;
	formfind newform;
	QString btype;
	newform.form=string; newform.type="*";res.append(newform);	// сама строчка = форма
	int i;
	for(i=0;i<forms.size();i++) {
		btype = forms[i].betype;
		only = (btype.indexOf("!")==0); if (only) btype.remove(0,1);		// only: betype начитается с !
		oka = ((typz.indexOf("*")!=-1) && !only);				// oka : в тек.типе есть * и нужный не начинается с !
		for (m=0;m<frmz.size();m++) oka = oka || (btype.indexOf(frmz[m])!=-1);
		if ((forms[i].end=="*" || string.endsWith(forms[i].end)) && (oka || (btype.indexOf("*")!=-1))) {
			newform.form = QString(string).left(string.size()-forms[i].remove).append(forms[i].add);
			newform.type = forms[i].type;
			newform.comment = lcom;
			if (lcom!="" && forms[i].comment!="") newform.comment.prepend(";");
			if (forms[i].comment!="") newform.comment.prepend(forms[i].comment);
			res.append(newform);
			if (rec<6) {
				curfind = getbackforms(newform.form,newform.type,rec+1,newform.comment);
				if (curfind.size()>1) {curfind.removeAt(0); res += curfind;}
			}
		}
	}
	return res;
}

void loadForms() {
	QStringList list;
	QString lastend;
	formitem newitem;
	bool ok;
	forms.clear();
	QString formpath = basedir + "forms4.jrf";
	QFile file(formpath);
	if (file.exists()) {
		file.open(QFile::ReadOnly);
		while (!file.atEnd()) {
			list = QDialog::trUtf8(file.readLine()).remove(QRegExp("[\r\n]")).split("\t");
			if (list.size()>4) {
				if (list[0]!="") lastend=list[0];
				if (list[0]=="" && lastend!="") list[0]=lastend;
				if (list[0]!="") {
					newitem.end = list[0];
					newitem.remove = list[1].toInt(&ok,10);
					newitem.add = list[2];
					newitem.betype = list[3];
					newitem.type = list[4];
					if (list.size()>5) {newitem.comment=list[5];} else {newitem.comment="";}
					if (ok) {forms.append(newitem);}
				}
			}
		}
		qDebug()<<"Forms total\t"<<forms.size();
		file.close();
	} else {
		qDebug()<<"Form file"<<formpath<<"doesn't exist. No forms aviable";
	}
}

void loadKanji(QString fname, int clr) {
	QString kanjpath = basedir + fname;
	QFile file(kanjpath);
	QStringList list;
	QString ch;
	kanjitem itm;
	if (clr) kanji.clear();
	int cnt = kanji.size();
	if (file.open(QFile::ReadOnly)) {
		while(!file.atEnd()) {
			list = QDialog::trUtf8(file.readLine()).remove(QRegExp("[\r\n]")).split("\t", Qt::SkipEmptyParts);
			if (list.size() > 1) {
				ch = list[0];
				itm.rd_on = list[1];
				if (list.size() > 2) {
					itm.rd_kun = list[2];
				} else {
					itm.rd_kun.clear();
				}
				kanji[ch] = itm;
			}
		}
		qDebug() << fname << "loaded :" << kanji.size() - cnt << "entries, total " << kanji.size();
		file.close();
	} else {
		qDebug() << "Can't open Kanji file" << kanjpath;
	}
}

void saveLeaf(QFile& file, dNode* nod) {
	dWord wrd;
	foreach (wrd, nod->words) {
		file.write(wrd.word.toUtf8());
		file.write("\t");
		file.write(wrd.read.toUtf8());
		file.write("\t");
		file.write(wrd.type.toUtf8());
		file.write("\t");
		file.write(wrd.trans.toUtf8());
		file.write("\r\n");
	}
	foreach (QChar key, nod->childs.keys()) {
		saveLeaf(file, &nod->childs[key]);
	}
}

void saveDict() {
	QFile file(dictpath);
	if (!file.open(QFile::WriteOnly)) return;
	saveLeaf(file, &dict);
	file.close();
}

void loadDict() {
	QFile file(dictpath);
	if (!file.open(QFile::ReadOnly)) return;
	dict.words.clear();
	dict.childs.clear();
	QStringList line;
	dWord word;
	int count = 0;
	while (!file.atEnd()) {
		line = QDialog::trUtf8(file.readLine()).remove(QRegExp("[\r\n]")).split("\t",Qt::SkipEmptyParts);
		if (line.size() > 3) {
			word.word = line[0];
			word.read = line[1];
			word.type = line[2];
			word.trans = line[3];
			addWord(word);
			count++;
		}
	}
	qDebug() << "Words total\t"<< count;
}

void reloadAll() {
	loadDict();
	loadForms();
	QDir dir(basedir);
	QStringList lst = dir.entryList(QStringList() << "*.jrk", QDir::Files);
	int f = 1;
	foreach(QString path, lst) {
		loadKanji(path, f);
		f = 0;
	}
}

void DictMain() {
	basedir = QDir::homePath().append("/.config/samstyle/jrdict/");
	formpath = basedir + "forms4.jrf";
	dictpath = basedir + "dict.dic";

	reloadAll();
}
