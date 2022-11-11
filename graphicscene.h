#ifndef MYGRAPHICSCENE_H
#define MYGRAPHICSCENE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QPen>
#include <QLineEdit>
#include <QLabel>
#include <QVector>

struct Point {
    int row;
    int column;
};

class GraphicsScene : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsScene(QWidget *parent = 0);
    ~GraphicsScene();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void on_pushButton_clicked();

private:
    void createTable(int new_rowCount, int new_columnCount);
    void setPoint(int row, int column, QString p);
    void removePoint(int row, int column, bool isA);
    void distributePoints(int row, int column);

    void readSettings();
    void writeSettings();

    void paintPath();
    bool findPath();
    void deletePath();

    int rowCount = 10;
    int columnCount = 10;
    int square_size = 50;
    int shift = 50;

    QGraphicsScene *scene;
    QGraphicsItemGroup *tableGroup;
    QVector<QVector <QGraphicsRectItem*>> table;
    QGraphicsItemGroup *pathGroup;
    QVector<QGraphicsLineItem*> path;

    QPen *blackPen;
    QBrush *redBrush = new QBrush(Qt::red);
    QBrush *whiteBrush = new QBrush(Qt::white);
    QBrush *blackBrush = new QBrush(Qt::black);
    QLineEdit *widthText;
    QLineEdit *heightText;
    QGraphicsTextItem *textA;
    QGraphicsTextItem *textB;

    QVector<QVector<int>> map;
    bool isSetA = false;
    bool isSetB = false;
    int row_A;
    int column_A;
    int row_B;
    int column_B;

    bool _pan = false;
    int _panStartX, _panStartY;

};

#endif // MYGRAPHICSCENE_H
