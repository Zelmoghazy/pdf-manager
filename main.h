#pragma once

#include <QMainWindow>
#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QtPlugin> // Required for static plugins
#include <QToolBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QLineEdit>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QSettings>
#include <QInputDialog>
#include <QThread>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <algorithm>

#include "pdfinfo.h"

#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin) // Windows
#endif   // QT_STATIC

class PDFManager : public QMainWindow 
{
    Q_OBJECT
  public:
    PDFManager(QWidget *parent = nullptr);
  private slots:
    void initApp();
    void setFont();

    void createMenuBar();
    void menuBarActionStub();

    void createSearchbar();

    void createCategoriesArea();
    void onAddCategoryClicked();
    void setupToolBoxConnections(); 
    void onToolBoxItemChanged(int index);
    void onToolBoxItemClicked(bool checked);
    void collapseAllToolBoxItems();
    void filterToolBoxItems();

    void setupNewCat(PDFCat &cat);
    void setupNewPDF(PDFCat &category, QString &filePath); 

    void addNewPDF(PDFCat &category);
    void openPDF(PDFCat &cat, const QString &filePath);
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus, PDFCat &category);

    void loadData();
    void loadConfig();
    bool serializeData();
    bool deserializePDFCat(std::istream &in, PDFCat &cat);
  protected:
    void closeEvent(QCloseEvent *event) override;
  public:
    QFont *defaultFont;
    std::vector<PDFCat> PDFcats;
    QMap<QProcess *, QString> processToPDF;

    // Menu Bar
    QMenuBar *menuBar = nullptr;
    QMenu *fileMenu = nullptr;
    QMenu *editMenu = nullptr;
    QMenu *viewMenu = nullptr;
    QMenu *helpMenu = nullptr;

    QWidget *centralWidget = nullptr;
    QVBoxLayout *vbox = nullptr;
    QScrollArea *categoriesArea = nullptr;
    QToolBox *toolbox = nullptr;

    // Search Bar
    QLineEdit *searchBar = nullptr;

    QString LastBrowsedPath;
    QString configPath = "pdfmanager.conf";

    QAction *exitAction = nullptr;

    int dummyIndex = -1;
    int lastIndex = -1;
    bool toolBoxChanged = false;
    bool firstProcess = true;
};