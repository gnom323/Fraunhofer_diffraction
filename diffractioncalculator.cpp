#include "diffractioncalculator.h"
#include <cmath>
#include <QDebug>
#include <complex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DiffractionCalculator::DiffractionCalculator(QObject *parent)
    : QObject(parent), m_l(0.0), m_w(0.0), m_d(0.0), m_N(1), m_k(0.0),
    m_xMin(-1e-3), m_xMax(1e-3), m_pts(2000)
{
}

void DiffractionCalculator::setLambda(double l)
{
    m_l = l;
    double lm = m_l * 1e-9;
    m_k = (lm > 0) ? (2.0 * M_PI / lm) : 0.0;
    qDebug() << "Lambda:" << m_l << "нм, k:" << m_k;
}

void DiffractionCalculator::setWide(double w)
{
    m_w = w;
    qDebug() << "Wide:" << m_w << "мкм";
}

void DiffractionCalculator::setPeriod(double p)
{
    m_d = p;
    qDebug() << "Period:" << m_d << "мкм";
}

void DiffractionCalculator::setSlitsCount(int n)
{
    m_N = n;
    qDebug() << "Number of slits:" << m_N;
}

void DiffractionCalculator::setRange(double xMin, double xMax, int pts)
{
    m_xMin = xMin;
    m_xMax = xMax;
    m_pts = pts;
    qDebug() << "Численное интегрирование: x от" << xMin << "до" << xMax << "м, точек:" << pts;
}

double DiffractionCalculator::f(double x)
{
    if (m_N <= 0 || m_w <= 0 || m_d <= 0) return 0.0;

    double wm = m_w * 1e-6;      // ширина щели в метрах
    double dm = m_d * 1e-6;      // период в метрах
    double hw = wm / 2.0;        // половина ширины щели

    // Начало решетки: центрируем относительно 0
    double start = - (m_N - 1) * dm / 2.0;

    for (int i = 0; i < m_N; ++i) {
        double slitCenter = start + i * dm;
        double left = slitCenter - hw;
        double right = slitCenter + hw;

        if (x >= left && x <= right) {
            return 1.0;  // внутри щели
        }
    }
    return 0.0;  // вне щелей
}

std::complex<double> DiffractionCalculator::F(double sinTheta)
{
    if (m_k == 0.0 || m_l <= 0) return std::complex<double>(0.0, 0.0);

    std::complex<double> I(0.0, 0.0);
    double dx = (m_xMax - m_xMin) / m_pts;

    for (int i = 0; i <= m_pts; ++i) {
        double x = m_xMin + i * dx;
        double val = f(x);

        if (val != 0.0) {
            double phase = -m_k * x * sinTheta;
            I += val * std::complex<double>(cos(phase), sin(phase)) * dx;
        }
    }
    return I;
}

double DiffractionCalculator::to_sin(double ang)
{
    return sin(ang * M_PI / 180.0);
}

double DiffractionCalculator::norm(double I, double Imax)
{
    return (Imax <= 0) ? I : I / Imax;
}

double DiffractionCalculator::from_amp(const std::complex<double>& a)
{
    return std::norm(a);
}

double DiffractionCalculator::I_analytical(double ang)
{
    // Аналитический метод работает только для одной щели
    if (m_N != 1) return 0.0;

    if (m_l <= 0 || m_w <= 0) return 0.0;

    double st = to_sin(ang);
    double wm = m_w * 1e-6;
    double lm = m_l * 1e-9;
    double k = 2.0 * M_PI / lm;
    double arg = k * wm * st / 2.0;

    double A = (fabs(arg) < 1e-10) ? wm : wm * sin(arg) / arg;
    return A * A;
}

double DiffractionCalculator::I_numerical(double ang)
{
    if (m_l <= 0 || m_w <= 0 || m_N <= 0) return 0.0;
    return from_amp(F(to_sin(ang)));
}

QVector<QPair<double, double>> DiffractionCalculator::I_range_analytical(double start, double end, int pts)
{
    QVector<QPair<double, double>> res;

    if (pts <= 1 || m_l <= 0 || m_w <= 0 || m_N != 1) return res;

    double step = (end - start) / (pts - 1);
    double Imax = 0.0;
    QVector<double> Ivals;

    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        double I = I_analytical(ang);
        Ivals.append(I);
        if (I > Imax) Imax = I;
    }

    if (Imax <= 0) Imax = 1.0;

    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        res.append(qMakePair(ang, Ivals[i] / Imax));
    }

    return res;
}

QVector<QPair<double, double>> DiffractionCalculator::I_range_numerical(double start, double end, int pts)
{
    QVector<QPair<double, double>> res;

    if (pts <= 1 || m_l <= 0 || m_w <= 0 || m_N <= 0) return res;

    double step = (end - start) / (pts - 1);
    double Imax = 0.0;
    QVector<double> Ivals;

    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        double I = I_numerical(ang);
        Ivals.append(I);
        if (I > Imax) Imax = I;
    }

    if (Imax <= 0) Imax = 1.0;

    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        res.append(qMakePair(ang, Ivals[i] / Imax));
    }

    qDebug() << "Численный расчет для" << m_N << "щелей, период" << m_d << "мкм";
    return res;
}