#include "graphicscene.h"
#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QWheelEvent>
#include <QQueue>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QSettings>

GraphicsScene::GraphicsScene(QWidget *parent)
    : QGraphicsView(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    scene = new QGraphicsScene(-shift, -shift, 1400, 950, this);
    setMouseTracking(true);
    this->setScene(scene);

    QFont font1;
    font1.setPointSize(36);
    font1.setBold(true);

    QLabel *label1 = new QLabel(this);
    label1->setObjectName("label");
    label1->setGeometry(QRect(270, 850, 61, 61));
    label1->setFont(font1);
    label1->setText(QCoreApplication::translate("GraphicsScene", "W", nullptr));

    QLabel *label2 = new QLabel(this);
    label2->setObjectName("label2");
    label2->setGeometry(QRect(760, 850, 61, 61));
    label2->setFont(font1);
    label2->setText(QCoreApplication::translate("GraphicsScene", "H", nullptr));

    QFont font2;
    font2.setPointSize(28);
    font2.setBold(true);

    widthText = new QLineEdit(this);
    widthText->setObjectName("width");
    widthText->setGeometry(QRect(330, 860, 75, 41));
    widthText->setFont(font2);

    heightText = new QLineEdit(this);
    heightText->setObjectName("height");
    heightText->setGeometry(QRect(810, 860, 75, 41));
    heightText->setFont(font2);

    tableGroup = new QGraphicsItemGroup;
    pathGroup = new QGraphicsItemGroup;
    scene->addItem(tableGroup);
    scene->addItem(pathGroup);
    blackPen = new QPen(Qt::black, 3, Qt::SolidLine,  Qt::RoundCap);

    readSettings();
}

GraphicsScene::~GraphicsScene()
{
    writeSettings();
}

void GraphicsScene::setPoint(int row, int column, QString p)
{
    table[row][column]->setBrush(*redBrush);
    QFont font;
    font.setPointSize(30);
    if (p == "A") {
        textA = scene->addText("A", font);
        textA->setPos(column * square_size - textA->boundingRect().width()/2.0 + square_size / 2,
                      row * square_size -  textA->boundingRect().height()/2.0 + square_size / 2);
    } else {
        textB = scene->addText("B", font);
        textB->setPos(column * square_size - textB->boundingRect().width()/2.0 + square_size / 2,
                      row * square_size -  textB->boundingRect().height()/2.0 + square_size / 2);
    }
}

void GraphicsScene::removePoint(int row, int column, bool isA)
{
    table[row][column]->setBrush(*whiteBrush);
    if (isA) {
        scene->removeItem(textA);
        delete textA;
    } else {
        scene->removeItem(textB);
        delete textB;
    }
}

void GraphicsScene::distributePoints(int row, int column)
{
    if (table[row][column]->brush() == *blackBrush)
        return;
    if (!isSetA) {
        if (isSetB && row_B == row && column_B == column) {
            removePoint(row, column, false);
            isSetB = false;
        } else {
            setPoint(row, column, "A");
            isSetA = true;
            row_A =  row;
            column_A = column;
            if(isSetB) paintPath();
         }
    } else {
        if (row_A == row && column_A == column) {
            removePoint(row, column, true);
            isSetA = false;
            deletePath();
        } else if (isSetB && row_B == row && column_B == column) {
            removePoint(row, column, false);
            isSetB = false;
            deletePath();
        } else {
            if (isSetB)
                removePoint(row_B, column_B, false);
            deletePath();
            setPoint(row, column, "B");
            isSetB = true;
            row_B = row;
            column_B = column;
            paintPath();
        }
    }
}

void GraphicsScene::wheelEvent(QWheelEvent * event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0/scaleFactor, 1.0/scaleFactor);
    }
}

void GraphicsScene::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        _pan = true;
        _panStartX = event->pos().x();
        _panStartY = event->pos().y();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    } else if (event->button() == Qt::LeftButton) {
        QPointF scenePt = mapToScene(event->pos());
        QGraphicsItem *item = scene->itemAt(scenePt, transform());
        if (item == nullptr) {
            event->accept();
            return;
        }
        if (item == textA)  {
            distributePoints(row_A, column_A);
            event->accept();
            return;
        }
        if (item == textB) {
            distributePoints(row_B, column_B);
            event->accept();
            return;
        }
        QPointF localPt = item->mapFromScene(scenePt);
        int x = localPt.x();
        int y = localPt.y();
        if (0 <= x && x <= square_size * columnCount &&
                0 <= y && y <= square_size * rowCount)
        {
            int row = y / square_size;
            int column = x / square_size;
            distributePoints(row, column);
        }
        event->accept();
        return;
    }
    event->ignore();
}

void GraphicsScene::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        _pan = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    event->ignore();
}

void GraphicsScene::mouseMoveEvent(QMouseEvent *event)
{
    if (_pan) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->pos().x() - _panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->pos().y() - _panStartY));
        _panStartX = event->pos().x();
        _panStartY = event->pos().y();
        event->accept();
        return;
    }
    event->ignore();

}

void GraphicsScene::createTable(int new_rowCount, int new_columnCount)
{
    if (isSetA)
        removePoint(row_A, column_A, true);
    if (isSetB)
        removePoint(row_B, column_B, false);
    isSetA = false; isSetB = false;
    deletePath();

    if (table.size() != 0) {
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < columnCount; j++) {
                scene->removeItem(table[i][j]);
                delete table[i][j];
            }
        }
    }
    if (isSetA) {
        removePoint(row_A, column_A, true);
        isSetA = false;
    }
    if (isSetB) {
        removePoint(row_B, column_B, true);
        isSetB = false;
    }
    rowCount = std::min(new_rowCount, 200);
    columnCount = std::min(new_columnCount, 200);


    srand(time(NULL));
    table = QVector<QVector <QGraphicsRectItem*>>(rowCount);

    for(int i = 0 ; i < rowCount; i++) {
        for(int j = 0 ; j < columnCount; j++) {
            QGraphicsRectItem *rect;
            if (((rand() % 100)) < 16) {
                rect = scene->addRect(square_size * j, square_size * i, square_size, square_size, *blackPen, *blackBrush);
            }
            else {
                rect = scene->addRect(square_size * j, square_size * i, square_size, square_size, *blackPen, *whiteBrush);
            }
            table[i].push_back(rect);
            tableGroup->addToGroup(rect);
        }
    }
}

void GraphicsScene::paintPath()
{
    QFuture future = QtConcurrent::run([this] {return findPath();});
    if (!future.result()) {
        QMessageBox::information(this, "Failure", "Пути нет");
        return;
    }

    QPen greenPen = QPen(Qt::darkGreen, 3, Qt::SolidLine,  Qt::RoundCap);
    QVector<int> rowShift   {0, -1, 0, 1};
    QVector<int> columnShift{-1, 0, 1, 0};
    int prevRow = row_B, prevColumn = column_B;
    int x1 = column_B * square_size + square_size / 2;
    int y1 = row_B * square_size + square_size / 2;
    while (prevRow != row_A || prevColumn != column_A) {
        for (int i = 0; i < 4; i++) {
            int row = prevRow + rowShift[i];
            int column = prevColumn + columnShift[i];
            if (0 <= row && row < rowCount
                    && 0 <= column && column < columnCount
                    && (map[row][column] + 1) == map[prevRow][prevColumn]
                    && map[row][column] != -2)
            {

                int x2 = x1 + square_size * columnShift[i];
                int y2 = y1 + square_size * rowShift[i];
                QGraphicsLineItem * line = scene->addLine(x1, y1, x2, y2, greenPen);
                path.push_back(line);
                pathGroup->addToGroup(line);
                prevRow = row;
                prevColumn = column;
                x1 = x2;
                y1 = y2;
                break;
            }
        }
    }
}

bool GraphicsScene::findPath()
{
    QVector<int> rowShift   {0, -1, 0, 1};
    QVector<int> columnShift{-1, 0, 1, 0};
    QQueue<Point> q;
    q.push_back({row_A, column_A});
    map =  QVector<QVector <int>>(rowCount);
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
            if (table[i][j]->brush() == *blackBrush) {
                map[i].push_back(-1);
            } else {
                map[i].push_back(-2);
            }
        }
    }
    map[row_A][column_A] = 0;
    while (!q.empty()) {
        Point p = q.front();
        if (p.row == row_B && p.column == column_B)
            return true;
        for (int i = 0; i < 4; i++) {
            int row = p.row + rowShift[i];
            int column = p.column + columnShift[i];

            if(0 <= row && row < rowCount
               && 0 <= column && column < columnCount
               && map[row][column] == -2)
            {
                map[row][column] = map[p.row][p.column] + 1;
                q.push_back({row, column});

            }

        }
        q.pop_front();
    }
    return false;
}

void GraphicsScene::deletePath()
{
    for (int i = 0; i < path.size(); i++) {
        scene->removeItem(path[i]);
        delete path[i];
    }
    path.clear();
}

void GraphicsScene::on_pushButton_clicked()
{
    bool widthOk;
    bool heightOk;
    int row = this->heightText->text().toInt(&heightOk);
    int column = this->widthText->text().toInt(&widthOk);
    if (widthOk && heightOk) {
        if (row == 0)
            row = 10;
        if (column == 0)
            column = 10;
        createTable(row, column);
    } else {
        QMessageBox::information(this, "Error", "Ширина и высота должны быть целыми числами");
    }
}

void GraphicsScene::readSettings()
{
    QSettings settings("QtSettings", "GraphicScene");
    rowCount = settings.value("rowCount").toInt();
    columnCount = settings.value("columnCount").toInt();
    if (rowCount == 0 || columnCount == 0)  {
        rowCount = 10;
        columnCount = 10;
        createTable(10, 10);
        return;
    }
    table = QVector<QVector <QGraphicsRectItem*>>(rowCount);
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
            int item = settings.value("table" + QString::number(i) + "/" + QString::number(j)).toInt();
            QGraphicsRectItem *rect;
            if (item == -1) {
                rect = scene->addRect(square_size * j, square_size * i, square_size, square_size, *blackPen, *blackBrush);
            }
            else {
                rect = scene->addRect(square_size * j, square_size * i, square_size, square_size, *blackPen, *whiteBrush);
            }
            table[i].push_back(rect);
            tableGroup->addToGroup(rect);
        }
    }

    isSetA = settings.value("isSetA").toBool();
    if (isSetA) {
        row_A = settings.value("row_A").toInt();
        column_A = settings.value("column_A").toInt();
        setPoint(row_A, column_A, "A");
    }
    isSetB = settings.value("isSetB").toBool();
    if (isSetB) {
        row_B = settings.value("row_B").toInt();
        column_B = settings.value("column_B").toInt();
        setPoint(row_B, column_B, "B");
    }
    if (isSetA && isSetB)
        paintPath();

    settings.clear();
}


void GraphicsScene::writeSettings()
{
    QSettings settings("QtSettings", "GraphicScene");
    settings.setValue("rowCount", rowCount);
    settings.setValue("columnCount", columnCount);
    for (int i = 0; i < rowCount; i++) {
        settings.beginGroup("table" + QString::number(i));
        for (int j = 0; j < columnCount; j++) {
            if (table[i][j]->brush() == *blackBrush) {
                settings.setValue(QString::number(j), -1);
            } else {
                settings.setValue(QString::number(j), -2);
            }
        }
        settings.endGroup();
    }
    settings.setValue("isSetA", isSetA);
    settings.setValue("isSetB", isSetB);
    settings.setValue("row_A", row_A);
    settings.setValue("column_A", column_A);
    settings.setValue("row_B", row_B);
    settings.setValue("column_B", column_B);

}
