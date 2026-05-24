#pragma once

#include <QString>

struct dWord {
	QString word;
	QString read;
	QString type;
	QString trans;
	QString dict;	// filepath of dict it is from
};

struct dNode {
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

void saveDicts();
void loadKanji(QString, int = 0);
void loadForms();
void reloadAll();

formfind katatohira(formfind);
bool wrdCompare(dWord, dWord);

QList<dictfind> scanWords(formfind, bool);
void addWord(dWord);
void updWord(dWord, dWord);
void delWord(dWord);

QList<dictfind> scanWords(formfind,bool);
QList<formfind> getbackforms(QString, QString, int, QString);
