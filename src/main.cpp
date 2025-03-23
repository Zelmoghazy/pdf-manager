#include "pdfmanager.hpp"

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);

    PDFManager manager;
    manager.show();

    return app.exec();
}
