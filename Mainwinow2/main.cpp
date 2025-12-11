#include "Mainwinow2.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <iostream>

int main(int argc, char* argv[])
{
    // Simple command-line options: --doc or --describe prints project documentation and exits
    if (argc > 1) {
        QString arg1 = QString::fromLocal8Bit(argv[1]).toLower();
        if (arg1 == "--doc" || arg1 == "--describe") {
            // Try to open PROJECT_DOC.md from the current working directory
            QFile docFile("PROJECT_DOC.md");
            if (docFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&docFile);
                QString content = in.readAll();
                docFile.close();
                std::cout << content.toStdString() << std::endl;
                return 0;
            }

            // Fallback: print a short built-in description
            const char* fallback =
                "CSE (Qt/C++) demo banking app\n"
                "Use --doc to include detailed documentation (PROJECT_DOC.md not found in working directory).\n";
            std::cout << fallback << std::endl;
            return 0;
        }
        if (arg1 == "--help" || arg1 == "-h") {
            const char* help =
                "Usage:\n"
                "  Mainwinow2.exe            Launch GUI application\n"
                "  Mainwinow2.exe --doc      Print project documentation (PROJECT_DOC.md) and exit\n"
                "  Mainwinow2.exe --describe Alias for --doc\n"
                "  Mainwinow2.exe --help     Show this help message\n";
            std::cout << help << std::endl;
            return 0;
        }
    }

    QApplication a(argc, argv);
    Mainwinow2 w;
    w.show();
    return a.exec();
}