#include "pdfmanager.hpp"
#include "Prof.h"

static CompareFunc timestampSort() 
{
    return [](QWidget* a, QWidget* b) {
        WordWrapButton* btnA = qobject_cast<WordWrapButton*>(a);
        WordWrapButton* btnB = qobject_cast<WordWrapButton*>(b);
        if (btnA && btnB) {
            return btnA->getTimestamp() > btnB->getTimestamp(); // Newer on top (descending)
        }
        return false;
    };
}
    
static CompareFunc alphabeticalSort() 
{
    return [](QWidget* a, QWidget* b) {
        WordWrapButton* btnA = qobject_cast<WordWrapButton*>(a);
        WordWrapButton* btnB = qobject_cast<WordWrapButton*>(b);
        if (btnA && btnB) {
            return btnA->getText().toLower() < btnB->getText().toLower(); // Alphabetical (ascending)
        }
        return false;
    };
}


PDFManager::PDFManager(QWidget *parent)
{
    START_TIMING("STARTUP TIME");

    setWindowTitle("PDF Manager");
    setWindowFlags(Qt::Window);
    resize(800, 400);

#ifdef Q_OS_WIN
    setWindowIcon(QIcon(QDir::currentPath() + "\\" + "Images" + "\\" +  "icon.png")); 
#endif 
    PDFManager::setFont();

    setDockOptions(QMainWindow::AnimatedDocks    | 
                   QMainWindow::AllowNestedDocks |
                   QMainWindow::AllowTabbedDocks);

    START_TIMING("Creating Menu and Sidebar");
    createMenuBar();

    createSidebar();
    STOP_TIMING("Creating Menu and Sidebar");


    contentArea = new QWidget(this);
    setCentralWidget(contentArea);
    contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(5, 5, 5, 5);

    createSearchbar();

    START_TIMING("Creating Content Area");
    createCategoriesArea();
    STOP_TIMING("Creating Content Area");

    START_TIMING("Loading Data");
    loadData();
    STOP_TIMING("Loading Data");

    START_TIMING("Init App");
    initApp();
    STOP_TIMING("Init App");

    START_TIMING("Update Buttons");
    contentLayout->addWidget(searchBar);
    contentLayout->addWidget(categoriesArea);
    updateMainSidebarButtons();
    STOP_TIMING("Update Buttons");

    STOP_TIMING("STARTUP TIME");
    SAVE_TIMING_RESULTS();
}

void PDFManager::addMainSidebarSection(const QString &text, const QIcon &icon, int idx) 
{
    QPushButton *btn = new QPushButton();

    btn->setIcon(icon);
    btn->setText(text);
    btn->setIconSize(QSize(24, 24));
    btn->setMinimumHeight(40);

    connect(btn, &QPushButton::clicked,
            [this, idx]() 
            {
                toggleSecondaryDock(idx); 
            });

    mainSidebarLayout->addWidget(btn);
    mainSidebarButtons.append(btn);
}

void PDFManager::createSidebar() 
{
    QIcon homeIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\home.png");
    QIcon computerIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\computer.png");
    QIcon filesIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\google-docs.png");
    QIcon settingsIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\settings.png");
    QIcon helpIcon("C:\\Users\\zezo_\\Desktop\\Programming\\staticQT\\Images\\help.png");

    buttonDataList = {
        {"Search",
         homeIcon,
         "Search Contents",
         {"Dashboard", "Shortcuts", "Recent Items"}},

        {"Layout",
         computerIcon,
         "Layout",
         {"System Info", "Disk Management", "Device Manager", "Network"}},

        {"Files",
         filesIcon,
         "File Browser",
         {"Documents", "Pictures", "Music", "Videos", "Downloads"}},

        {"Settings",
         settingsIcon,
         "Application Settings",
         {"Appearance", "Performance", "Notifications", "Privacy", "Updates"}},

        {"Help",
         helpIcon,
         "Help & Support",
         {"Documentation", "FAQ", "Contact Support", "About"}}
    };

    /* ---------------------------- Main Sidebar ---------------------------- */
    QDockWidget *mainSidebarDock = new QDockWidget("Main Sidebar", this);
    mainSidebarDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
    mainSidebarDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mainSidebarDock->setTitleBarWidget(new QWidget()); // no title

    mainSidebar = new QWidget();
    mainSidebar->setMinimumWidth(mainCollapsedWidth);
    mainSidebar->setMaximumWidth(mainCollapsedWidth);
    mainSidebar->setStyleSheet("background-color: #282a36;");

    mainSidebarLayout = new QVBoxLayout(mainSidebar);
    mainSidebarLayout->setContentsMargins(5, 10, 5, 10);
    mainSidebarLayout->setSpacing(5);

    for(int i = 0; i < buttonDataList.size(); ++i) 
    {
        addMainSidebarSection(buttonDataList[i].text, buttonDataList[i].icon, i);
    }
    
    mainSidebarLayout->addStretch();

    toggleMainButton = new QPushButton();
    toggleMainButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    mainSidebarLayout->addWidget(toggleMainButton);
    connect(toggleMainButton, &QPushButton::clicked,
            this, &PDFManager::toggleMainSidebar);

    mainSidebarDock->setWidget(mainSidebar);

    addDockWidget(Qt::LeftDockWidgetArea, mainSidebarDock);

    /*--------------------------------------------------------------------------*/

    // Create secondary dock widget
    secondaryDock = new QDockWidget("Options", this);
    secondaryDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                                   Qt::RightDockWidgetArea);
    secondaryDock->setFeatures(QDockWidget::NoDockWidgetFeatures); 
    secondaryDock->setMinimumWidth(secondaryDockWidth);

    QWidget *dockContents = new QWidget();
    QVBoxLayout *dockLayout = new QVBoxLayout(dockContents);
    dockLayout->setContentsMargins(10, 10, 10, 10);

    secondaryStack = new QStackedWidget(dockContents);

    // add options 
    searchWidget = new PDFSearchWidget(dockContents);
    secondaryStack->addWidget(searchWidget);

    // Connect to its signals if needed
    connect(searchWidget, &PDFSearchWidget::searchStarted, this, 
    []()
    {
        qDebug() << "Search started";
    });

    connect(searchWidget, &PDFSearchWidget::searchFinished, this, 
    [this]()
    {
        QMessageBox::information(this, "Search" , "Search done !", QMessageBox::Ok);
    });

    QWidget *viewPanel = new QWidget();
    auto viewLayout = new QVBoxLayout(viewPanel);

    QPushButton* switchViewBtn = new QPushButton("Switch View");
    connect(switchViewBtn, &QPushButton::clicked, 
            this, &PDFManager::switchPDFView);

    viewLayout->addWidget(switchViewBtn);
    viewLayout->addStretch();

    secondaryStack->addWidget(viewPanel);

    for (int i = 2; i < buttonDataList.size(); ++i) 
    {
        QWidget *panel = createSecondaryPanel(i);
        secondaryStack->addWidget(panel);
    }

    dockLayout->addWidget(secondaryStack);
    secondaryDock->setWidget(dockContents);

    addDockWidget(Qt::LeftDockWidgetArea, secondaryDock);

    // Add a tab dock relationship to ensure proper positioning
    splitDockWidget(mainSidebarDock, secondaryDock, Qt::Horizontal);

    connect(secondaryDock, &QDockWidget::visibilityChanged,
        [this](bool visible) {
            if (!visible) {
                currentSecondaryIndex = -1;
                isSecondaryDockVisible = false;
                updateMainSidebarButtons();
            }
        });

    // Initially hide the dock
    secondaryDock->hide();
    isSecondaryDockVisible = false;

    // Setup animation for main sidebar
    mainSidebarAnimation = new QPropertyAnimation(mainSidebar, "minimumWidth");
    mainSidebarAnimation->setDuration(10);
    connect(mainSidebarAnimation, &QPropertyAnimation::finished, 
            this, &PDFManager::updateMainSidebarButtons);
}

void PDFManager::toggleMainSidebar() 
{
    mainSidebarAnimation->stop();

    if (isMainSidebarExpanded) 
    {
        toggleMainButton->setText("");
        toggleMainButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
        mainSidebarAnimation->setStartValue(mainSidebar->width());
        mainSidebarAnimation->setEndValue(mainCollapsedWidth);
        mainSidebar->setMaximumWidth(mainCollapsedWidth);

        if (isSecondaryDockVisible) {
            closeSecondaryDock();
        }
    } 
    else 
    {
        toggleMainButton->setText("");
        toggleMainButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
        mainSidebarAnimation->setStartValue(mainSidebar->width());
        mainSidebarAnimation->setEndValue(mainExpandedWidth);
        mainSidebar->setMaximumWidth(mainExpandedWidth);
    }

    mainSidebarAnimation->start();
    isMainSidebarExpanded = !isMainSidebarExpanded;
}

void PDFManager::updateMainSidebarButtons() 
{
    for (int i = 0; i < mainSidebarButtons.size(); ++i) 
    {
        QPushButton *btn = mainSidebarButtons[i];

        if (isMainSidebarExpanded) 
        {
            btn->setText(buttonDataList[i].text);
            btn->setStyleSheet("text-align: left; padding-left: 10px;");
        } 
        else 
        {
            btn->setText("");
            btn->setStyleSheet("text-align: center; padding-left: 0px;");
        }

        if (i == currentSecondaryIndex && isSecondaryDockVisible) 
        {
            btn->setStyleSheet(btn->styleSheet() +
                               "; background-color: #d0d0d0;");
        }
    }
}

void PDFManager::toggleSecondaryDock(int index) 
{
    if (currentSecondaryIndex == index && isSecondaryDockVisible) {
        closeSecondaryDock();
        return;
    }

    secondaryStack->setCurrentIndex(index);

    secondaryDock->setWindowTitle(buttonDataList[index].text + " Options");

    if (!isSecondaryDockVisible) 
    {
        secondaryDock->show();
        isSecondaryDockVisible = true;
    }

    int oldIndex = currentSecondaryIndex;
    currentSecondaryIndex = index;

    updateMainSidebarButtons();
}

void PDFManager::closeSecondaryDock() 
{
    if (!isSecondaryDockVisible)
        return;

    secondaryDock->hide();

    isSecondaryDockVisible = false;
    currentSecondaryIndex = -1;

    updateMainSidebarButtons();
}

QWidget *PDFManager::createSecondaryPanel(int index) 
{
    const auto &buttonData = buttonDataList[index];

    QWidget *panel = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(panel);

    // Add a header
    QLabel *headerLabel = new QLabel(buttonData.text + " Options");
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(headerFont.pointSize() + 2);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    layout->addWidget(headerLabel);

    // Add the sub-options as buttons
    for (const auto &subOption : buttonData.subOptions) 
    {
        QPushButton *btn = new QPushButton(subOption);
        btn->setStyleSheet("text-align: left; padding: 8px;");

        layout->addWidget(btn);
    }

    layout->addStretch();

    return panel;
}

void PDFManager::setupPDFButton(PDFInfo &pdf, PDFCat &cat)
{
    int squareSize = 100; 

    QString filePath = QString::fromStdString(pdf.file_path);
    QFileInfo fileInfo(filePath);

    searchWidget->addDocument(filePath);

    if(hasCachedThumbnail(filePath) && pdf.total_page_num > 0) 
    {
        pdf.thumbnail = getCachedThumbnail(filePath);
    } 
    else 
    {
        std::unique_ptr<Poppler::Document> document(Poppler::Document::load(filePath));
        if (document && !document->isLocked()) 
        {
            pdf.total_page_num = document->numPages();

            std::unique_ptr<Poppler::Page> pdfPage(document->page(0));
    
            if (pdfPage) 
            {
    
                QImage image = pdfPage->renderToImage(72.0, 72.0); // 72 DPI
        
                pdf.thumbnail = QPixmap::fromImage(image).scaled(
                    squareSize, squareSize, 
                    Qt::KeepAspectRatio, 
                    Qt::SmoothTransformation
                );

                saveThumbnail(filePath, pdf.thumbnail);
            }
        }
    }

    pdf.button = new WordWrapButton(QString("%1 (Page %2 of %3)")
                                .arg(QString::fromStdString(pdf.file_name))
                                .arg(pdf.page_num)
                                .arg(pdf.total_page_num), nullptr);

    pdf.button->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);

    connect(pdf.button, &WordWrapButton::clicked, this,
        [this, filePath = pdf.file_path, button = cat.addButton]() 
        { 
            for (auto& myCat : PDFcats) {
                if (myCat.addButton == button) {
                    openPDF(myCat, QString::fromStdString(filePath)); 
                    break;
                }
            }
        });

    cat.layout->addWidget(pdf.button);

    //------------------------------------------------------------------------
            
    pdf.flowButton = new WordWrapButton("", nullptr);

    pdf.flowButton->setMinimumSize(squareSize, squareSize);
    pdf.flowButton->setMaximumSize(squareSize, squareSize);
    pdf.flowButton->setFixedSize(squareSize, squareSize);
    pdf.flowButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    pdf.flowButton->setIcon(QIcon(pdf.thumbnail));
    pdf.flowButton->setIconSize(QSize(squareSize, squareSize));
        
    pdf.flowButton->setToolTip(QString("%1 (Page %2 of %3)")
                        .arg(QString::fromStdString(pdf.file_name))
                        .arg(pdf.page_num)
                        .arg(pdf.total_page_num));

    connect(pdf.flowButton, &WordWrapButton::clicked, this,
        [this, filePath = pdf.file_path, button = cat.addButton]() 
        { 
            for (auto& myCat : PDFcats) {
                if (myCat.addButton == button) {
                    openPDF(myCat, QString::fromStdString(filePath)); 
                    break;
                }
            }
        });

    cat.flowLayout->addWidget(pdf.flowButton);

    updateTimestamps(pdf, pdf.last_opened_time);
}

bool PDFManager::hasCachedThumbnail(const QString& pdfFilePath)
{
    QFileInfo pdfInfo(pdfFilePath);
    QString thumbPath =  QDir::currentPath() + "/cache"+ "/" + pdfInfo.fileName() + ".thumb.png";
    QFileInfo thumbInfo(thumbPath);
        
    return thumbInfo.exists() && thumbInfo.lastModified() >= pdfInfo.lastModified();
}

QPixmap PDFManager::getCachedThumbnail(const QString& pdfFilePath)
{
    QPixmap result;
    QFileInfo pdfInfo(pdfFilePath);

    QString thumbPath = QDir::currentPath() + "/cache"+ "/" + pdfInfo.fileName() + ".thumb.png";
        
    if (result.load(thumbPath)) {
        return result;
    }
        
    return QPixmap(); // Return empty pixmap if not found
}

void PDFManager::saveThumbnail(const QString& pdfFilePath, const QPixmap& thumbnail)
{
    QDir cacheDir(QDir::currentPath() + "/cache");
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    
    QFileInfo pdfInfo(pdfFilePath);
    QString thumbPath = QDir::currentPath() + "/cache"+ "/" + pdfInfo.fileName() + ".thumb.png";

    if (!thumbnail.save(thumbPath, "PNG")) {
        qWarning() << "Failed to save thumbnail:" << thumbPath;
    }
}

void PDFManager::initApp() 
{
    for (auto& cat : PDFcats)
    {
        PDFManager::setupNewCat(cat);

        toolbox->addItem(cat.container, cat.category.c_str());

        for (auto& pdf : cat.PDFFiles)
        {
            setupPDFButton(pdf, cat);
        }
    }

    QWidget *dummyWidget = new QWidget();
    dummyWidget->setFixedHeight(0);
    dummyWidget->setVisible(false);

    dummyIndex = toolbox->addItem(dummyWidget, "");

    setupToolBoxConnections(); 
}

void PDFManager::setFont() 
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
        qWarning() << "Failed to load font from path:" << fontPath << ", a default font is used instead !";
        defaultFont = new QFont("Open Sans", 18);
    }

    QApplication::setFont(*defaultFont);
}

void PDFManager::createMenuBar() 
{
    menuBar = QMainWindow::menuBar();

    // File menu
    fileMenu = menuBar->addMenu("&File");

    QAction *newAction = fileMenu->addAction(QIcon::fromTheme("document-new"), "&New");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered,
            this, &PDFManager::menuBarActionStub);

    QAction *openAction = fileMenu->addAction(QIcon::fromTheme("document-open"), "&Open...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    QAction *saveAction = fileMenu->addAction(QIcon::fromTheme("document-save"), "&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, 
        [this]()
        {
            dirty = false;
            if (!serializeData()) 
            {
                QMessageBox::critical(this, "Error",
                                      "Failed to save data.",
                                      QMessageBox::Ok);
            }
        });

    exitAction = fileMenu->addAction(QIcon::fromTheme("application-exit"), "E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, 
            qApp, &QApplication::quit);

    fileMenu->addSeparator();

    // Edit menu
    editMenu = menuBar->addMenu("&Edit");

    QAction *undoAction = editMenu->addAction(QIcon::fromTheme("edit-undo"), "&Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    QAction *redoAction = editMenu->addAction(QIcon::fromTheme("edit-redo"), "&Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered,
            this, &PDFManager::menuBarActionStub);

    editMenu->addSeparator();

    QAction *cutAction = editMenu->addAction(QIcon::fromTheme("edit-cut"), "Cu&t");
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    QAction *copyAction = editMenu->addAction(QIcon::fromTheme("edit-copy"), "&Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    QAction *pasteAction = editMenu->addAction(QIcon::fromTheme("edit-paste"), "&Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    // View menu
    viewMenu = menuBar->addMenu("&View");

    // Help menu
    helpMenu = menuBar->addMenu("&Help");

    QAction *aboutAction = helpMenu->addAction(QIcon::fromTheme("help-about"), "&About");
    connect(aboutAction, &QAction::triggered, 
            this, &PDFManager::menuBarActionStub);

    QFont menuFont("Open Sans", 18);
    menuBar->setFont(*defaultFont);

    foreach (QWidget *widget, menuBar->findChildren<QWidget *>()) 
    {
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

void PDFManager::createSearchbar()
{
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");
    searchBar->setFont(*defaultFont);

    connect(searchBar, &QLineEdit::textChanged, 
            this, &PDFManager::filterToolBoxItems);
}

void PDFManager::createCategoriesArea() 
{
    categoriesArea = new QScrollArea(this);
    categoriesArea->setFrameShape(QFrame::NoFrame);
    categoriesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    categoriesArea->setWidgetResizable(true);
    categoriesArea->setMinimumWidth(250);
    categoriesArea->setStyleSheet(
        "QScrollArea {"
        "    background-color: #1e1e1e;"   
        "    border: none;"                
        "}"
    );

    toolbox = new QToolBox(this);
    toolbox->setStyleSheet(
        "QToolBox {"
        "    background-color: #1e1e1e;"  
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
        "    background-color: #1a1a1a;"   
        "    color: #6d6d6d;"              
        "}"
    );
    toolbox->setFixedHeight(600);

    // make toolbox scrollable
    categoriesArea->setWidget(toolbox);
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

        dirty = true;
    }
}

void PDFManager::setupToolBoxConnections() 
{
    // disconnect first so we dont get multiple invocations
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

void PDFManager::collapseAllToolBoxItems() 
{
    toolbox->setCurrentIndex(dummyIndex);
    toolbox->setItemText(dummyIndex, "");
}

void PDFManager::filterToolBoxItems() 
{
    QString searchText = searchBar->text();
    
    if (searchText.isEmpty()) 
    {
        for (auto &cat : PDFcats) {
            for (auto &pdf : cat.PDFFiles) {
                pdf.button->setVisible(true);
                pdf.flowButton->setVisible(true);
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
        for (auto &pdf : cat.PDFFiles) 
        {
            bool match = QString::fromStdString(pdf.file_name)
                                .contains(searchText, Qt::CaseInsensitive);
            pdf.button->setVisible(match);
            pdf.flowButton->setVisible(match);
            // at least one child matches for the toolbox to be enabled
            if (match){
                childMatch = true;
                // expand it also if it matches
                toolbox->setCurrentIndex(idx);
            }
        }
        toolbox->setItemEnabled(idx, childMatch);
        idx++;
    }
}

#if 0
QMap<int, QPair<QString, QWidget *>> removedItems;
void PDFManager::filterToolBoxItems(QToolBox *toolbox, const QString &searchText) 
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

void PDFManager::switchPDFView() 
{
    for (auto& cat : PDFcats) {
        int nextIndex = (cat.stackedWidget->currentIndex() + 1) % cat.stackedWidget->count();
        cat.stackedWidget->setCurrentIndex(nextIndex);

        QWidget* parent = cat.container->parentWidget();
        while (parent) {
            QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parent);
            if (scrollArea) {
                scrollArea->verticalScrollBar()->setValue(0); 
                break;
            }

            QScrollBar* scrollBar = parent->findChild<QScrollBar*>();
            if (scrollBar) {
                scrollBar->setValue(0); 
                break;
            }

            parent = parent->parentWidget();
        }
    }
}

void PDFManager::sortCatPDFs(PDFCat& category, CompareFunc sortfunc) 
{
    category.layout->setCompareFunc(sortfunc);
    category.flowLayout->setCompareFunc(sortfunc);
}

void PDFManager::setupNewCat(PDFCat& cat)
{
    cat.container = new QWidget(this);
    cat.container->setStyleSheet("QWidget {"
                                 "    background-color: #1e1e1e;"
                                 "}");

    auto VViewLayout = new QVBoxLayout();

    cat.layout = new SortedVBoxLayout(timestampSort());
    cat.layout->setSpacing(5);
    cat.layout->setContentsMargins(30, 10, 30, 10);

    // Add and center an add button to add new pdfs
    cat.addButton = new QPushButton("+");
    cat.addButton->setFixedSize(30, 30);

    auto centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(cat.addButton);
    centerLayout->addStretch();

    VViewLayout->addLayout(cat.layout);
    VViewLayout->addLayout(centerLayout);
    VViewLayout->addStretch(1);

    connect(cat.addButton, &QPushButton::clicked, this,
            // I may sort categories or do other stuff like edit the name 
            // so I have to look up the category using the button
            [this, button = cat.addButton]() 
            {
                for (auto& myCat : PDFcats) {
                    if (myCat.addButton == button) {
                        this->addNewPDF(myCat);
                        break;
                    }
                }
            });

    auto FViewLayout = new QVBoxLayout();

    cat.flowLayout = new SortedFlowLayout(timestampSort());
    cat.flowLayout->setSpacing(5);
    cat.flowLayout->setContentsMargins(30, 10, 30, 10);

    cat.flowAddButton = new QPushButton("+");
    cat.flowAddButton->setFixedSize(30, 30);

    auto flowCenterLayout = new QHBoxLayout();
    flowCenterLayout->addStretch();
    flowCenterLayout->addWidget(cat.flowAddButton);
    flowCenterLayout->addStretch();
    //cat.flowAddButton->setMinimumSize(squareSize, squareSize);
    //cat.flowAddButton->setMaximumSize(squareSize, squareSize);
    //cat.flowAddButton->setFixedSize(squareSize, squareSize);
    //cat.flowAddButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    FViewLayout->addLayout(cat.flowLayout);
    FViewLayout->addLayout(flowCenterLayout);
    FViewLayout->addStretch(1);

    connect(cat.flowAddButton, &QPushButton::clicked, this,
        // I may sort categories or do other stuff like edit the name 
        // so I have to look up the category using the button
        [this, button = cat.flowAddButton]() 
        {
            for (auto& myCat : PDFcats) {
                if (myCat.flowAddButton == button) {
                    this->addNewPDF(myCat);
                    break;
                }
            }
        });

    cat.stackedWidget = new QStackedWidget(cat.container);
    
    QWidget* vboxContainer = new QWidget();
    QWidget* flowContainer = new QWidget();
    
    vboxContainer->setLayout(VViewLayout);       
    flowContainer->setLayout(FViewLayout);   
    
    cat.stackedWidget->addWidget(vboxContainer); // Index 0
    cat.stackedWidget->addWidget(flowContainer); // Index 1
        
    QVBoxLayout* containerLayout = new QVBoxLayout(cat.container);
    containerLayout->addWidget(cat.stackedWidget);
}

void PDFManager::setupNewPDF(PDFCat &category, QString &filePath) 
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    qInfo() << filePath << "," << fileName;

    // check whether this pdf already exists or not
    bool isDuplicate = false;
    for (auto &pdf : category.PDFFiles) 
    {
        if (pdf.file_name == fileName.toStdString()) {
            isDuplicate = true;
            QMessageBox::information(this, "Duplicate File",
                                     "This PDF file is already in your list.");
            break;
        }
    }
    if (!isDuplicate) 
    {
        PDFInfo newPDF;
        
        newPDF.file_name = fileName.toStdString();
        newPDF.file_path = filePath.toStdString();

        setupPDFButton(newPDF, category);
        updateTimestamps(newPDF);
            
        category.PDFFiles.push_back(newPDF);

        dirty = true;
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

void PDFManager::updateTimestamps(PDFInfo &pdf, qint64 time)
{
    pdf.last_opened_time = time;
    pdf.button->setTimestamp(time);
    pdf.flowButton->setTimestamp(time);
}

void PDFManager::updateTimestamps(PDFInfo &pdf)
{
    PDFManager::updateTimestamps(pdf, QDateTime::currentSecsSinceEpoch());
}

void PDFManager::openPDF(PDFCat &cat, const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    QProcess* process = new QProcess(this);

    if (processToPDF.isEmpty()) {
        firstProcess = true;
    }else{
        firstProcess = false;
    }

    processToPDF[process] = fileName;

    process->setProperty("autoDelete", true);
   
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, button = cat.addButton](int exitCode, QProcess::ExitStatus exitStatus) 
            {
                for (auto& myCat : PDFcats) {
                    if (myCat.addButton == button) {
                        this->handleFinished(exitCode, exitStatus, myCat);
                        break;
                    }
                }       
            });

    connect(process, &QObject::destroyed, this,
            [this, process]() {
                qDebug() << "Process Destroyed"; 
            });

    connect(process, &QProcess::started, this,
            [this]() { 
                qDebug() << "Process started successfully"; 
            });

    connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError error) 
            {
                if (error == QProcess::FailedToStart) 
                {
                    QMessageBox::critical(this, "Error",
                                          "Failed to start Process");
                    processToPDF.remove(process);
                    process->deleteLater();
                }
            });

#ifdef Q_OS_WIN
    QStringList arguments;
        
    // Open to stored page
    for (auto& pdf : cat.PDFFiles) 
    {
        if ((QString::fromStdString(pdf.file_name) == fileName)) 
        {
            arguments << QString("-new-window");
            arguments << QString("-view");
            arguments << QString("single page");
            arguments << QString("-page");
            arguments << QString::number(pdf.page_num);

            updateTimestamps(pdf);
            sortCatPDFs(cat, timestampSort());
            break;
        }
    }
    arguments << filePath;

    qDebug()<< arguments;

    // Kill sumatra as we depend on the first process
    if(firstProcess) {
        QProcess killProcess;
        killProcess.start("taskkill", QStringList() << "/F" << "/IM" << "SumatraPDF.exe");
        killProcess.waitForFinished();
        QThread::msleep(500);
    }
        
    process->start("C:\\Users\\zezo_\\AppData\\Local\\SumatraPDF\\SumatraPDF.exe", arguments);
#endif
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
        
    if (exitCode != 0) 
    {
        // Sumatra reuses the same process so 
        // just update the number when all windows are closed
        qDebug() << "Exit code : "
                 << exitCode << "\n";
        return;
    } 
    
    for (auto it = processToPDF.begin(); it != processToPDF.end(); ++it) 
    {
        QProcess *process = it.key();      // Get the QProcess pointer (key)

        QString fileName = it.value();
        if (fileName.isEmpty()) 
            continue;

        for (auto& pdf : category.PDFFiles) 
        {
            if (QString::fromStdString(pdf.file_name) == fileName) 
            {
                PDFInfo tempPdf(pdf.file_name);

        #ifdef Q_OS_WIN
                tempPdf.parseSumatraSettings("C:\\Users\\zezo_\\AppData\\Local\\SumatraPDF\\SumatraPDF-settings.txt");
        #endif        
                pdf.page_num = tempPdf.page_num;
                
                if (pdf.button) 
                {
                    pdf.button->setText(QString("%1 (Page %2 of %3)")
                        .arg(QString::fromStdString(pdf.file_name))
                        .arg(pdf.page_num)
                        .arg(pdf.total_page_num));
                }

                if (pdf.flowButton) 
                {
                    pdf.flowButton->setToolTip(QString("%1 (Page %2 of %3)")
                        .arg(QString::fromStdString(pdf.file_name))
                        .arg(pdf.page_num)
                        .arg(pdf.total_page_num));
                }
                
                qDebug() << "Updated page number for" << QString::fromStdString(pdf.file_name) 
                            << "to page:" << pdf.page_num;
                dirty = true;

                break;
            }
        }

        qDebug() << "Process:" << process << "PDF Name:" << fileName;
    }
    processToPDF.clear();
}

void PDFManager::loadData() 
{
#ifdef Q_OS_WIN
    QString currentDirConfigPath = QDir::currentPath() + "\\" + configPath;
#endif 

    std::ifstream in(currentDirConfigPath.toStdString());

    if (!in.is_open()){
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
                // skip empty lines
            }
        }
    }
}

void PDFManager::loadConfig()
{
#ifdef Q_OS_WIN
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
#endif
}

bool PDFManager::serializeData() 
{
#ifdef Q_OS_WIN
    QString currentDirConfigPath = QDir::currentPath() + "\\" + "pdfmanager.conf";
#endif
    std::ofstream file(currentDirConfigPath.toStdString());

    if (!file.is_open()) 
    {
        return false;
    }

    for (auto &cat : PDFcats) 
    {
        file << cat.category << "," << cat.PDFFiles.size() << "\n";

        for (const auto &pdf : cat.PDFFiles) 
        {
            file <<  "\"" << pdf.file_name <<  "\"" <<  "," 
                 <<  "\"" << pdf.file_path << "\"" << "," 
                 << pdf.page_num << "," << pdf.total_page_num << "," 
                 << pdf.last_opened_time
                 << "\n";
        }

        file << "\n";
    }
    
    return true;
}

bool PDFManager::verifyFilePath(PDFInfo &pdf) 
{
    QFileInfo fileInfo(QString::fromStdString(pdf.file_path));

    if (!fileInfo.exists()) 
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("File not found: " + QString::fromStdString(pdf.file_name));
        msgBox.setInformativeText("The file at path: " + QString::fromStdString(pdf.file_path) + " could not be found. Would you like to locate it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        
        int ret = msgBox.exec();
        
        if (ret == QMessageBox::Yes) 
        {
            QString startDir = QDir::homePath();
            // Try to use the directory from the original path as starting point
            QFileInfo origPathInfo(QString::fromStdString(pdf.file_path));
            if (origPathInfo.dir().exists()) {
                startDir = origPathInfo.dir().path();
            }
            
            QString newPath = QFileDialog::getOpenFileName(
                nullptr,                                             // parent
                "Locate " + QString::fromStdString(pdf.file_name),   // caption
                startDir,                                            // directory
                "PDF Files (*.pdf);;All Files (*.*)"                 // filter
            );
            
            if (!newPath.isEmpty()) 
            {
                pdf.file_path = newPath.toStdString();
                return true;
            } 
            else 
            {
                // User canceled the dialog
                QMessageBox skipMsg;
                skipMsg.setIcon(QMessageBox::Question);
                skipMsg.setText("No file selected");
                skipMsg.setInformativeText("Do you want to skip this file?");
                skipMsg.setStandardButtons(QMessageBox::Yes);
                skipMsg.setDefaultButton(QMessageBox::Yes);
                
                return false;
            }
        }
        else 
        {
            QMessageBox skipMsg;
            skipMsg.setIcon(QMessageBox::Question);
            skipMsg.setText("File not located");
            skipMsg.setInformativeText("Do you want to skip this file?");
            skipMsg.setStandardButtons(QMessageBox::Yes);
            skipMsg.setDefaultButton(QMessageBox::Yes);
            
            return false;
        }
    }
    return true;
}

// TODO: error detection this shits its pants if the filename has a comma 
// I dont really know how to fix it without changing the format
// maybe add quotes ?? 
bool PDFManager::deserializePDFCat(std::istream &in, PDFCat &cat) 
{
    std::string line;

    if (!std::getline(in, line)) {
        return false;
    }

    line = trim(line);

    if(line.empty()) {
        return false;
    }

    // Category
    size_t comma_pos = line.find(",");
    if (comma_pos == std::string::npos) {
        return false;
    }
    cat.category = line.substr(0, comma_pos);
    int fileCount = std::stoi(line.substr(comma_pos + 1));

    cat.PDFFiles.clear();

    // Category entries
    for (int i = 0; i < fileCount; i++) 
    {
        if (!std::getline(in, line)) {
            return false;
        }

        PDFInfo pdf;
        size_t curr_pos = 0;

        // File name
        size_t fileNameQuote1 = line.find('"', curr_pos);
        if (fileNameQuote1 == std::string::npos) 
            return false;
        size_t fileNameQuote2 = line.find('"', fileNameQuote1 + 1);
        if (fileNameQuote2 == std::string::npos) 
            return false;
        pdf.file_name = line.substr(fileNameQuote1 + 1, fileNameQuote2 - fileNameQuote1 - 1);
        curr_pos = fileNameQuote2 + 2; 

        // File Path
        size_t filePathQuote1 = line.find('"', curr_pos);
        if (filePathQuote1 == std::string::npos) 
            return false;
        size_t filePathQuote2 = line.find('"', filePathQuote1 + 1);
        if (filePathQuote2 == std::string::npos) 
            return false;
        pdf.file_path = line.substr(filePathQuote1 + 1, filePathQuote2 - filePathQuote1 - 1);
        curr_pos = filePathQuote2 + 2; 

        if(!verifyFilePath(pdf)){
            continue;
        }

        std::string remaining = line.substr(curr_pos);

        std::istringstream iss(remaining);

        std::string page_num_str;
        std::getline(iss, page_num_str, ',');  
        if (page_num_str.empty()) {
            return false;
        }
        pdf.page_num = std::stoi(page_num_str);

        std::string total_page_number_str;
        std::getline(iss, total_page_number_str, ',');  
        if (total_page_number_str.empty()) {
            return false;
        }
        pdf.total_page_num = std::stoi(total_page_number_str);
        
        std::string page_timestamp_str;
        std::getline(iss, page_timestamp_str);  
        if (page_timestamp_str.empty()) {
            return false;
        }
        pdf.last_opened_time = std::stoll(page_timestamp_str);

        cat.PDFFiles.push_back(pdf);
    }

    if (!std::getline(in, line)) {
        return true; 
    }
    if (!trim(line).empty()) {
        return false;
    }
    
    return true;
}

void PDFManager::closeEvent(QCloseEvent *event) 
{
    if(dirty)
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
}
