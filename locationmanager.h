#ifndef LOCATIONMANAGER_H
#define LOCATIONMANAGER_H

#include <QObject>
#include <QStringList>

class LocationManager : public QObject
{
    Q_OBJECT

public:
    explicit LocationManager(QObject *parent = nullptr);

    // Gerenciamento de favoritos
    void addLocation(const QString &city);
    void removeLocation(const QString &city);
    bool isFavorite(const QString &city) const;
    QStringList getFavorites() const;
    int count() const { return m_favorites.count(); }
    void reorderLocations(const QStringList &newOrder);

    // Persistencia
    void saveFavorites();
    void loadFavorites();

signals:
    void favoritesChanged();
    void locationAdded(const QString &city);
    void locationRemoved(const QString &city);

private:
    QStringList m_favorites;
    QString getFavoritesFilePath() const;
};

#endif // LOCATIONMANAGER_H
