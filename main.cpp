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
#include <QDialog>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin) // Windows

#define MAX(a, b) (a>b)?a:b

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
    // For linux 
    void parseOkularSettings(const std::string& settings_path)
    {

    }
};

struct PDFCat
{
    std::string category;
    std::vector<PDFInfo> PDFFiles;
    QWidget *container;
    QVBoxLayout *layout;
    QPushButton *addButton;

    PDFCat(const std::string name) : category(name)
    { 
    }
};

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
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus, PDFCat &category);
    void addNewPDF(PDFCat &category);
    void openPDF(PDFCat &cat, const QString &filePath);
    void filterToolBoxItems();
    void collapseAllToolBoxItems(QToolBox *toolbox);
    void createMenu();
    void serializeData();
    void loadData();
  public:
    QMenu *fileMenu;
    QAction *exitAction;
    QWidget *centralWidget;
    QVBoxLayout *vbox;
    QToolBox *toolbox;
    QLineEdit *searchBar;
    std::vector<PDFCat> PDFcats;
    QMap<QProcess *, QString> processToPDF;

  protected:
    void closeEvent(QCloseEvent *event) override 
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Save Changes", "Do you want to save changes before exiting?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) 
        {
            if (serializeData()) 
            {
                event->accept();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Failed to save data. Close anyway?",
                                      QMessageBox::Yes | QMessageBox::No);

                if (QMessageBox::Yes) {
                    event->accept();
                } else {
                    event->ignore();
                }
            }
        } else if (reply == QMessageBox::No) {
            event->accept();
        } else {
            event->ignore();
        }
    }
};

PDFManager::PDFManager(QWidget *parent)
{
    setWindowTitle("PDF Manger");
    setWindowFlags(Qt::Window);
    setWindowIcon(QIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\icon.png")); 


    QFont *defaultFont;
    QString fontPath = "C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\CaskaydiaCoveNerdFont-Regular.ttf";
    int fontId = QFontDatabase::addApplicationFont(fontPath);

    if (fontId != -1) 
    {
        QString fontFamily =
            QFontDatabase::applicationFontFamilies(fontId).at(0);
        defaultFont = new QFont(fontFamily, 18);
    } else {
        // Handle the error: the font could not be loaded
        qWarning() << "Failed to load font from path:" << fontPath;
        defaultFont = new QFont("Arial", 12);
    }

    QApplication::setFont(*defaultFont);
    resize(800, 400);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto main_layout = new QVBoxLayout(centralWidget);

    createMenu();
        
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");
    searchBar->setFont(*defaultFont);

    QObject::connect(searchBar, &QLineEdit::textChanged,
                     this, &PDFManager::filterToolBoxItems);

    main_layout->addWidget(searchBar);

    toolbox = new QToolBox(this);
    toolbox->setStyleSheet(
        "QToolBox {"
        "    background-color: #1e1e1e;"   // Set the background color of the
        "}"
        "QToolBox::tab {"
        "    background-color: #282a36;"   
        "    color: white;"                
        "    border-radius: 4px;"          
        "}"
        "QToolBox::tab:selected {"         
        "    background-color: #44475a;"   
        "}"
        "QToolBox::tab:hover {"            
        "    background-color: #6272a4;"  
        
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

    PDFcats.push_back(PDFCat("Programming"));
    PDFcats.push_back(PDFCat("Math"));
    PDFcats.push_back(PDFCat("Science"));
    PDFcats.push_back(PDFCat("Literature"));
    PDFcats.push_back(PDFCat("Music"));

    for (auto& cat : PDFcats)
    {
        cat.container = new QWidget(this);
        cat.container->setStyleSheet(
            "QWidget {"
            "    background-color: #1e1e1e;" 
            "}");
        cat.layout = new QVBoxLayout(cat.container);
        cat.layout->setSpacing(5);   
        cat.layout->setContentsMargins(30, 10, 30, 10);   

        toolbox->addItem(cat.container, cat.category.c_str());

        // Add and center an add button to add new pdfs
        cat.addButton = new QPushButton("+");
        cat.addButton->setFixedSize(30, 30);

        auto centerLayout = new QHBoxLayout();
        centerLayout->addStretch();
        centerLayout->addWidget(cat.addButton);
        centerLayout->addStretch();

        cat.layout->addLayout(centerLayout);
        // Add the "+" button before the stretch
        cat.layout->addStretch(1);   

        connect(cat.addButton, &QPushButton::clicked, this,
                [this, &cat]() {
                    this->addNewPDF(cat); 
                });
    }

    scrollArea->setWidget(toolbox);
    
    collapseAllToolBoxItems(toolbox);


    main_layout->addWidget(scrollArea);
}

void PDFManager::handleFinished(int exitCode, QProcess::ExitStatus exitStatus, PDFCat &category) 
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
        
    for (auto& pdf : category.PDFFiles) 
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

void PDFManager::addNewPDF(PDFCat& category) 
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
    for (auto &pdf : category.PDFFiles) 
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
        [this, filePath, &category](){
            openPDF(category, filePath);
        });

        PDFInfo newPDF;
        newPDF.file_name = fileName.toStdString();
        newPDF.file_path = filePath.toStdString();
        newPDF.button = pdfButton;
            
        category.PDFFiles.push_back(newPDF);
            
        int addButtonIndex = category.layout->indexOf(category.layout->itemAt(category.layout->count() - 2)->widget());
        category.layout->insertWidget(addButtonIndex, pdfButton);
    }
}

void PDFManager::openPDF(PDFCat &cat, const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    QProcess* process = new QProcess(this);

    processToPDF[process] = fileName;

    process->setProperty("autoDelete", true);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, &cat](int exitCode, QProcess::ExitStatus exitStatus) {
                this->handleFinished(exitCode, exitStatus, cat);
            });

    connect(process, &QObject::destroyed, this,
    [this, process]() {
        processToPDF.remove(process);
    });

    QStringList arguments;
    arguments << filePath;
        
    // If we have a stored page number, open to that page
    for (const auto& pdf : cat.PDFFiles) 
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
    
    if (searchText.isEmpty()) 
    {
        for (auto &cat : PDFcats) {
            for (auto &pdf : cat.PDFFiles) {
                pdf.button->setVisible(true); 
            }
        }
        for (int i = 0; i < toolbox->count(); ++i) {
            toolbox->setItemEnabled(i, true);
        }
        return;   // Exit the function early
    }
    int idx = 0;
    for (auto &cat : PDFcats) 
    {
        bool childMatch = false;
        for (auto &pdf : cat.PDFFiles) {
            if (searchText.isEmpty()) {
                pdf.button->setVisible(true);
            } else {
                bool match = QString::fromStdString(pdf.file_name)
                                    .contains(searchText, Qt::CaseInsensitive);
                pdf.button->setVisible(match);
                if (match)
                    childMatch = true;
            }
        }
        toolbox->setItemEnabled(idx, childMatch);
        idx++;
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

void PDFManager::createMenu() 
{
    fileMenu = menuBar()->addMenu(tr("&File"));

    exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setIcon(QIcon(":/icons/exit.png"));
    exitAction->setShortcut(QKeySequence::Quit); 

    fileMenu->addSeparator();

    QAction *openAction = fileMenu->addAction(tr("&Open"));
    openAction->setIcon(QIcon(":/icons/open.png")); 
    openAction->setShortcut(QKeySequence::Open);


    connect(exitAction, &QAction::triggered, 
            qApp, &QApplication::quit);

    menuBar()->setStyleSheet(
        "QMenuBar {"
        "    background-color: #2E3440;"   // Dark background
        "    color: #D8DEE9;"              // Light text
        "    font-weight: bold;"
        "    padding: 5px 10px;"
        "}"
        "QMenuBar::item {"
        "    padding: 6px 15px;"
        "    border-radius: 4px;"   // Rounded corners
        "    spacing: 5px;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: #4C566A;"   // Darker highlight
        "}"
        "QMenuBar::item:pressed {"
        "    background-color: #5E81AC;"   // Soft blue press effect
        "}"
        "QMenu {"
        "    background-color: #3B4252;"
        "    color: #D8DEE9;"
        "    border: 1px solid #4C566A;"
        "    padding: 5px;"
        "}"
        "QMenu::item {"
        "    padding: 8px 20px;"
        "    border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #5E81AC;"   // Softer selection color
        "    color: #ECEFF4;"
        "}"
        "QMenu::separator {"
        "    height: 1px;"
        "    background: #4C566A;"
        "    margin: 4px 10px;"
        "}");
}

void PDFManager::serializeData() 
{
    std::ofstream file("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\pdfmanager.conf");
    if (!file.is_open()) {
        return false;
    }
    for (auto &cat : PDFcats) 
    {
        out << cat.category << ", " << cat.PDFFiles.size() << "\n";

        for (const auto &pdf : cat.PDFFiles) {
            out << pdf.file_name << ", " << pdf.file_path << ", " << pdf.page_num
                << "\n";
        }


        out << "\n";
    }
}

bool deserializePDFCat(std::istream &in, PDFCat &cat) 
{
    std::string line;

    if (!std::getline(in, line) || line.empty()) {
        return false;
    }

    size_t comma_pos = line.find(", ");
    if (comma_pos == std::string::npos) {
        return false;
    }

    cat.category = line.substr(0, comma_pos);
    int fileCount = std::stoi(line.substr(comma_pos + 2));

    cat.PDFFiles.clear();
    for (int i = 0; i < fileCount; i++) 
    {
        if (!std::getline(in, line)) {
            return false;
        }

        std::istringstream iss(line);
        PDFInfo pdf;

        std::getline(iss, pdf.file_name, ',');
        if (pdf.file_name.empty()) {
            return false;
        }
        pdf.file_name = pdf.file_name.at(0) == ' ' ? pdf.file_name.substr(1)
                                                   : pdf.file_name;

        std::getline(iss, pdf.file_path, ',');
        if (pdf.file_path.empty()) {
            return false;
        }
        pdf.file_path = pdf.file_path.at(0) == ' ' ? pdf.file_path.substr(1)
                                                   : pdf.file_path;

        std::string page_num_str;
        std::getline(iss, page_num_str);
        if (page_num_str.empty()) {
            return false;
        }
        page_num_str =
            page_num_str.at(0) == ' ' ? page_num_str.substr(1) : page_num_str;
        pdf.page_num = std::stoi(page_num_str);

        cat.PDFFiles.push_back(pdf);
    }

    std::getline(in, line);

    return true;
}

void PDFManager::loadData() 
{
    std::ofstream in("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\pdfmanager.conf");
    if (!in.is_open()) {
        return;
    }

    PDFCat cat;

    while (in.peek() != EOF) 
    {
        if (deserializePDFCat(in, cat)) 
        {
            PDFcats.push_back(cat);
        } 
        else 
        {
            if (in.peek() == EOF) {
                break;
            }
            std::string line;
            while (in.peek() != EOF && std::getline(in, line) && line.empty()) {
            }
        }
    }
}


struct PDFInfo
{
    bool found = false;
    std::string file_name;
    std::string file_path;
    int page_num = 0;
    std::string mode;

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    PDFManager manager;
    manager.show();

    return app.exec();
}

#include <main.moc>
