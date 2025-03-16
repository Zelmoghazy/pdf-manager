#include "main.h"

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

PDFManager::PDFManager(QWidget *parent)
{
    setWindowTitle("PDF Manger");
    setWindowFlags(Qt::Window);
#ifdef Q_OS_WIN
    setWindowIcon(QIcon(QDir::currentPath() + 
                  "\\" + "Images" + "\\" +  "icon.png")); 
#endif 

    PDFManager::setFont(defaultFont);

    resize(800, 400);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto main_layout = new QVBoxLayout(centralWidget);

    createMenuBar();
        
    createSearchbar();

   

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: #1e1e1e;"   
        "    border: none;"                
        "}");

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
        "QToolBox::tab:disabled {"         // Style for disabled tabs
        "    background-color: #1a1a1a;"   // Darker background for disabled
                                           // state
        "    color: #6d6d6d;"              // Grayed out text for disabled state
        "}"
    );

    loadData();

    for (auto& cat : PDFcats)
    {
        PDFManager::setupNewCat(cat);
        toolbox->addItem(cat.container, cat.category.c_str());
        for (auto& pdf : cat.PDFFiles)
        {
            auto pdfButton = new QPushButton(QString::fromStdString(pdf.file_name));
            pdfButton->setSizePolicy(QSizePolicy::Expanding,
                                     QSizePolicy::Expanding);

            connect(pdfButton, &QPushButton::clicked, this,
                [this, filePath = pdf.file_path, &cat]() { 
                    openPDF(cat, QString::fromStdString(filePath)); 
                });

            pdf.button = pdfButton;

            int addButtonIndex = cat.layout->indexOf(
                cat.layout->itemAt(cat.layout->count() - 2)->widget()
            );
            cat.layout->insertWidget(addButtonIndex, pdfButton);
        }
    }

    QWidget *dummyWidget = new QWidget();
    dummyWidget->setFixedHeight(0);
    dummyWidget->setVisible(false);

    dummyIndex = toolbox->addItem(dummyWidget, "");

    PDFManager::setupToolBoxConnections(); 
    scrollArea->setWidget(toolbox);

    main_layout->addWidget(searchBar);
    main_layout->addWidget(scrollArea);
}

void PDFManager::setFont(QFont *&defaultFont) 
{
#ifdef Q_OS_WIN
    QString fontPath = QDir::currentPath() 
                       + "\\" + "Images" + "\\" 
                       + "CaskaydiaCoveNerdFont-Regular.ttf";
#endif

    int fontId = QFontDatabase::addApplicationFont(fontPath);

    if (fontId != -1) 
    {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        defaultFont = new QFont(fontFamily, 18);
    } 
    else 
    {
        qWarning() << "Failed to load font from path:" << fontPath << ", a default font is used";
        defaultFont = new QFont("Open Sans", 18);
    }

    QApplication::setFont(*defaultFont);
}

void PDFManager::createSearchbar()
{
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");
    searchBar->setFont(*defaultFont);

    QObject::connect(searchBar, &QLineEdit::textChanged, 
                     this, &PDFManager::filterToolBoxItems);
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
    if (LastBrowsedPath.isEmpty()) 
    {
        LastBrowsedPath = QFileDialog::getOpenFileName(
            this, "Open PDF File", QDir::homePath(), "PDF Files (*.pdf))");
    } 
    else 
    {
        LastBrowsedPath = QFileDialog::getOpenFileName(
            this, "Open PDF File", LastBrowsedPath, "PDF Files (*.pdf))");
    }    

    qInfo() << "Selected file path:" << LastBrowsedPath;

    if (LastBrowsedPath.isEmpty()) {
        qInfo() << "No file selected.";
        return;
    }

    setupNewPDF(category, LastBrowsedPath);
}

void PDFManager::setupNewPDF(PDFCat &category, QString &filePath) 
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    qInfo() << filePath << "," << fileName;

    // check whether this pdf already exists or not
    bool isDuplicate = false;
    for (auto &pdf : category.PDFFiles) {
        if (pdf.file_name == fileName.toStdString()) {
            isDuplicate = true;
            QMessageBox::information(this, "Duplicate File",
                                     "This PDF file is already in your list.");
            break;
        }
    }

    if (!isDuplicate) {
        auto pdfButton = new QPushButton(fileName);
        pdfButton->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Expanding);

        connect(pdfButton, &QPushButton::clicked, this,
                [this, filePath, &category]() { openPDF(category, filePath); });

        PDFInfo newPDF;
        newPDF.file_name = fileName.toStdString();
        newPDF.file_path = filePath.toStdString();
        newPDF.button = pdfButton;

        category.PDFFiles.push_back(newPDF);

        int addButtonIndex = category.layout->indexOf(
            category.layout->itemAt(category.layout->count() - 2)->widget());
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

void PDFManager::collapseAllToolBoxItems() 
{
    toolbox->setCurrentIndex(dummyIndex);
    toolbox->setItemText(dummyIndex, "");
}

void PDFManager::setupToolBoxConnections() 
{
    disconnect(toolbox, &QToolBox::currentChanged, this,
               &PDFManager::onToolBoxItemChanged);

    connect(toolbox, &QToolBox::currentChanged, this,
            &PDFManager::onToolBoxItemChanged);

    for (QWidget *w : toolbox->findChildren<QWidget *>()) {
        if (w->inherits("QToolBoxButton")) {
            QAbstractButton *button = qobject_cast<QAbstractButton *>(w);
            disconnect(button, &QAbstractButton::clicked, this,
                       &PDFManager::onToolBoxItemClicked);
        }
    }

    for (QWidget *w : toolbox->findChildren<QWidget *>()) {
        if (w->inherits("QToolBoxButton")) {
            QAbstractButton *button = qobject_cast<QAbstractButton *>(w);
            connect(button, &QAbstractButton::clicked, this,
                    &PDFManager::onToolBoxItemClicked);
        }
    }
}

void PDFManager::onToolBoxItemClicked(bool checked) 
{
    qDebug() << "Item clicked\n";

    if (toolbox->currentIndex() == dummyIndex)
    {
        onAddCategoryClicked();
    }

    if (toolBoxChanged) 
    {
        toolBoxChanged = false;
        return;
    }

    collapseAllToolBoxItems();
}

void PDFManager::onToolBoxItemChanged(int index) 
{
    qDebug() << "ToolBoxItem changed !, index : " << index
             << "\n ";

    toolBoxChanged = true;
    lastIndex = index;
}

void PDFManager::onAddCategoryClicked()
{
    bool ok;
    QString categoryName 
            = QInputDialog::getText( this, "Add New Category",
                "Enter category name:", QLineEdit::Normal, "", &ok);

    if (ok && !categoryName.isEmpty()) 
    {
        QWidget *dummyWidget = toolbox->widget(dummyIndex);
        toolbox->removeItem(dummyIndex);

        PDFCat newCat(categoryName.toStdString());
        PDFcats.push_back(newCat);

        auto &cat = PDFcats.back();

        PDFManager::setupNewCat(cat);

        toolbox->addItem(cat.container, QString::fromStdString(cat.category));

        dummyIndex = toolbox->addItem(dummyWidget, "");
        toolbox->setItemText(dummyIndex, "");

        setupToolBoxConnections();

        toolbox->setCurrentIndex(dummyIndex - 1);
    }
}

void PDFManager::setupNewCat(PDFCat& cat)
{
    cat.container = new QWidget(this);
    cat.container->setStyleSheet("QWidget {"
                                 "    background-color: #1e1e1e;"
                                 "}");
    cat.layout = new QVBoxLayout(cat.container);
    cat.layout->setSpacing(5);
    cat.layout->setContentsMargins(30, 10, 30, 10);

    // Add and center an add button to add new pdfs
    cat.addButton = new QPushButton("+");
    cat.addButton->setFixedSize(30, 30);

    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(cat.addButton);
    centerLayout->addStretch();

    cat.layout->addLayout(centerLayout);
    cat.layout->addStretch(1);

    connect(cat.addButton, &QPushButton::clicked, this,
            [this, &cat]() { this->addNewPDF(cat); });
}

void PDFManager::createMenuBar() 
{
    menuBar = QMainWindow::menuBar();

    fileMenu = menuBar->addMenu("&File");

    QAction *newAction =
        fileMenu->addAction(QIcon::fromTheme("document-new"), "&New");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QAction *openAction =
        fileMenu->addAction(QIcon::fromTheme("document-open"), "&Open...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QAction *saveAction =
        fileMenu->addAction(QIcon::fromTheme("document-save"), "&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    exitAction = fileMenu->addAction(QIcon::fromTheme("application-exit"), "E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    fileMenu->addSeparator();

        // Edit menu
    editMenu = menuBar->addMenu("&Edit");

    QAction *undoAction =
        editMenu->addAction(QIcon::fromTheme("edit-undo"), "&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QAction *redoAction =
        editMenu->addAction(QIcon::fromTheme("edit-redo"), "&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    editMenu->addSeparator();

    QAction *cutAction =
        editMenu->addAction(QIcon::fromTheme("edit-cut"), "Cu&t");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QAction *copyAction =
        editMenu->addAction(QIcon::fromTheme("edit-copy"), "&Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QAction *pasteAction =
        editMenu->addAction(QIcon::fromTheme("edit-paste"), "&Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    // View menu
    viewMenu = menuBar->addMenu("&View");

        // Help menu
    helpMenu = menuBar->addMenu("&Help");

    QAction *aboutAction =
        helpMenu->addAction(QIcon::fromTheme("help-about"), "&About");
    connect(aboutAction, &QAction::triggered, this, &PDFManager::menuBarActionStub);

    QFont menuFont("Open Sans", 18);
    menuBar->setFont(*defaultFont);

    foreach (QWidget *widget, menuBar->findChildren<QWidget *>()) {
        widget->setFont(*defaultFont);
    }
    menuBar->setStyleSheet(
        "QMenuBar {"
        "    background-color: #2E3440;"   // Dark background
        "    color: #D8DEE9;"              // Light text
        //"    font-weight: bold;"
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

void PDFManager::menuBarActionStub() 
{
    qDebug() << "Action taken";
}

bool PDFManager::serializeData() 
{
    QString currentDirConfigPath = QDir::currentPath() + "\\" + "pdfmanager.conf";

    std::ofstream file(currentDirConfigPath.toStdString());

    if (!file.is_open()) 
    {
        return false;
    }

    for (auto &cat : PDFcats) 
    {
        file << cat.category << ", " << cat.PDFFiles.size() << "\n";

        for (const auto &pdf : cat.PDFFiles) 
        {
            file << pdf.file_name << ", " << pdf.file_path << ", " << pdf.page_num
                << "\n";
        }

        file << "\n";
    }
    
    return true;
}

bool PDFManager::deserializePDFCat(std::istream &in, PDFCat &cat) 
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
    QString currentDirConfigPath = QDir::currentPath() + "\\" + "pdfmanager.conf";

    std::ifstream in(currentDirConfigPath.toStdString());

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

void PDFManager::loadConfig()
{
    
    QString sumatraPath;
    const QString CONFIG_FILENAME = "pdfmanager_config.ini";
    QString currentDirConfigPath = QDir::currentPath() + "/" + CONFIG_FILENAME;

    QFileInfo configFileInfo(currentDirConfigPath);

    if (configFileInfo.exists())
    {
        QSettings settings(currentDirConfigPath, QSettings::IniFormat);
        sumatraPath = settings.value("Paths/SumatraPDF").toString();

        if (!QFileInfo(sumatraPath).exists()) {
            sumatraPath.clear(); 
        }
    }

    // No configuration, create one
    if (sumatraPath.isEmpty()) 
    {
        QString commonPath = QDir::homePath() + "\\AppData\\Local\\SumatraPDF\\SumatraPDF.exe";
        if (QFileInfo(commonPath).exists()) {
            sumatraPath = commonPath;
        }

        // SumatraPDF doesnt exist in the default location
        if (sumatraPath.isEmpty()) 
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("SumatraPDF Not Found");
            msgBox.setText("SumatraPDF is required to view PDF files.");
            msgBox.setInformativeText("Would you like to locate SumatraPDF on "
                                      "your system or download it?");
            QPushButton *locateButton = msgBox.addButton("Locate", QMessageBox::ActionRole);
            QPushButton *downloadButton = msgBox.addButton("Download", QMessageBox::ActionRole);
            QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);

            msgBox.exec();

            if (msgBox.clickedButton() == locateButton) 
            {
                sumatraPath = QFileDialog::getOpenFileName(this, "Locate SumatraPDF Executable", QDir::homePath(), "Executable Files (*.exe)");

                if (sumatraPath.isEmpty()) {
                    QMessageBox::warning(this, "Operation Cancelled",
                                         "PDF viewer path not set.");
                    return;
                }
            } 
            else if (msgBox.clickedButton() == downloadButton) 
            {
                QDesktopServices::openUrl(
                    QUrl("https://www.sumatrapdfreader.org/"
                         "download-free-pdf-viewer"));
                QMessageBox::information(this,
                    "Download SumatraPDF",
                    "Please install SumatraPDF and try again.\n"
                    "After installation, you will be prompted to locate the "
                    "executable.");
                return;
            } 
            else 
            {
                QMessageBox::warning(this, "Operation Cancelled",
                                     "PDF viewer path not set.");
                return;
            }
        }

        // Save the path to config file
        QDir configDir(QFileInfo(currentDirConfigPath).path());
        if (!configDir.exists()) {
            configDir.mkpath(".");
        }

        QSettings settings(currentDirConfigPath, QSettings::IniFormat);
        settings.setValue("Paths/SumatraPDF", sumatraPath);
        settings.sync();
    }
    
}

void PDFManager::closeEvent(QCloseEvent *event) 
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Save Changes", "Do you want to save changes before exiting?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        if (serializeData()) {
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

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    PDFManager manager;
    manager.show();

    return app.exec();
}
