#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QThread>
#include <QtConcurrent>
#include <memory>

#include <poppler-qt6.h>

class PDFSearchWidget : public QWidget {
    Q_OBJECT

public:
    struct SearchResult {
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

    QStringList documentPaths;
    QLineEdit* searchInput;
    QTableWidget* resultsTable;
    QThread* workerThread;
};

