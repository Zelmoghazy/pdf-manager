#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QThread>
#include <QtConcurrent>
#include <memory>
#include <unordered_map>

#include <poppler-qt6.h>

class PDFSearchWidget : public QWidget 
{
    Q_OBJECT

public:
    struct SearchResult 
    {
        QString filepath;
        int pageNumber;
    };

    explicit PDFSearchWidget(QWidget* parent = nullptr);
    ~PDFSearchWidget();

    void setDocuments(const QStringList& filepaths);
    void addDocument(const QString& filepath);

signals:
    void searchStarted();
    void searchFinished();

private slots:
    void startSearch();

private:
    void setupUI();
    void setupSearchThread();
    void searchInDocument(const QString& filepath, const QString& term);
    void addSearchResult(const SearchResult& result);
    void clearSearchResults();
    void createPageButton(int row, const QString& filePath, int pageNumber);
    void openPDFat(const QString &filePath, int page_num);

    std::unordered_map<QPushButton*, std::unique_ptr<QPushButton>> pageButtons;
    QStringList documentPaths;
    QLineEdit* searchInput;
    QTableWidget* resultsTable;
    QThread* workerThread;
};

