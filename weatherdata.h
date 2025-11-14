#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QString>

class WeatherData
{
public:
    WeatherData();

    // Getters
    QString cityName() const { return m_cityName; }
    QString country() const { return m_country; }
    double temperature() const { return m_temperature; }
    double feelsLike() const { return m_feelsLike; }
    QString description() const { return m_description; }
    int humidity() const { return m_humidity; }
    double windSpeed() const { return m_windSpeed; }
    QString iconCode() const { return m_iconCode; }

    // Setters
    void setCityName(const QString &name) { m_cityName = name; }
    void setCountry(const QString &country) { m_country = country; }
    void setTemperature(double temp) { m_temperature = temp; }
    void setFeelsLike(double feels) { m_feelsLike = feels; }
    void setDescription(const QString &desc) { m_description = desc; }
    void setHumidity(int humidity) { m_humidity = humidity; }
    void setWindSpeed(double speed) { m_windSpeed = speed; }
    void setIconCode(const QString &code) { m_iconCode = code; }

    // Validação
    bool isValid() const { return !m_cityName.isEmpty(); }

private:
    QString m_cityName;
    QString m_country;
    double m_temperature;
    double m_feelsLike;
    QString m_description;
    int m_humidity;
    double m_windSpeed;
    QString m_iconCode;
};

#endif // WEATHERDATA_H
