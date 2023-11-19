#pragma once

#include <QString>

struct dWord {
	QString word;
	QString read;
	QString type;
	QString trans;
};

struct dNode {
//	QChar id;
//	QList<dNode> childs;
	QMap<QChar, dNode> childs;
	QList<dWord> words;
};

struct kanjitem {
	QString rd_kun;
	QString rd_on;
};

struct formitem {
	QString end;
	int remove;
	QString add;
	QString betype;
	QString type;
	QString comment;
};

struct formfind {
	QString form;
	QString type;
	QString comment;
};

struct dictfind {
	int begin;
	int len;
	QString src;
	dWord word;
	QString comment;
};

extern QList<dictfind> findres;

void loadDict();
void saveDict();
void loadKanji(QString, int = 0);
void loadForms();

formfind katatohira(formfind);
bool wrdCompare(dWord, dWord);

QList<dictfind> scanWords(formfind, bool);
void addWord(dWord);
void updWord(dWord, dWord);
void delWord(dWord);

QList<dictfind> scanWords(formfind,bool);
QList<formfind> getbackforms(QString, QString, int, QString);
