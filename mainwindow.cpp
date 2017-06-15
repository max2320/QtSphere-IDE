#include <QProcess>
#include <QFileDialog>
#include <QMenu>
#include <QCloseEvent>
#include <QTime>
#include <QTextStream>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <assert.h>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>


#include "mainwindow.h"
#include "util.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "settingswindow.h"
#include "objects/spherefile.h"
#include "modifiedfilesdialog.h"
//#include "objects/textfile.h"

MainWindow* MainWindow::_instance = NULL;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    Q_ASSERT(!_instance);
    _instance = this;
    ui->setupUi(this);

	this->syntaxMode = new QComboBox(ui->statusBar);
    QStringList syntaxes_str;
    syntaxes_str << "Plaintext" << "Javascript" << "TypeScript" << "SGM Project File" << "JSON";
	this->syntaxMode->addItems(syntaxes_str);
	this->syntaxMode->setStyleSheet("margin-right: 8px;");

	ui->statusBar->addWidget(new QLabel("Ready."));
	ui->statusBar->addPermanentWidget(this->syntaxMode);

	ui->toolbarNewButton->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
	ui->toolbarOpenButton->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
	ui->toolbarSaveButton->setIcon(this->style()->standardIcon(QStyle::SP_DriveFDIcon));

	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	QTableWidgetItem* h = new QTableWidgetItem("Task");
	h->setCheckState(Qt::Unchecked);

	ui->tableWidget->setItem(0, 1, h);

	ui->centralWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->centralWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(showContextMenu(const QPoint&)));

	ui->openFileTabs->setTabsClosable(true);
	//prepareForText();
	openProject("");


	QList<int> mainSplitterList;
	QList<int> consoleSplitterList;
	mainSplitterList << 150 << this->width()-150;
	consoleSplitterList << 350 << 1;
	ui->splitter->setSizes(mainSplitterList);
	ui->consoleSplitter->setSizes(consoleSplitterList);
	this->signalMapper = new QSignalMapper(this) ;
}

QString MainWindow::getStatus() {
    foreach(QObject* child, ui->statusBar->children()) {
        if(getWidgetType(child) == "QLabel") {
            QLabel* lbl = qobject_cast<QLabel*>(child);
            return lbl->text();
        }
    }
    return "";
}

void MainWindow::setStatus(QString status) {
	QLabel* msg = ui->statusBar->findChild<QLabel *>();
	if(msg == nullptr) {
		return;
	}
	msg->setText(status);
}

MainWindow::~MainWindow() {
	disconnect(ui->centralWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(showContextMenu(const QPoint&)));
	delete ui;
}

MainWindow* MainWindow::instance() {
    if(!_instance)
        _instance = new MainWindow;
    return _instance;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    /*event->accept(); // leaving this here until we can reliably check to see if an open file has been midified
	event->ignore();
	if(handleModifiedFiles() > 0)
        event->accept();*/
}

void MainWindow::console(QVariant s, int which) {
	QString prefix = "[" + QTime::currentTime().toString("h:mm:s ap") + "] ";
    QString line = prefix + s.toString();
	switch (which) {
		case 0:
            ui->buildLogText->insertPlainText(line);
		break;
		case 1:
            ui->consoleText->appendPlainText(line);
		break;
		case 2:
            // exceptions, probably make a QTableView
		break;
		default:
            ui->consoleText->insertPlainText(line);
		break;
	}
}

void MainWindow::on_actionAbout_triggered() {
	AboutDialog aboutDialog;
	aboutDialog.setModal(true);
	aboutDialog.exec();
}

void MainWindow::on_actionSphere_2_0_API_triggered() {
	QProcess process;
}


void MainWindow::on_actionMiniRT_API_triggered() {
	QProcess process;
}

void MainWindow::onTextWidgetChanged() {
	QObject* editorObject = QObject::sender();
	//QsciScintilla* editor = static_cast<QsciScintilla*>(editorObject);
	//editor->setMarginWidth(0, editor->fontMetrics().width(QString::number(editor->lines())) + 6);
}

void MainWindow::saveCurrentTab() {
	if(ui->openFileTabs->count() == 0) return;
	QObjectList tabChildren = ui->openFileTabs->currentWidget()->children();
	QWidget* currentWidget = ui->openFileTabs->currentWidget();
	if(QString(currentWidget->metaObject()->className()) == "QsciScintilla") {
		QsciScintilla* currentEditor = static_cast<QsciScintilla*>(ui->openFileTabs->currentWidget());
		QString saveFileName = QFileDialog::getSaveFileName(this,
								"Save script", "","Script (*.js);;Text file (*.txt);;All files (*)");
		QFile saveFile(saveFileName);
		if(!saveFile.open(QIODevice::WriteOnly)) {
			this->console("Failed saving file: " + saveFile.errorString());
			return;
		} else {
			QTextStream out(&saveFile);
			out.setCodec("UTF-8");
			out << currentEditor->text();
			saveFile.flush();
			saveFile.close();
		}
	}
}

void MainWindow::showContextMenu(const QPoint &pos) {
	QPoint globalPos = ui->treeWidget->mapToGlobal(pos);

	QMenu rMenu;
	QWidget* currentWidget = ui->centralWidget->childAt(pos);

	if(currentWidget->objectName() == "treeWidget")
		rMenu.addAction("treeWidget");
	else if(currentWidget->objectName() == "treeView")
		rMenu.addAction("treeView");
	else
		rMenu.addAction(getWidgetType(currentWidget));

	rMenu.addAction("Menu item 1");
	rMenu.addAction(ui->toolbarPlayGame);

	QAction* selectedItem = rMenu.exec(globalPos);
	if (selectedItem) {
		this->console("selectedItem", 1);
	} else {
		this->console("notSelectedItem", 1);
	}
}

void MainWindow::openFile(QString fileName) {
	QString fn;
    if(fileName == "") {
		fn = QFileDialog::getOpenFileName(this, "Open file", QDir::currentPath(),
												"All supported files (*.sgm *.txt *.js *.ts *.rmp *.rss *.rws);;"
												"Sphere projects (*.sgm);;"
												"Script files (*.js *.ts *.coffee);;"
                                                "Text files (*.txt);;"
												"All files (*.*)");
	} else {
		fn = fileName;
	}

	if(fn == "") return;

	QFile* file = new QFile(fn);
	QString fileExtension;
	if (!file->open(QIODevice::ReadWrite | QIODevice::Text)) return;
	QFileInfo fi = QFileInfo(fn);
	QByteArray bytes = file->readAll();


	QsciScintilla* newTextEdit = new QsciScintilla(this);
	newTextEdit->setText(bytes);
	this->setupTextBox(newTextEdit, "JavaScript");

	newTextEdit->setObjectName("textEdit" + QString::number(ui->openFileTabs->count()) );
	this->openFiles.append(newTextEdit);
	ui->openFileTabs->insertTab(0, newTextEdit, fi.fileName());
	ui->openFileTabs->setCurrentIndex(0);
}

void MainWindow::openProject(QString fileName) {
    this->console("Loading project: " + fileName);
}

void MainWindow::handleModifiedFiles() {
    if(this->openFiles.count() == 0) return;
    ModifiedFilesDialog mfd(this);
    int num_modified = 0;
    QList<QTextEdit *> openEditors = ui->openFileTabs->findChildren<QTextEdit *>();

    for(int t = 0; t < openEditors.count(); t++) {
       /* if(openEditors.at(t)->document()->toPlainText() != this->openFiles.at(t)->text) {
            mfd.addModifiedItem(this->openFiles.at(t));
            num_modified++;
        }*/
    }
    if(num_modified > 0)
        mfd.exec();
}

void MainWindow::setupTextBox(QsciScintilla* box, QString type) {
	QFont boxfont("Courier", 10);
	// type will eventually change the lexer
	box->setFont(boxfont);
	box->setMarginsFont(box->font());

	box->setMarginWidth(0, QFontMetrics(box->font()).width(QString::number(box->lines()))+6);
	box->setMarginLineNumbers(0, true);
	box->setMarginsBackgroundColor(QColor("#cccccc"));

	box->setCaretLineVisible(true);
	box->setCaretLineBackgroundColor(QColor("#ffe4e4"));

	QsciLexerJavaScript *lexer = new QsciLexerJavaScript();
	lexer->setDefaultFont(boxfont);
	lexer->setFoldComments(true);
	box->setLexer(lexer);

	QsciScintilla::FoldStyle state = static_cast<QsciScintilla::FoldStyle>((!box->folding())*5);
	if(!state) box->foldAll(false);
	box->setFolding(state);
	connect(box, SIGNAL(textChanged()), this, SLOT(onTextWidgetChanged()));
}

void MainWindow::on_actionExit_triggered() {
    handleModifiedFiles();
    this->close(); // just temporary until I have a way to test if the currently open file has been modified
    //if(handleModifiedFiles() > 0) this->close();
}


void MainWindow::on_actionOpen_triggered() {
	this->openFile();
}

void MainWindow::on_actionConfigure_QtSphere_IDE_triggered() {
    SettingsWindow settingsWindow(this);
    settingsWindow.setModal(true);
    settingsWindow.exec();
}

void MainWindow::on_toolbarNewButton_triggered() {
	QsciScintilla* newTextEdit = new QsciScintilla(this);
	this->setupTextBox(newTextEdit, "JavaScript");

	newTextEdit->setObjectName("textEdit" + QString::number(ui->openFileTabs->count()) );

	this->openFiles.append(newTextEdit);
	ui->openFileTabs->insertTab(0, newTextEdit, "<Untitled>");
	ui->openFileTabs->setCurrentIndex(0);
}

void MainWindow::on_toolbarSaveButton_triggered() {
	this->saveCurrentTab();
}

void MainWindow::on_toolbarOpenButton_triggered() {
    this->openFile();
}

void MainWindow::on_openFileTabs_tabCloseRequested(int index) {
	this->openFiles.removeAt(index);
    ui->openFileTabs->removeTab(index);
}

void MainWindow::on_actionUndo_triggered() {
    // incredibly hacky? Yes, but it works.
    ui->openFileTabs->children().at(ui->openFileTabs->currentIndex())->findChildren<QTextEdit *>().at(0)->undo();
}

void MainWindow::on_openFileTabs_currentChanged(int index) {
	if(this->openFiles.count() > 0) {
        /*switch(this->openFiles.at(index)->fileType) {
			case QSIFile::JavaScript:
				this->syntaxMode->setCurrentIndex(1);
			break;
			default:
				this->syntaxMode->setCurrentIndex(0);
			break;
        }*/
	}
}
