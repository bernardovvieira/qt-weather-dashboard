#ifndef FORECASTDATA_H
#define FORECASTDATA_H

#include <QString>
#include <QDateTime>
#include <QList>

class ForecastItem
{
public:
    ForecastItem();

    QDateTime dateTime() const { return m_dateTime; }
    double tempMin() const { return m_tempMin; }
    double tempMax() const { return m_tempMax; }
    QString description() const { return m_description; }
    QString iconCode() const { return m_iconCode; }

    void setDateTime(const QDateTime &dt) { m_dateTime = dt; }
    void setTempMin(double temp) { m_tempMin = temp; }
    void setTempMax(double temp) { m_tempMax = temp; }
    void setDescription(const QString &desc) { m_description = desc; }
    void setIconCode(const QString &code) { m_iconCode = code; }

private:
    QDateTime m_dateTime;
    double m_tempMin;
    double m_tempMax;
    QString m_description;
    QString m_iconCode;
};

class ForecastData
{
public:
    ForecastData();

    void addItem(const ForecastItem &item) { m_items.append(item); }
    QList<ForecastItem> items() const { return m_items; }
    int count() const { return m_items.count(); }

    void clear() { m_items.clear(); }

private:
    QList<ForecastItem> m_items;
};

#endif // FORECASTDATA_H
