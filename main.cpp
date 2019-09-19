#include <QApplication>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QStyleFactory>
#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QFileSystemWatcher>

const QString menuStyles =
    "QMenu {"
        "background: #080808;"
        "color: #d6d6d6;"
    "}"
    "QMenu::item:selected {"
        "background: #242424;"
    "}";

const QString bbswitch = "/proc/acpi/bbswitch";

QString read(QString filename) {
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(nullptr, "error", file.errorString());
    }

    QTextStream in(&file);

    return in.readAll();
}

bool isOn() {
    return read(bbswitch).split(" ")[1].trimmed() != "OFF";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon iconOn(":/assets/on.png");
    QIcon iconOff(":/assets/off.png");

    QSystemTrayIcon tray(isOn() ? iconOn : iconOff);

    QMenu menu;
    menu.setStyleSheet(menuStyles);

    auto toggleAction = new QAction("Toggle GPU");
    auto checkAction = new QAction("Force-check status");
    auto quitAction = new QAction("Quit");

    QObject::connect(toggleAction, &QAction::triggered, [] {
        bool on = isOn();

        int exitCode = QProcess::execute("pkexec", QStringList() << "sh" << "-c" << QString("echo %1 >> %2 ").arg(on ? "OFF" : "ON", bbswitch));

        if (exitCode != 0) {
            QMessageBox::information(nullptr, "error", QString("Error toggling GPU.\nExit code: %1").arg(exitCode));
        }
    });

    QObject::connect(checkAction, &QAction::triggered, [&tray, iconOn, iconOff] {
        tray.setIcon(isOn() ? iconOn : iconOff);
    });

    QObject::connect(quitAction, &QAction::triggered, [] {
        std::exit(0);
    });

    menu.addAction(toggleAction);
    menu.addAction(checkAction);
    menu.addAction(quitAction);

    tray.setContextMenu(&menu);
    tray.show();

    QFileSystemWatcher watcher;
    watcher.addPath(bbswitch);

    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, [&tray, iconOn, iconOff] {
        tray.setIcon(isOn() ? iconOn : iconOff);
    });

    return a.exec();
}
