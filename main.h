#pragma once

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QtPlugin> // Required for static plugins
#include <QToolBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QObject>
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <algorithm>

#include "pdfinfo.h"

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin) // Windows

#if 0
QMap<int, QPair<QString, QWidget *>> removedItems;
void filterToolBoxItems(QToolBox *toolbox, const QString &searchText) 
{
    // First restore any previously removed items if we're doing a new search
    if (!removedItems.isEmpty()) {
        QMapIterator<int, QPair<QString, QWidget *>> it(removedItems);
        while (it.hasNext()) {
            it.next();
            int idx = it.key();
            QString title = it.value().first;
            QWidget *widget = it.value().second;

            // Insert at original index if possible, otherwise append
            if (idx < toolbox->count()) {
                toolbox->insertItem(idx, widget, title);
            } else {
                toolbox->addItem(widget, title);
            }
        }
        removedItems.clear();
    }

    // Now remove items that don't match the search
    if (!searchText.isEmpty()) {
        for (int i = toolbox->count() - 1; i >= 0; --i) {
            QString itemName = toolbox->itemText(i);
            bool match = itemName.contains(searchText, Qt::CaseInsensitive);

            if (!match) {
                // Store item before removing
                QWidget *widget = toolbox->widget(i);
                removedItems.insert(i, qMakePair(itemName, widget));

                // Remove item from toolbox but don't delete the widget
                toolbox->removeItem(i);
            }
        }
    }
}
#endif

class PDFManager : public QMainWindow 
{
    Q_OBJECT
  public:
    PDFManager(QWidget *parent = nullptr);
  private slots:
    void setFont(QFont *&defaultFont);
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus, PDFCat &category);
    void addNewPDF(PDFCat &category);
    void openPDF(PDFCat &cat, const QString &filePath);
    void createSearchbar();
    void filterToolBoxItems();
    void collapseAllToolBoxItems();
    void onToolBoxItemChanged(int index);
    void onToolBoxItemClicked(bool checked);
    void onAddCategoryClicked();
    void setupToolBoxConnections(); 
    void setupNewCat(PDFCat &cat);
    void setupNewPDF(PDFCat &category,
                                 QString &filePath); 
    void createMenuBar();
    bool serializeData();
    bool deserializePDFCat(std::istream &in, PDFCat &cat);
    void loadData();
    void loadConfig();
    void menuBarActionStub();
  protected:
    void closeEvent(QCloseEvent *event) override;
  public:
    QMenuBar *menuBar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QAction *exitAction;
    QWidget *centralWidget;
    QVBoxLayout *vbox;
    QToolBox *toolbox;
    int dummyIndex = -1;
    int lastIndex = -1;
    bool toolBoxChanged = false;
    QFont *defaultFont;
    QLineEdit *searchBar;
    std::vector<PDFCat> PDFcats;
    QMap<QProcess *, QString> processToPDF;
    QString LastBrowsedPath;
};