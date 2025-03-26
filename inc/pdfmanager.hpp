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
#include <QStackedWidget>
#include <QDockWidget>
#include <QPropertyAnimation>
#include <QStyle>
#include <QList>
#include <QScrollBar>
#include <flowlayout.h>
#include <poppler-qt6.h>
#include <QPixmap>
#include <QImage>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <algorithm>

#include "pdfinfo.h"
#include "pdfsearch.hpp"
#include "utils.hpp"


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
    void setupPDFButton(PDFInfo &pdf, PDFCat &cat);

    void addNewPDF(PDFCat &category);
    void switchPDFView();
    void openPDF(PDFCat &cat, const QString &filePath);
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus, PDFCat &category);

    void loadData();
    void loadConfig();
    bool serializeData();
    bool deserializePDFCat(std::istream &in, PDFCat &cat);
    bool verifyFilePath(PDFInfo &pdf);
    void createSidebar();
    void toggleMainSidebar();
    void updateMainSidebarButtons();
    void toggleSecondaryDock(int index);
    void closeSecondaryDock();

        // Helper methods
    QWidget *createSecondaryPanel(int index);
  protected:
    void closeEvent(QCloseEvent *event) override;
  public:
    QWidget *centralWidget = nullptr;
    QHBoxLayout *mainLayout = nullptr;

    // Main content Area
    QWidget *contentArea;
    QVBoxLayout *contentLayout = nullptr;
    QScrollArea *categoriesArea = nullptr;
    QToolBox *toolbox = nullptr;

    // Menu Bar
    QMenuBar *menuBar = nullptr;
    QMenu *fileMenu = nullptr;
    QMenu *editMenu = nullptr;
    QMenu *viewMenu = nullptr;
    QMenu *helpMenu = nullptr;

    // Main Sidebar
    QWidget *mainSidebar = nullptr;
    QVBoxLayout *mainSidebarLayout = nullptr;
    QList<QPushButton *> mainSidebarButtons;
    QPushButton *toggleMainButton;
    bool isMainSidebarExpanded = false;
    int mainExpandedWidth = 200;
    int mainCollapsedWidth = 50;
    QPropertyAnimation *mainSidebarAnimation;

    struct ButtonData {
        QString text;
        QIcon icon; 
        QString contentTitle;
        QList<QString> subOptions;
    };
    QList<ButtonData> buttonDataList;

    // Sidebar dock widget
    QDockWidget *secondaryDock;
    bool isSecondaryDockVisible = false;
    QStackedWidget *secondaryStack;
    int currentSecondaryIndex = -1;
    int secondaryDockWidth = 250;


    // Search Bar
    QLineEdit *searchBar = nullptr;

    PDFSearchWidget* searchWidget;

    QFont *defaultFont;
    std::vector<PDFCat> PDFcats;
    QMap<QProcess *, QString> processToPDF;

    QString LastBrowsedPath;
    QString configPath = "pdfmanager.conf";

    QAction *exitAction = nullptr;

    int dummyIndex = -1;
    int lastIndex = -1;
    bool toolBoxChanged = false;
    bool firstProcess = true;
    bool dirty = false;
};