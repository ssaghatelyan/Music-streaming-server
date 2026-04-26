#include <QApplication>
#include "LoginWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(26, 26, 26));
    dark.setColor(QPalette::WindowText,      QColor(204, 204, 204));
    dark.setColor(QPalette::Base,            QColor(17, 17, 17));
    dark.setColor(QPalette::AlternateBase,   QColor(31, 31, 31));
    dark.setColor(QPalette::Text,            QColor(204, 204, 204));
    dark.setColor(QPalette::Button,          QColor(31, 31, 31));
    dark.setColor(QPalette::ButtonText,      QColor(204, 204, 204));
    dark.setColor(QPalette::Highlight,       QColor(124, 106, 245));
    dark.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    dark.setColor(QPalette::PlaceholderText, QColor(100, 100, 100));
    app.setPalette(dark);

    LoginWindow login;
    login.show();

    return app.exec();
}
