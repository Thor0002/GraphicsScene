#include "graphicscene.h"

#include <QApplication>
#include <QPushButton>
#include <QSettings>
#include <QDataStream>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GraphicsScene graphicsView;
    graphicsView.setObjectName("graphicsView");
    graphicsView.setGeometry(QRect(0, 0, 1400, 950));
    graphicsView.setWindowTitle("GraphicsView");

    QPushButton pushButton(&graphicsView);
    pushButton.setObjectName("pushButton");
    pushButton.setGeometry(QRect(1250, 0, 100, 40));
    pushButton.setText(QCoreApplication::translate("GraphicsScene", "Генерировать", nullptr));

    graphicsView.show();
    QMetaObject::connectSlotsByName(&graphicsView);
    return a.exec();
}
