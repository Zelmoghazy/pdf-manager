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

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin) // Windows

#define MAX(a, b) (a>b)?a:b

struct PDFInfo
{
    bool found = false;
    std::string file_name;
    std::string file_path;
    int page_num = 0;
    std::string mode;

    QPushButton *button = nullptr;

    PDFInfo() {}
    PDFInfo(std::string filename) : file_name(filename){}

    // Dumbest way I could think of right now, should improve later 
    void parseSumatraSettings(const std::string &settings_path) 
    {
        std::ifstream file(settings_path, std::ios::binary);

        if (!file) {
            std::cerr << "Couldn't read file !\n";
            return;
        }
        
        // read with 8k chunks
        constexpr size_t buf_size = 8192;
        char buffer[buf_size];

        file.rdbuf()->pubsetbuf(buffer, buf_size);

        constexpr char file_states[] = "FileStates";
        constexpr size_t file_states_len = 10;

        std::string line;
        line.reserve(512);

        // Find FileStates
        bool found_filestates = false;
        while (std::getline(file, line))
        {
            if ((line.size() >= file_states_len))
            {
                if (memcmp(&line[0], file_states, file_states_len) == 0) 
                {
                    found_filestates = true;
                    break;
                }
            }
        }

        // Shouldnt happen !
        if (!found_filestates)
        {
            std::cerr << "Couldn't find FileStates in sumatra settings file !\n";
            return;
        }

        // start parsing
        constexpr char file_path[] = "FilePath";
        constexpr size_t file_path_len = 8; 

        constexpr char page_no[] = "PageNo";
        constexpr size_t page_no_len = 6;
        
        int bracket_level = 1;
        bool insidePdfEntry = false;

        while (std::getline(file, line))
        {
            // skip whitespace
            size_t pos = 0;
            while (pos < line.size() && std::isspace((unsigned char)line[pos]))
                ++pos;

            size_t end = line.size();
            while (end > pos && std::isspace((unsigned char)line[end - 1]))
                --end;

            // Check if line is empty after trimming
            if (pos >= end)
                continue; // skip empty lines

            if (line[pos] == '[' || line[end-1] == '[') 
            {
                bracket_level++;
                if (bracket_level == 2)  // we are inside a file state
                {
                    insidePdfEntry = true;
                }
                continue;
            }
            else if (line[pos] == ']')
            {
                bracket_level--;
                if (bracket_level == 1 && insidePdfEntry)
                {
                    if (found) {
                        return;
                    }
                    insidePdfEntry = false;
                }
                continue;
            }

            // start parsing entries
            if (insidePdfEntry)
            {
                // Check filename first for a match
                if ((pos <= (line.size() - file_path_len)) &&  
                    (memcmp(&line[pos], file_path, file_path_len) == 0)) 
                {
                    size_t path_start = line.find('=', pos);

                    if (path_start != std::string::npos)
                    {
                        // Get the full path
                        std::string fullPath = line.substr(path_start+1);

                        // Trim whitespace
                        size_t startPos = 0;
                        while (startPos < fullPath.length() && std::isspace((unsigned char)(fullPath[startPos]))) {
                            startPos++;
                        }
                        
                        size_t endPos = fullPath.length();
                        while (endPos > startPos && std::isspace((unsigned char)(fullPath[endPos - 1]))) {
                            endPos--;
                        }

                        size_t lastSlash1 = fullPath.rfind('\\', endPos - 1);
                        size_t lastSlash2 = fullPath.rfind('/' , endPos - 1);
                        size_t lastSlash;

                        if (lastSlash1 == std::string::npos)
                        {
                            lastSlash = lastSlash2;
                        } 
                        else if (lastSlash2 == std::string::npos)
                        {
                            lastSlash = lastSlash1;
                        } else {
                            lastSlash = MAX(lastSlash1, lastSlash2);
                        }

                        if (lastSlash != std::string::npos)
                        {
                            lastSlash++;
                            if (fullPath.compare(lastSlash, endPos - lastSlash,
                                                 file_name, 0, file_name.length()) == 0) 
                            {
                                // found a match
                                found = true;
                            } 
                        }
                    }
                }

                // Continue parsing if found only
                if(found)
                {
                    if ((pos <= line.size() - page_no_len) && 
                        memcmp(&line[pos], page_no, page_no_len) == 0) 
                    {
                        size_t path_start = line.find('=', pos);

                        if (path_start != std::string::npos)
                        {
                            std::string PageNo = line.substr(path_start+1);

                            // Trim whitespace
                            size_t startPos = 0;
                            while (startPos < PageNo.length() && 
                                   std::isspace((unsigned char)(PageNo[startPos]))) 
                            {
                                startPos++;
                            }
                            
                            size_t endPos = PageNo.length();
                            while (endPos > startPos &&
                                   std::isspace((unsigned char)(PageNo[endPos - 1]))) 
                            {
                                endPos--;
                            }

                            page_num = std::stoi(PageNo.substr(startPos, endPos - startPos));
                            return;
                        }
                    } 
                }
            }
        }
    }
};

static QWidget *embedIntoHBoxLayout(QWidget *w, int margin = 5)
{
    auto result = new QWidget;
    auto layout = new QHBoxLayout(result);
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->addWidget(w);
    return result;
}

static QWidget *embedIntoVBoxLayout(QWidget *w, int margin = 5) 
{
    auto result = new QWidget;
    auto layout = new QVBoxLayout(result);
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->addWidget(w);
    return result;
}

//QMap<int, QPair<QString, QWidget *>> removedItems;
//void filterToolBoxItems(QToolBox *toolbox, const QString &searchText) 
//{
//    // First restore any previously removed items if we're doing a new search
//    if (!removedItems.isEmpty()) {
//        QMapIterator<int, QPair<QString, QWidget *>> it(removedItems);
//        while (it.hasNext()) {
//            it.next();
//            int idx = it.key();
//            QString title = it.value().first;
//            QWidget *widget = it.value().second;
//
//            // Insert at original index if possible, otherwise append
//            if (idx < toolbox->count()) {
//                toolbox->insertItem(idx, widget, title);
//            } else {
//                toolbox->addItem(widget, title);
//            }
//        }
//        removedItems.clear();
//    }
//
//    // Now remove items that don't match the search
//    if (!searchText.isEmpty()) {
//        for (int i = toolbox->count() - 1; i >= 0; --i) {
//            QString itemName = toolbox->itemText(i);
//            bool match = itemName.contains(searchText, Qt::CaseInsensitive);
//
//            if (!match) {
//                // Store item before removing
//                QWidget *widget = toolbox->widget(i);
//                removedItems.insert(i, qMakePair(itemName, widget));
//
//                // Remove item from toolbox but don't delete the widget
//                toolbox->removeItem(i);
//            }
//        }
//    }
//}

class PDFManager : public QMainWindow 
{
    Q_OBJECT
  public:
    PDFManager(QWidget *parent = nullptr);
  private slots:
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void addNewPDF();
    void openPDF(const QString &filePath);
    void filterToolBoxItems();
    void collapseAllToolBoxItems(QToolBox *toolbox);
  private:
    QWidget *centralWidget;
    QVBoxLayout *vbox;
    QToolBox *toolbox;
    QLineEdit *searchBar;
    std::vector<PDFInfo> PDFFiles;
    QMap<QProcess *, QString> processToPDF;
};

PDFManager::PDFManager(QWidget *parent)
{
    setWindowTitle("PDF Manger");
    setWindowFlags(Qt::Window);
    setWindowIcon(QIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\icon.png")); 

    QFont defaultFont("Arial", 18);
    QApplication::setFont(defaultFont);
    resize(800, 400);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto main_layout = new QVBoxLayout(centralWidget);
        
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");
    searchBar->setFont(defaultFont);

    QObject::connect(searchBar, &QLineEdit::textChanged,
                     this, &PDFManager::filterToolBoxItems);

    main_layout->addWidget(searchBar);

    toolbox = new QToolBox(this);
    toolbox->setStyleSheet(
        "QToolBox {"
        "    background-color: #1e1e1e;"   // Set the background color of the
        "}"
        "QToolBox::tab {"
        "    background-color: #3f5570;"   
        "    color: white;"                
        "    border-radius: 4px;"          
        "}"
        "QToolBox::tab:selected {"         
        "    background-color: #2d5bb9;"   
        "    font-weight: bold;"           
        "}"
        "QToolBox::tab:hover {"            
        "    background-color: #3d73c9;"   
        "}"
    );

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: #1e1e1e;"   
        "    border: none;"                
        "}");

    auto container = new QWidget(this);
    container->setStyleSheet(
        "QWidget {"
        "    background-color: #1e1e1e;" 
        "}");
    vbox = new QVBoxLayout(container);
    vbox->setSpacing(5);   
    vbox->setContentsMargins(30, 10, 30, 10);   

    toolbox->addItem(container, "Programming");

    scrollArea->setWidget(toolbox);
    
    // Add and center an add button to add new pdfs
    auto addButton = new QPushButton("+");
    addButton->setFixedSize(30, 30);

    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(addButton);
    centerLayout->addStretch();

    vbox->addLayout(centerLayout);
    // Add the "+" button before the stretch
    vbox->addStretch(1);   

    connect(addButton, &QPushButton::clicked,
            this, &PDFManager::addNewPDF);

        
    collapseAllToolBoxItems(toolbox);


    main_layout->addWidget(scrollArea);
}

void PDFManager::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* process = qobject_cast<QProcess*>(sender());

    if (!process) 
        return;
        
    if (exitStatus == QProcess::CrashExit) 
    {
        QMessageBox::critical(this, "Process Failed",
                                    "The running process crashed!");
        return;
    }
        
    QString fileName = processToPDF.value(process);
    if (fileName.isEmpty()) 
        return;
        
    if (exitCode == 0) 
    {
        //QMessageBox::information(this, "Process Window", "Conversion Done!");
    } else {
        QMessageBox::warning(this, "Process Window", "Error occurred, something went wrong");
    }
        
    for (auto& pdf : PDFFiles) 
    {
        if (QString::fromStdString(pdf.file_name) == fileName) 
        {
            PDFInfo tempPdf(pdf.file_name);
            tempPdf.parseSumatraSettings("C:\\Users\\zezo_\\AppData\\Local\\SumatraPDF\\SumatraPDF-settings.txt");
                
            pdf.page_num = tempPdf.page_num;
                
            if (pdf.button) 
            {
                pdf.button->setText(QString("%1 (Page %2)")
                    .arg(QString::fromStdString(pdf.file_name))
                    .arg(pdf.page_num));
            }
                
            qDebug() << "Updated page number for" << QString::fromStdString(pdf.file_name) 
                        << "to page:" << pdf.page_num;
            break;
        }
    }
}

void PDFManager::addNewPDF()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open PDF File",
                                                    QDir::homePath(), "PDF Files (*.pdf))");

    qInfo() << "Selected file path:" << filePath;

    if (filePath.isEmpty()){
        qInfo() << "No file selected.";
        return;
    }

    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    qInfo() << filePath << "," << fileName;

    // check whether this pdf already exists or not
    bool isDuplicate = false;
    for (const auto& pdf : PDFFiles) 
    {
        if (pdf.file_name == fileName.toStdString()) 
        {
            isDuplicate = true;
            QMessageBox::information(
                this,
                "Duplicate File",
                "This PDF file is already in your list."
            );
            break;
        }
    }

    if(!isDuplicate)
    {
        auto pdfButton = new QPushButton(fileName);
        pdfButton->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding); 
            
        connect(pdfButton, &QPushButton::clicked, this, 
        [this, filePath]()
        {
            openPDF(filePath);
        });

        PDFInfo newPDF;
        newPDF.file_name = fileName.toStdString();
        newPDF.file_path = filePath.toStdString();
        newPDF.button = pdfButton;
            
        PDFFiles.push_back(newPDF);
            
        int addButtonIndex = vbox->indexOf(vbox->itemAt(vbox->count() - 2)->widget());
        vbox->insertWidget(addButtonIndex, pdfButton);
    }
}

void PDFManager::openPDF(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    QProcess* process = new QProcess(this);

    processToPDF[process] = fileName;

    process->setProperty("autoDelete", true);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PDFManager::handleFinished);

    connect(process, &QObject::destroyed, this,
    [this, process]() {
        processToPDF.remove(process);
    });

    QStringList arguments;
    arguments << filePath;
        
    // If we have a stored page number, open to that page
    for (const auto& pdf : PDFFiles) 
    {
        if (QString::fromStdString(pdf.file_name) == fileName && pdf.page_num > 0) {
            arguments << QString("-view");
            arguments << QString("single page");
            arguments << QString("-new-window");
            arguments << QString("-page");
            arguments << QString::number(pdf.page_num);
            break;
        }
    }
        
    process->start("C:\\Users\\zezo_\\AppData\\Local\\SumatraPDF\\SumatraPDF.exe", arguments);

    if (!process->waitForStarted()){
        QMessageBox::critical(this, "Error","Failed to start Process");
    }
}

void PDFManager::filterToolBoxItems() 
{
    QString searchText = searchBar->text();
    
    if (searchText.isEmpty()) {
        for (auto &pdf : PDFFiles) {
            pdf.button->setVisible(true); 
        }
        for (int i = 0; i < toolbox->count(); ++i) {
            toolbox->setItemEnabled(i, true);
        }
        return;   // Exit the function early
    }

    for (int i = 0; i < toolbox->count(); ++i) 
    {
        bool childMatch = false;
        for (auto &pdf : PDFFiles) {
            if (searchText.isEmpty()) {
                pdf.button->setVisible(true);
            } else {
                bool match = QString::fromStdString(pdf.file_name)
                                    .contains(searchText, Qt::CaseInsensitive);
                pdf.button->setVisible(match);
                childMatch = true;
            }
        }
        toolbox->setItemEnabled(i, childMatch);
    }
}

void PDFManager::collapseAllToolBoxItems(QToolBox *toolbox) 
{
    QWidget *dummyWidget = new QWidget();
    dummyWidget->setFixedHeight(0);
    dummyWidget->setVisible(false);

    int dummyIndex = toolbox->addItem(dummyWidget, "");

    toolbox->setCurrentIndex(dummyIndex);

    toolbox->setItemText(dummyIndex, "");
}

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    PDFManager manager;
    manager.show();

    return app.exec();
}

#include <main.moc>
