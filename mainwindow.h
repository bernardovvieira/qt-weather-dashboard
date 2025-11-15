#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include "weatherservice.h"
#include "locationmanager.h"
#include "weatherdata.h"
#include "forecastdata.h"
#include "citysearchwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCitySelected(const QString &cityName, double lat, double lon);
    void onWeatherDataReady(const WeatherData &data);
    void onForecastDataReady(const ForecastData &data);
    void onWeatherError(const QString &error);
    void onAddFavoritesClicked();
    void onLoadFavoriteClicked();
    void onRemoveFavoriteClicked();
    void onFavoritesChanged();
    void onIconDownloaded(QNetworkReply *reply);
    void onClearClicked();
    void onAboutClicked();
    void onLoadFirstFavoriteClicked();
    void onFavoritesReordered();

private:
    Ui::MainWindow *ui;
    WeatherService *m_weatherService;
    LocationManager *m_locationManager;
    QNetworkAccessManager *m_iconManager;
    CitySearchWidget *m_citySearchWidget;
    WeatherData m_currentWeather;
    QString m_currentCity;
    QString m_currentCityFull;
    QMap<QString, QPixmap> m_forecastIcons;

    void updateWeatherDisplay(const WeatherData &data);
    void updateForecastDisplay(const ForecastData &data);
    void updateFavoritesList();
    void setStatusMessage(const QString &message);
    void downloadWeatherIcon(const QString &iconCode);
    void downloadForecastIcon(const QString &iconCode, int row);
    void clearResults();
    void loadFirstFavorite();  // ADICIONAR
};

#endif // MAINWINDOW_H
