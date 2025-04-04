#include <QFileInfo>
#include <QHeaderView>
#include <QMetaObject>

#include "pdfsearch.hpp"

PDFSearchWidget::PDFSearchWidget(QWidget* parent)
                                : QWidget(parent)
{
    setupUI();
    setupSearchThread();
}

PDFSearchWidget::~PDFSearchWidget() 
{
    workerThread->quit();
    workerThread->wait();
}

void PDFSearchWidget::setDocuments(const QStringList& filepaths) 
{
    documentPaths = filepaths;
}

void PDFSearchWidget::addDocument(const QString& filepath) 
{
    if (!documentPaths.contains(filepath)) {
        documentPaths.append(filepath);
    }
}

void PDFSearchWidget::setupUI() 
{
    auto layout = new QVBoxLayout(this);

    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Enter search term...");
    layout->addWidget(searchInput);

    auto searchButton = new QPushButton("Search", this);
    connect(searchButton, &QPushButton::clicked, 
            this, &PDFSearchWidget::startSearch);
    layout->addWidget(searchButton);

    resultsTable = new QTableWidget(this);
    resultsTable->setColumnCount(2);
    resultsTable->setHorizontalHeaderLabels({"Document", "Page"});
    resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    layout->addWidget(resultsTable);

    setLayout(layout);
}

void PDFSearchWidget::setupSearchThread() 
{
    workerThread = new QThread(this);
    workerThread->start();
}

void PDFSearchWidget::clearSearchResults()
{
    pageButtons.clear();
    
    resultsTable->setRowCount(0);
}

void PDFSearchWidget::startSearch() 
{
    QString term = searchInput->text().trimmed();

    if (term.isEmpty()) 
        return;

    clearSearchResults();
    emit searchStarted();

    QtConcurrent::run([this, term]() 
    {
        for (const auto& filepath : documentPaths) {
            searchInDocument(filepath, term);
        }
        // Invokes the function with arguments in the event loop of the main thread.
        QMetaObject::invokeMethod(this, &PDFSearchWidget::searchFinished,
                                  Qt::QueuedConnection);
    });
}

void PDFSearchWidget::searchInDocument(const QString& filepath, const QString& term) 
{
    std::unique_ptr<Poppler::Document> doc(Poppler::Document::load(filepath));
    if (!doc){
        return;
    }

    QString lowercaseTerm = term.toLower();

    for (int pageNum = 0; pageNum < doc->numPages(); ++pageNum) 
    {
        std::unique_ptr<Poppler::Page> page(doc->page(pageNum));
        if (!page){
            continue;
        }

        QString pageText = page->text(QRect());
        if (pageText.toLower().contains(lowercaseTerm)) 
        {
            SearchResult result{filepath, pageNum + 1};
            QMetaObject::invokeMethod(this, [this, result]() 
            {
                addSearchResult(result);
            }, Qt::QueuedConnection);
        }
    }
}

void PDFSearchWidget::openPDFat(const QString &filePath, int page_num) 
{
   QProcess* process = new QProcess();

#ifdef Q_OS_WIN
    QStringList arguments;
    arguments << QString("-new-window");
    arguments << QString("-view");
    arguments << QString("single page");
    arguments << QString("-page");
    arguments << QString::number(page_num);
    arguments << filePath;
    
    process->start("C:\\Users\\zezo_\\AppData\\Local\\SumatraPDF\\SumatraPDF.exe", arguments);
#endif
}

void PDFSearchWidget::createPageButton(int row, const QString& filePath, int pageNumber) 
{
    auto pageButton = std::make_unique<QPushButton>(QString("%1").arg(pageNumber));

    // Get a raw pointer before moving
    QPushButton* buttonPtr = pageButton.get();
    
    connect(pageButton.get(), &QPushButton::clicked, this, 
            [this, filePath, pageNumber]() {
                openPDFat(filePath, pageNumber);
            });

    resultsTable->setCellWidget(row, 1, buttonPtr);

    pageButtons[buttonPtr] = std::move(pageButton);
}

void PDFSearchWidget::addSearchResult(const SearchResult& result) 
{
    int row = resultsTable->rowCount();
    resultsTable->insertRow(row);
    resultsTable->setItem(row, 0, new QTableWidgetItem(QFileInfo(result.filepath).fileName()));
    //resultsTable->setItem(row, 1, new QTableWidgetItem(QString::number(result.pageNumber)));
    createPageButton(row, result.filepath, result.pageNumber);
}
