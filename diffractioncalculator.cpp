#include "diffractioncalculator.h"
#include <cmath>
#include <QDebug>
#include <complex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DiffractionCalculator::DiffractionCalculator(QObject *parent)
    : QObject(parent), m_l(0.0), m_w(0.0), m_k(0.0), m_xMin(-1e-3), m_xMax(1e-3), m_pts(1000)
{
}

void DiffractionCalculator::setLambda(double l)
{
    m_l = l;
    // l уже в нанометрах, переводим в метры
    double lm = m_l * 1e-9;
    m_k = (lm > 0) ? (2.0 * M_PI / lm) : 0.0;
    qDebug() << "Lambda:" << m_l << "нм =" << lm << "м, k:" << m_k;
}

void DiffractionCalculator::setWide(double w)
{
    m_w = w;
    // w уже в микрометрах, переводим в метры
    double wm = m_w * 1e-6;
    qDebug() << "Wide:" << m_w << "мкм =" << wm << "м";
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
    double wm = m_w * 1e-6;  // ширина щели в метрах
    double hw = wm / 2.0;
    return (fabs(x) <= hw) ? 1.0 : 0.0;
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
    if (m_l <= 0 || m_w <= 0) {
        qDebug() << "I_analytical: параметры не установлены!" << m_l << m_w;
        return 0.0;
    }

    double st = to_sin(ang);
    double wm = m_w * 1e-6;      // ширина в метрах
    double lm = m_l * 1e-9;      // длина волны в метрах
    double k = 2.0 * M_PI / lm;  // волновое число

    // Аргумент для sinc: (π * a * sinθ) / λ = (k * a * sinθ) / 2
    double arg = k * wm * st / 2.0;

    double A;
    if (fabs(arg) < 1e-10) {
        A = wm;  // sinc(0) = 1, амплитуда = a
    } else {
        A = wm * sin(arg) / arg;
    }

    double I = A * A;

    // Для отладки - выводим только для первых нескольких углов
    static int callCount = 0;
    if (callCount < 5) {
        qDebug() << "I_analytical:" << ang << "град, st =" << st << ", arg =" << arg << ", I =" << I;
        callCount++;
    }

    return I;
}

double DiffractionCalculator::I_numerical(double ang)
{
    if (m_l <= 0 || m_w <= 0) return 0.0;
    return from_amp(F(to_sin(ang)));
}

QVector<QPair<double, double>> DiffractionCalculator::I_range_analytical(double start, double end, int pts)
{
    QVector<QPair<double, double>> res;

    qDebug() << "I_range_analytical: start=" << start << "end=" << end << "pts=" << pts;

    if (pts <= 1 || m_l <= 0 || m_w <= 0) {
        qDebug() << "Ошибка: неверные параметры!";
        return res;
    }

    double step = (end - start) / (pts - 1);
    double Imax = 0.0;
    QVector<double> Ivals;

    // Сначала вычисляем все интенсивности и находим максимум
    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        double I = I_analytical(ang);
        Ivals.append(I);
        if (I > Imax) Imax = I;
    }

    qDebug() << "Imax =" << Imax;

    if (Imax <= 0) {
        qDebug() << "Предупреждение: Imax <= 0, устанавливаю Imax = 1";
        Imax = 1.0;
    }

    // Нормируем и заполняем результат
    for (int i = 0; i < pts; ++i) {
        double ang = start + i * step;
        double normI = Ivals[i] / Imax;
        res.append(qMakePair(ang, normI));
    }

    qDebug() << "Возвращено" << res.size() << "точек";
    if (res.size() > 0) {
        qDebug() << "Первая точка:" << res[0].first << "->" << res[0].second;
        qDebug() << "Центр:" << res[pts/2].first << "->" << res[pts/2].second;
    }

    return res;
}

QVector<QPair<double, double>> DiffractionCalculator::I_range_numerical(double start, double end, int pts)
{
    QVector<QPair<double, double>> res;

    if (pts <= 1 || m_l <= 0 || m_w <= 0) return res;

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

    qDebug() << "Численный расчет завершен";
    return res;
}