#include <iostream>
#include <QtCore>

using std::cerr;
using std::cout;

void usage(const char* binaryName)
{
    cerr << binaryName << " <argument>\n";
    cerr << "Arguments: include, lib, bin\n";
    exit(1);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    if (args.count() != 2) {
        usage(argv[0]);
    } else {
        QString arg = args.at(1);
        QLibraryInfo::LibraryLocation res = QLibraryInfo::PrefixPath;
        if (arg == "include") {
            res = QLibraryInfo::HeadersPath;
        } else if (arg == "lib") {
            res = QLibraryInfo::LibrariesPath;
        } else if (arg == "bin") {
            res = QLibraryInfo::BinariesPath;
        } else {
            usage(argv[0]);
        }
        cout << qPrintable(QLibraryInfo::location(res)) << "\n";
    }
    return 0;
}
