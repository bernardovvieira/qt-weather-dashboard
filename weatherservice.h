#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "weatherdata.h"

class WeatherService : public QObject
{
    Q_OBJECT

public:
    explicit WeatherService(QObject *parent = nullptr);
    void fetchWeather(const QString &city);

signals:
    void weatherDataReady(const WeatherData &data);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    WeatherData parseWeatherData(const QByteArray &jsonData);

    const QString API_KEY = "9df9796620be86e3680b4b40f7cf3098";
    const QString BASE_URL = "https://api.openweathermap.org/data/2.5/weather";
};

#endif // WEATHERSERVICE_H
