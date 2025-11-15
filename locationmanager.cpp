#include "locationmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

LocationManager::LocationManager(QObject *parent)
    : QObject(parent)
{
    loadFavorites();
}

void LocationManager::addLocation(const QString &city)
{
    QString normalizedCity = city.trimmed();

    if (normalizedCity.isEmpty()) {
        qWarning() << "Cannot add empty city name";
        return;
    }

    if (m_favorites.contains(normalizedCity, Qt::CaseInsensitive)) {
        qWarning() << "City already in favorites:" << normalizedCity;
        return;
    }

    m_favorites.append(normalizedCity);
    saveFavorites();
    emit locationAdded(normalizedCity);
    emit favoritesChanged();
}

void LocationManager::removeLocation(const QString &city)
{
    QString normalizedCity = city.trimmed();

    // Remove case-insensitive
    for (int i = 0; i < m_favorites.size(); ++i) {
        if (m_favorites[i].compare(normalizedCity, Qt::CaseInsensitive) == 0) {
            m_favorites.removeAt(i);
            saveFavorites();
            emit locationRemoved(normalizedCity);
            emit favoritesChanged();
            return;
        }
    }

    qWarning() << "City not found in favorites:" << normalizedCity;
}

void LocationManager::reorderLocations(const QStringList &newOrder)
{
    m_favorites = newOrder;
    saveFavorites();
    emit favoritesChanged();
}

bool LocationManager::isFavorite(const QString &city) const
{
    return m_favorites.contains(city.trimmed(), Qt::CaseInsensitive);
}

QStringList LocationManager::getFavorites() const
{
    return m_favorites;
}

QString LocationManager::getFavoritesFilePath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create data directory:" << dataPath;
        }
    }

    return dataPath + "/favorites.json";
}

void LocationManager::saveFavorites()
{
    QJsonArray jsonArray;
    for (const QString &city : m_favorites) {
        jsonArray.append(city);
    }

    QJsonDocument doc(jsonArray);
    QString filePath = getFavoritesFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        qWarning() << "Error:" << file.errorString();
        return;
    }

    qint64 bytesWritten = file.write(doc.toJson());
    file.close();

    if (bytesWritten == -1) {
        qWarning() << "Failed to write favorites to file";
    } else {
        qDebug() << "Favorites saved successfully:" << m_favorites.count() << "cities";
    }
}

void LocationManager::loadFavorites()
{
    QString filePath = getFavoritesFilePath();
    QFile file(filePath);

    if (!file.exists()) {
        qDebug() << "Favorites file does not exist yet:" << filePath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open favorites file:" << filePath;
        qWarning() << "Error:" << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "Invalid JSON in favorites file";
        return;
    }

    QJsonArray jsonArray = doc.array();
    m_favorites.clear();

    for (const QJsonValue &value : jsonArray) {
        if (value.isString()) {
            m_favorites.append(value.toString());
        }
    }

    qDebug() << "Favorites loaded successfully:" << m_favorites.count() << "cities";
}
