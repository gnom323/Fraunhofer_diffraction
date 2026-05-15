#ifndef DIFFRACTIONCALCULATOR_H
#define DIFFRACTIONCALCULATOR_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <complex>

class DiffractionCalculator : public QObject
{
    Q_OBJECT

public:
    explicit DiffractionCalculator(QObject *parent = nullptr);

    // Установка параметров
    void setLambda(double l);           // длина волны (нм)
    void setWide(double w);             // ширина щели (мкм)
    void setPeriod(double p);           // период решетки (мкм)
    void setSlitsCount(int n);          // количество щелей

    // Аналитический метод (только для одной щели)
    double I_analytical(double ang);
    QVector<QPair<double, double>> I_range_analytical(double start, double end, int pts);

    // Численный метод (для любого количества щелей)
    double I_numerical(double ang);
    QVector<QPair<double, double>> I_range_numerical(double start, double end, int pts);

    // Геттеры
    double getLambda() const { return m_l; }
    double getWide() const { return m_w; }
    double getPeriod() const { return m_d; }
    int getSlitsCount() const { return m_N; }
    double getK() const { return m_k; }

    // Функция пропускания (для решетки)
    virtual double f(double x);

    // Преобразование Фурье (численное)
    std::complex<double> F(double sinTheta);

    // Настройка численного интегрирования
    void setRange(double xMin, double xMax, int pts);

private:
    double m_l;      // длина волны (нм)
    double m_w;      // ширина щели (мкм)
    double m_d;      // период решетки (мкм)
    int m_N;         // количество щелей
    double m_k;      // волновой вектор

    double m_xMin;   // мин. координата для интегрирования
    double m_xMax;   // макс. координата для интегрирования
    int m_pts;       // количество точек интегрирования

    // Вспомогательные функции
    double to_sin(double ang);
    double norm(double I, double Imax);
    double from_amp(const std::complex<double>& a);
};

#endif // DIFFRACTIONCALCULATOR_H