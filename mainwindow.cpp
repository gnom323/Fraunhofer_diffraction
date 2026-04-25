#include "mainwindow.h"
#include "diffractioncalculator.h"
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>

// ========== РЕАЛИЗАЦИЯ EmptyDisplayArea ==========

EmptyDisplayArea::EmptyDisplayArea(QWidget *parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    setStyleSheet("border: 2px solid #cccccc; background-color: #f5f5f5;");
    setText("Правая область\n(пусто)\nРазмер: 900x600\n\nВведите значения и нажмите 'Рассчитать'");
    setFixedSize(900, 600);
}

QSize EmptyDisplayArea::sizeHint() const {
    return QSize(900, 600);
}

QSize EmptyDisplayArea::minimumSizeHint() const {
    return QSize(900, 600);
}

void EmptyDisplayArea::drawGraph(const QVector<QPair<double, double>>& data, double lambda, double wide) {
    if (data.isEmpty()) {
        QLabel::setText("Нет данных для отображения");
        return;
    }

    // Создаем изображение для графика
    QImage image(880, 580, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // Настройки отступов
    int leftMargin = 70;
    int rightMargin = 40;
    int topMargin = 60;
    int bottomMargin = 50;
    int graphWidth = image.width() - leftMargin - rightMargin;
    int graphHeight = image.height() - topMargin - bottomMargin;

    // Находим максимальные значения
    double maxIntensity = 0.0;
    double minAngle = data.first().first;
    double maxAngle = data.last().first;

    for (const auto& point : data) {
        if (point.second > maxIntensity) {
            maxIntensity = point.second;
        }
    }

    // Рисуем оси
    painter.setPen(QPen(Qt::black, 2));

    // Ось Y
    painter.drawLine(leftMargin, topMargin, leftMargin, image.height() - bottomMargin);
    // Ось X
    painter.drawLine(leftMargin, image.height() - bottomMargin,
                     image.width() - rightMargin, image.height() - bottomMargin);

    // Рисуем стрелки на осях
    painter.drawLine(leftMargin, topMargin, leftMargin - 5, topMargin + 10);
    painter.drawLine(leftMargin, topMargin, leftMargin + 5, topMargin + 10);
    painter.drawLine(image.width() - rightMargin, image.height() - bottomMargin,
                     image.width() - rightMargin - 10, image.height() - bottomMargin - 5);
    painter.drawLine(image.width() - rightMargin, image.height() - bottomMargin,
                     image.width() - rightMargin - 10, image.height() - bottomMargin + 5);

    // Подписи осей
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(image.width() / 2 - 50, image.height() - 8, "Угол θ (градусы)");

    // Вертикальная ось
    painter.save();
    painter.translate(18, image.height() / 2);
    painter.rotate(-90);
    painter.drawText(-30, 0, "Нормированная интенсивность");
    painter.restore();

    // Определяем шаг для меток на оси X (кратный 5)
    int xTickStep = 5;

    // Находим ближайшие кратные 5 значения
    int firstTick = (static_cast<int>(ceil(minAngle / xTickStep)) * xTickStep);
    int lastTick = (static_cast<int>(floor(maxAngle / xTickStep)) * xTickStep);

    // Рисуем метки на оси X с шагом 5
    painter.setFont(QFont("Arial", 9));
    painter.setPen(QPen(Qt::black, 1));

    for (int angle = firstTick; angle <= lastTick; angle += xTickStep) {
        if (angle >= minAngle && angle <= maxAngle) {
            int x = leftMargin + (angle - minAngle) / (maxAngle - minAngle) * graphWidth;
            painter.drawLine(x, image.height() - bottomMargin, x, image.height() - bottomMargin + 6);

            QString label;
            if (angle == 0) {
                label = "0";
            } else if (angle > 0) {
                label = QString::number(angle);
            } else {
                label = QString::number(angle);
            }

            painter.drawText(x - 12, image.height() - bottomMargin + 22, label);
        }
    }

    // Добавляем дополнительную метку в начале и конце если нужно
    if (firstTick > minAngle + 1) {
        int x = leftMargin;
        painter.drawLine(x, image.height() - bottomMargin, x, image.height() - bottomMargin + 6);
        painter.drawText(x - 12, image.height() - bottomMargin + 22, QString::number(minAngle, 'f', 0));
    }

    if (lastTick < maxAngle - 1) {
        int x = leftMargin + graphWidth;
        painter.drawLine(x, image.height() - bottomMargin, x, image.height() - bottomMargin + 6);
        painter.drawText(x - 12, image.height() - bottomMargin + 22, QString::number(maxAngle, 'f', 0));
    }

    // Рисуем метки на оси Y
    int numYTicks = 5;
    for (int i = 0; i <= numYTicks; ++i) {
        double intensity = maxIntensity * i / numYTicks;
        int y = image.height() - bottomMargin - graphHeight * i / numYTicks;
        painter.drawLine(leftMargin - 6, y, leftMargin, y);

        QString label;
        if (intensity >= 0.99 && intensity <= 1.01) {
            label = "1.0";
        } else if (intensity < 0.01) {
            label = "0";
        } else {
            label = QString::number(intensity, 'f', 2);
        }

        painter.drawText(leftMargin - 45, y + 4, label);
    }

    // Рисуем график интенсивности
    QPen graphPen(QColor(52, 152, 219), 2);
    painter.setPen(graphPen);

    double prevX = leftMargin + (data[0].first - minAngle) / (maxAngle - minAngle) * graphWidth;
    double prevY = image.height() - bottomMargin - (data[0].second / maxIntensity) * graphHeight;

    for (int i = 1; i < data.size(); ++i) {
        double x = leftMargin + (data[i].first - minAngle) / (maxAngle - minAngle) * graphWidth;
        double y = image.height() - bottomMargin - (data[i].second / maxIntensity) * graphHeight;

        painter.drawLine(prevX, prevY, x, y);
        prevX = x;
        prevY = y;
    }

    // Заливаем область под графиком
    QPainterPath path;
    path.moveTo(leftMargin + (data[0].first - minAngle) / (maxAngle - minAngle) * graphWidth,
                image.height() - bottomMargin);

    for (int i = 0; i < data.size(); ++i) {
        double x = leftMargin + (data[i].first - minAngle) / (maxAngle - minAngle) * graphWidth;
        double y = image.height() - bottomMargin - (data[i].second / maxIntensity) * graphHeight;
        path.lineTo(x, y);
    }

    path.lineTo(prevX, image.height() - bottomMargin);
    path.closeSubpath();

    painter.fillPath(path, QColor(52, 152, 219, 50));

    // Информация о параметрах (только λ и a)
    painter.setPen(QPen(QColor(44, 62, 80), 2));
    painter.setFont(QFont("Arial", 11, QFont::Bold));

    // Белый прямоугольник под текстом
    QRect infoRect(leftMargin + 10, topMargin - 45, 250, 35);
    painter.fillRect(infoRect, QColor(255, 255, 255, 220));
    painter.drawRect(infoRect);

    painter.setPen(QPen(QColor(52, 152, 219), 2));
    QString lambdaText = QString("λ = %1 нм").arg(lambda, 0, 'f', 1);
    QString wideText = QString("a = %1 мкм").arg(wide, 0, 'f', 1);

    painter.drawText(leftMargin + 20, topMargin - 20, lambdaText);
    painter.drawText(leftMargin + 150, topMargin - 20, wideText);

    // Горизонтальная линия на уровне 0.5
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    if (maxIntensity > 0) {
        int halfMaxY = image.height() - bottomMargin - (0.5 / maxIntensity) * graphHeight;
        painter.drawLine(leftMargin, halfMaxY, image.width() - rightMargin, halfMaxY);
    }

    // Отображаем изображение
    QPixmap pixmap = QPixmap::fromImage(image);
    setPixmap(pixmap);
    setScaledContents(true);
}

// ========== РЕАЛИЗАЦИЯ CustomTitleBar ==========

CustomTitleBar::CustomTitleBar(QWidget *parent) : QWidget(parent), dragging(false) {
    setFixedHeight(40);

    closeButton = new QPushButton("✕", this);
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 15px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
        QPushButton:pressed {
            background-color: #a93226;
        }
    )");

    connect(closeButton, &QPushButton::clicked, this, &CustomTitleBar::closeClicked);

    setStyleSheet(R"(
        CustomTitleBar {
            background-color: #2c3e50;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
        }
    )");
}

void CustomTitleBar::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(44, 62, 80));

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(15, 0, width() - 100, height()), Qt::AlignVCenter,
                     "Дифракция Фраунгофера на щели - расчет интенсивности");

    closeButton->move(width() - closeButton->width() - 10, (height() - closeButton->height()) / 2);

    QWidget::paintEvent(event);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = true;
        dragPosition = event->globalPos() - parentWidget()->frameGeometry().topLeft();
        event->accept();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        parentWidget()->move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
        event->accept();
    }
}

// ========== РЕАЛИЗАЦИЯ MainWindow ==========

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), lambda(0.0), wide(0.0)
{
    // Создаем калькулятор
    calculator = new DiffractionCalculator(this);

    // Убираем стандартный заголовок окна
    setWindowFlags(Qt::FramelessWindowHint);

    // Создаем главный контейнер
    mainContainerLayout = new QVBoxLayout();
    mainContainerLayout->setContentsMargins(0, 0, 0, 0);
    mainContainerLayout->setSpacing(0);

    // Заголовок
    titleBar = new CustomTitleBar();
    mainContainerLayout->addWidget(titleBar);
    connect(titleBar, &CustomTitleBar::closeClicked, this, &MainWindow::closeWindow);

    // Центральный виджет
    centralWidget = new QWidget();
    centralWidget->setStyleSheet("background-color: #2c3e50;");

    // Горизонтальный layout
    QHBoxLayout *contentLayout = new QHBoxLayout(centralWidget);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(15);

    // ========== ЛЕВАЯ ПАНЕЛЬ ==========
    QWidget *leftPanel = new QWidget();
    leftPanel->setObjectName("leftPanel");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);

    // ===== БЛОК ДЛЯ ДЛИНЫ ВОЛНЫ (LAMBDA) =====
    QLabel *lambdaLabel = new QLabel("Длина волны (Lambda) в нанометрах (нм):");
    lambdaLabel->setStyleSheet("color: #2c3e50; font-weight: bold;");
    lambdaLabel->setAlignment(Qt::AlignCenter);

    lambdaInputField = new QLineEdit();
    lambdaInputField->setPlaceholderText("Введите значение в нм");
    lambdaInputField->setMinimumHeight(40);
    lambdaInputField->setStyleSheet("QLineEdit { background-color: white; border: 2px solid #bdc3c7; border-radius: 6px; padding: 10px; font-size: 14px; }");

    lambdaButton = new QPushButton("Сохранить Lambda");
    lambdaButton->setMinimumHeight(40);
    lambdaButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 6px; padding: 10px; font-size: 14px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #2980b9; }"
                                "QPushButton:pressed { background-color: #1c5980; }");

    QLabel *currentLambdaLabel = new QLabel("Текущее значение Lambda:");
    currentLambdaLabel->setStyleSheet("color: #2c3e50; font-weight: bold;");

    lambdaValueDisplay = new QLabel("0.0 нм");
    lambdaValueDisplay->setStyleSheet("color: #3498db; font-size: 16px; font-weight: bold;");
    lambdaValueDisplay->setAlignment(Qt::AlignCenter);

    // ===== БЛОК ДЛЯ ШИРИНЫ ЩЕЛИ (WIDE) =====
    QLabel *wideLabel = new QLabel("Ширина щели (Wide) в микрометрах (мкм):");
    wideLabel->setStyleSheet("color: #2c3e50; font-weight: bold; margin-top: 20px;");
    wideLabel->setAlignment(Qt::AlignCenter);

    wideInputField = new QLineEdit();
    wideInputField->setPlaceholderText("Введите значение в мкм");
    wideInputField->setMinimumHeight(40);
    wideInputField->setStyleSheet("QLineEdit { background-color: white; border: 2px solid #bdc3c7; border-radius: 6px; padding: 10px; font-size: 14px; }");

    wideButton = new QPushButton("Сохранить Wide");
    wideButton->setMinimumHeight(40);
    wideButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 6px; padding: 10px; font-size: 14px; font-weight: bold; }"
                              "QPushButton:hover { background-color: #2980b9; }"
                              "QPushButton:pressed { background-color: #1c5980; }");

    QLabel *currentWideLabel = new QLabel("Текущее значение Wide:");
    currentWideLabel->setStyleSheet("color: #2c3e50; font-weight: bold;");

    wideValueDisplay = new QLabel("0.0 мкм");
    wideValueDisplay->setStyleSheet("color: #3498db; font-size: 16px; font-weight: bold;");
    wideValueDisplay->setAlignment(Qt::AlignCenter);

    // ===== КНОПКА РАСЧЕТА =====
    calculateButton = new QPushButton("Рассчитать интенсивность");
    calculateButton->setMinimumHeight(45);
    calculateButton->setStyleSheet("QPushButton { background-color: #e67e22; color: white; border: none; border-radius: 6px; padding: 10px; font-size: 15px; font-weight: bold; }"
                                   "QPushButton:hover { background-color: #d35400; }"
                                   "QPushButton:pressed { background-color: #a04000; }");

    // ===== ВЫБОР МЕТОДА РАСЧЕТА =====
    QLabel *methodLabel = new QLabel("Метод расчета:");
    methodLabel->setStyleSheet("color: #2c3e50; font-weight: bold; margin-top: 10px;");
    methodLabel->setAlignment(Qt::AlignCenter);

    methodComboBox = new QComboBox();  // <--- ЭТА СТРОКА ВАЖНА!
    methodComboBox->addItem("Аналитический (sinc)");
    methodComboBox->addItem("Численный (FT)");
    methodComboBox->setMinimumHeight(35);
    methodComboBox->setStyleSheet("QComboBox { background-color: white; border: 2px solid #bdc3c7; border-radius: 6px; padding: 5px; font-size: 14px; }");

    // Добавляем в левую панель
    leftLayout->addWidget(methodLabel);
    leftLayout->addWidget(methodComboBox);

    // Добавляем все виджеты в левую панель
    leftLayout->addWidget(lambdaLabel);
    leftLayout->addWidget(lambdaInputField);
    leftLayout->addWidget(lambdaButton);
    leftLayout->addWidget(currentLambdaLabel);
    leftLayout->addWidget(lambdaValueDisplay);

    leftLayout->addWidget(wideLabel);
    leftLayout->addWidget(wideInputField);
    leftLayout->addWidget(wideButton);
    leftLayout->addWidget(currentWideLabel);
    leftLayout->addWidget(wideValueDisplay);

    leftLayout->addStretch();
    leftLayout->addWidget(calculateButton);

    // ========== ПРАВАЯ ПАНЕЛЬ (область для графика) ==========
    QScrollArea *scrollArea = new QScrollArea();
    emptyArea = new EmptyDisplayArea();
    scrollArea->setWidget(emptyArea);
    scrollArea->setWidgetResizable(false);
    scrollArea->setFixedSize(920, 620);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2c3e50; }");

    // Настройка размеров
    leftPanel->setMaximumWidth(400);
    leftPanel->setMinimumWidth(300);

    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(scrollArea);
    contentLayout->addStretch();

    mainContainerLayout->addWidget(centralWidget, 1);

    QWidget *container = new QWidget();
    container->setLayout(mainContainerLayout);
    setCentralWidget(container);

    // ========== ПОДКЛЮЧАЕМ СИГНАЛЫ ==========
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateAndDraw);

    // Лямбда для сохранения Lambda
    auto saveLambda = [this]() {
        QString text = lambdaInputField->text();
        bool ok;
        double value = text.toDouble(&ok);
        if (ok && value > 0) {
            this->lambda = value;
            calculator->setLambda(this->lambda);
            lambdaValueDisplay->setText(QString::number(this->lambda, 'f', 1) + " нм");
            qDebug() << "Lambda сохранена:" << this->lambda << "нм";
        } else {
            lambdaValueDisplay->setText("Ошибка! Введите число");
            qDebug() << "Ошибка ввода Lambda:" << text;
        }
    };

    // Лямбда для сохранения Wide
    auto saveWide = [this]() {
        QString text = wideInputField->text();
        bool ok;
        double value = text.toDouble(&ok);
        if (ok && value > 0) {
            this->wide = value;
            calculator->setWide(this->wide);
            wideValueDisplay->setText(QString::number(this->wide, 'f', 1) + " мкм");
            qDebug() << "Wide сохранена:" << this->wide << "мкм";
        } else {
            wideValueDisplay->setText("Ошибка! Введите число");
            qDebug() << "Ошибка ввода Wide:" << text;
        }
    };

    connect(lambdaButton, &QPushButton::clicked, this, saveLambda);
    connect(wideButton, &QPushButton::clicked, this, saveWide);
    connect(lambdaInputField, &QLineEdit::returnPressed, this, saveLambda);
    connect(wideInputField, &QLineEdit::returnPressed, this, saveWide);

    // ========== СТИЛИ ==========
    setStyleSheet(R"(
        QMainWindow {
            background-color: #2c3e50;
        }
        #leftPanel {
            background-color: #ecf0f1;
            border-radius: 10px;
            padding: 10px;
        }
    )");

    showFullScreen();
}

void MainWindow::calculateAndDraw()
{
    qDebug() << "=== РАСЧЕТ ДИФРАКЦИИ ===";
    qDebug() << "lambda =" << lambda << "нм, wide =" << wide << "мкм";

    if (lambda <= 0 || wide <= 0) {
        qDebug() << "ОШИБКА: параметры не установлены";
        emptyArea->QLabel::setText("Ошибка: сначала установите Lambda (>0) и Wide (>0)");
        return;
    }

    // Передаем параметры в калькулятор
    calculator->setLambda(lambda);
    calculator->setWide(wide);

    // Настраиваем диапазон для численного метода
    double wm = wide * 1e-6;
    calculator->setRange(-wm * 20, wm * 20, 2000);

    QVector<QPair<double, double>> res;

    // Теперь methodComboBox существует, можно безопасно использовать
    if (methodComboBox->currentIndex() == 0) {
        qDebug() << "Используем аналитический метод";
        res = calculator->I_range_analytical(-30.0, 30.0, 601);
    } else {
        qDebug() << "Используем численный метод";
        res = calculator->I_range_numerical(-30.0, 30.0, 601);
    }

    if (res.isEmpty()) {
        qDebug() << "ОШИБКА: результат пустой!";
        emptyArea->QLabel::setText("Ошибка при расчете");
        return;
    }

    qDebug() << "Получено" << res.size() << "точек, отрисовываем график";
    emptyArea->drawGraph(res, lambda, wide);
}

void MainWindow::closeWindow() {
    close();
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.addAction("Закрыть", this, &QWidget::close);
    menu.addAction("Выход", qApp, &QApplication::quit);
    menu.exec(event->globalPos());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (titleBar) {
        titleBar->update();
    }
}