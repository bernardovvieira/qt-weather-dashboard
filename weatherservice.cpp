#include "weatherservice.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>

WeatherService::WeatherService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherService::onReplyFinished);
}

void WeatherService::fetchWeather(const QString &city)
{
    if (city.trimmed().isEmpty()) {
        emit errorOccurred("City name cannot be empty");
        return;
    }

    QUrl url(WEATHER_URL);
    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("appid", API_KEY);
    query.addQueryItem("units", "metric");
    query.addQueryItem("lang", "en");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, "weather");
    m_networkManager->get(request);
}

void WeatherService::fetchForecast(const QString &city)
{
    if (city.trimmed().isEmpty()) {
        emit errorOccurred("City name cannot be empty");
        return;
    }

    QUrl url(FORECAST_URL);
    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("appid", API_KEY);
    query.addQueryItem("units", "metric");
    query.addQueryItem("lang", "en");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, "forecast");
    m_networkManager->get(request);
}

void WeatherService::onReplyFinished(QNetworkReply *reply)
{
    QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON response from API");
            reply->deleteLater();
            return;
        }

        QJsonObject json = doc.object();

        // Handle both string and numeric cod values
        if (json.contains("cod")) {
            int codValue = 0;

            // API returns cod as number (200) for current weather
            if (json["cod"].isDouble()) {
                codValue = json["cod"].toInt();
            }
            // API returns cod as string ("200") for forecast
            else if (json["cod"].isString()) {
                codValue = json["cod"].toString().toInt();
            }

            if (codValue != 200) {
                QString message = json["message"].toString();
                if (message.isEmpty()) {
                    message = QString("API returned error code %1").arg(codValue);
                }
                emit errorOccurred("API Error: " + message);
                reply->deleteLater();
                return;
            }
        }

        if (requestType == "weather") {
            WeatherData weatherData = parseWeatherData(data);
            if (weatherData.isValid()) {
                emit weatherDataReady(weatherData);
            } else {
                emit errorOccurred("Failed to parse weather data");
            }
        } else if (requestType == "forecast") {
            ForecastData forecastData = parseForecastData(data);
            emit forecastDataReady(forecastData);
        }
    } else {
        QString errorMsg = reply->errorString();

        if (reply->error() == QNetworkReply::HostNotFoundError) {
            errorMsg = "No internet connection or server not found";
        } else if (reply->error() == QNetworkReply::TimeoutError) {
            errorMsg = "Request timeout. Please try again";
        }

        emit errorOccurred(errorMsg);
    }

    reply->deleteLater();
}

WeatherData WeatherService::parseWeatherData(const QByteArray &jsonData)
{
    WeatherData data;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject json = doc.object();

    // City name
    data.setCityName(json["name"].toString());

    // Country
    if (json.contains("sys") && json["sys"].isObject()) {
        QJsonObject sys = json["sys"].toObject();
        data.setCountry(sys["country"].toString());
    }

    // Temperature, feels like, humidity
    if (json.contains("main") && json["main"].isObject()) {
        QJsonObject main = json["main"].toObject();
        data.setTemperature(main["temp"].toDouble());
        data.setFeelsLike(main["feels_like"].toDouble());
        data.setHumidity(main["humidity"].toInt());
    }

    // Wind speed
    if (json.contains("wind") && json["wind"].isObject()) {
        QJsonObject wind = json["wind"].toObject();
        data.setWindSpeed(wind["speed"].toDouble());
    }

    // Weather description and icon
    if (json.contains("weather") && json["weather"].isArray()) {
        QJsonArray weatherArray = json["weather"].toArray();
        if (!weatherArray.isEmpty()) {
            QJsonObject weather = weatherArray[0].toObject();
            data.setDescription(weather["description"].toString());
            data.setIconCode(weather["icon"].toString());
        }
    }

    return data;
}

ForecastData WeatherService::parseForecastData(const QByteArray &jsonData)
{
    ForecastData data;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject json = doc.object();

    if (!json.contains("list") || !json["list"].isArray()) {
        return data;
    }

    QJsonArray list = json["list"].toArray();

    // Process daily data (one per day around noon)
    QSet<QString> processedDates;

    for (const QJsonValue &value : list) {
        if (!value.isObject()) continue;

        QJsonObject item = value.toObject();

        // Get date/time
        qint64 timestamp = item["dt"].toVariant().toLongLong();
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        QString dateKey = dateTime.date().toString("yyyy-MM-dd");

        // Only take one forecast per day (around noon)
        if (processedDates.contains(dateKey)) {
            continue;
        }

        int hour = dateTime.time().hour();
        if (hour < 11 || hour > 14) {
            continue;
        }

        processedDates.insert(dateKey);

        ForecastItem forecastItem;
        forecastItem.setDateTime(dateTime);

        // Temperature (min/max approximation)
        if (item.contains("main") && item["main"].isObject()) {
            QJsonObject main = item["main"].toObject();
            double temp = main["temp"].toDouble();
            double tempMin = main["temp_min"].toDouble();
            double tempMax = main["temp_max"].toDouble();

            // Use actual min/max if available, otherwise approximate
            if (tempMin > 0 && tempMax > 0) {
                forecastItem.setTempMin(tempMin);
                forecastItem.setTempMax(tempMax);
            } else {
                forecastItem.setTempMin(temp - 3);
                forecastItem.setTempMax(temp + 3);
            }
        }

        // Weather description and icon
        if (item.contains("weather") && item["weather"].isArray()) {
            QJsonArray weatherArray = item["weather"].toArray();
            if (!weatherArray.isEmpty()) {
                QJsonObject weather = weatherArray[0].toObject();
                forecastItem.setDescription(weather["description"].toString());
                forecastItem.setIconCode(weather["icon"].toString());
            }
        }

        data.addItem(forecastItem);

        // Limit to 5 days
        if (data.count() >= 5) {
            break;
        }
    }

    return data;
}
