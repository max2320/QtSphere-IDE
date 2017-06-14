#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <QFile>
#include <QFileInfo>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include "objects/qsifile.h"

class TextFile : public QWidget {
	Q_OBJECT
    Q_ENUMS(TextTypeEnum)
public:
    enum TextTypeEnum {PlainText,JavaScript,TypeScript,SGM,JSON,};
	TextFile(QWidget *parent = Q_NULLPTR);
    void readFile(QString filename);
    void readFile(QFile file);
    QString fileName();
    QString fileExtension();
    QString filePath();
    QFileInfo fileInfo();
	QsciScintilla* getTextWidget();
private slots:
    void onTextChanged();

protected:
	QsciScintilla* textWidget;


private:
	QWidget parentWidget;
	QFileInfo fi;
};

#endif // TEXTFILE_H