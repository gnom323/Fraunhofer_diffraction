#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QScrollArea>
#include <QVector>
#include <QPair>
#include <QComboBox>
#include "diffractioncalculator.h"

// Класс для пустой области отображения
class EmptyDisplayArea : public QLabel {
    Q_OBJECT

public:
    explicit EmptyDisplayArea(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void drawGraph(const QVector<QPair<double, double>>& data, double lambda, double wide);
};

// Класс для кастомного заголовка окна
class CustomTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void closeClicked();

private:
    QPushButton *closeButton;
    QPoint dragPosition;
    bool dragging;
};

// Главное окно приложения
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void closeWindow();
    void calculateAndDraw();

private:
    QLineEdit *lambdaInputField;
    QLineEdit *wideInputField;
    QPushButton *lambdaButton;
    QPushButton *wideButton;
    QPushButton *calculateButton;
    QComboBox *methodComboBox;      // <--- ОБЪЯВЛЕНИЕ
    QLabel *lambdaValueDisplay;
    QLabel *wideValueDisplay;
    EmptyDisplayArea *emptyArea;
    CustomTitleBar *titleBar;
    QWidget *centralWidget;
    QVBoxLayout *mainContainerLayout;

    double lambda;
    double wide;

    DiffractionCalculator *calculator;
};

#endif // MAINWINDOW_H