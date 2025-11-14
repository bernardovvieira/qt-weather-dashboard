#include "weatherservice.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

    QUrl url(BASE_URL);
    QUrlQuery query;
    query.addQueryItem("q", city);
    query.addQueryItem("appid", API_KEY);
    query.addQueryItem("units", "metric"); // Celsius
    query.addQueryItem("lang", "en");
    url.setQuery(query);

    QNetworkRequest request(url);
    m_networkManager->get(request);
}

void WeatherService::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        // Parse JSON response
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON response from API");
            reply->deleteLater();
            return;
        }

        QJsonObject json = doc.object();

        // Verifica se a API retornou erro
        if (json.contains("cod") && json["cod"].toInt() != 200) {
            QString message = json["message"].toString();
            emit errorOccurred("API Error: " + message);
            reply->deleteLater();
            return;
        }

        WeatherData weatherData = parseWeatherData(data);

        if (weatherData.isValid()) {
            emit weatherDataReady(weatherData);
        } else {
            emit errorOccurred("Failed to parse weather data");
        }
    } else {
        // Erros de rede
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

    // Nome da cidade e pais
    data.setCityName(json["name"].toString());

    if (json.contains("sys") && json["sys"].isObject()) {
        QJsonObject sys = json["sys"].toObject();
        data.setCountry(sys["country"].toString());
    }

    // Dados principais (temperatura, umidade, sensacao termica)
    if (json.contains("main") && json["main"].isObject()) {
        QJsonObject main = json["main"].toObject();
        data.setTemperature(main["temp"].toDouble());
        data.setFeelsLike(main["feels_like"].toDouble());
        data.setHumidity(main["humidity"].toInt());
    }

    // Vento
    if (json.contains("wind") && json["wind"].isObject()) {
        QJsonObject wind = json["wind"].toObject();
        data.setWindSpeed(wind["speed"].toDouble());
    }

    // Condicoes climaticas (weather array)
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
