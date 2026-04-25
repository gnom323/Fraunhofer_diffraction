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

    void setLambda(double l);      // длина волны в нанометрах
    void setWide(double w);        // ширина щели в микрометрах

    double I_analytical(double ang);// аналитический метод
    QVector<QPair<double, double>> I_range_analytical(double start, double end, int pts);

    double I_numerical(double ang); // численный метод
    QVector<QPair<double, double>> I_range_numerical(double start, double end, int pts);

    double getLambda() const { return m_l; }
    double getWide() const { return m_w; }
    double getK() const { return m_k; }

    virtual double f(double x);     // функция пропускания

    std::complex<double> F(double sinTheta);  // преобразование Фурье

    void setRange(double xMin, double xMax, int pts);

private:
    double m_l;          // lambda
    double m_w;          // wide
    double m_k;          // волновой вектор

    double m_xMin;
    double m_xMax;
    int m_pts;           // количество точек

    double to_sin(double ang);
    double norm(double I, double Imax);
    double from_amp(const std::complex<double>& a);
};

#endif