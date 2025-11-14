#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QPixmap>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_weatherService(new WeatherService(this))
    , m_locationManager(new LocationManager(this))
    , m_iconManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    // Create and setup city search widget
    m_citySearchWidget = new CitySearchWidget(this);

    // Replace the cityLineEdit with CitySearchWidget
    QWidget *searchContainer = ui->cityLineEdit->parentWidget();
    QLayout *searchLayout = searchContainer->layout();

    if (searchLayout) {
        searchLayout->replaceWidget(ui->cityLineEdit, m_citySearchWidget);
        ui->cityLineEdit->deleteLater();
    }

    // Connect weather service signals
    connect(m_weatherService, &WeatherService::weatherDataReady,
            this, &MainWindow::onWeatherDataReady);
    connect(m_weatherService, &WeatherService::forecastDataReady,
            this, &MainWindow::onForecastDataReady);
    connect(m_weatherService, &WeatherService::errorOccurred,
            this, &MainWindow::onWeatherError);

    // Connect city search widget
    connect(m_citySearchWidget, &CitySearchWidget::citySelected,
            this, &MainWindow::onCitySelected);

    // Connect UI buttons
    connect(ui->addFavoritesPushButton, &QPushButton::clicked,
            this, &MainWindow::onAddFavoritesClicked);
    connect(ui->loadFavoritePushButton, &QPushButton::clicked,
            this, &MainWindow::onLoadFavoriteClicked);
    connect(ui->removeFavoritePushButton, &QPushButton::clicked,
            this, &MainWindow::onRemoveFavoriteClicked);
    connect(ui->clearPushButton, &QPushButton::clicked,
            this, &MainWindow::onClearClicked);

    // Connect location manager
    connect(m_locationManager, &LocationManager::favoritesChanged,
            this, &MainWindow::onFavoritesChanged);

    // Connect icon download manager
    connect(m_iconManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onIconDownloaded);

    // Configure forecast table
    ui->forecastTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->forecastTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->forecastTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->forecastTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Load favorites list
    updateFavoritesList();

    // Load first favorite city automatically
    loadFirstFavorite();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCitySelected(const QString &cityName, double lat, double lon)
{
    Q_UNUSED(lat);
    Q_UNUSED(lon);

    m_currentCity = cityName;
    setStatusMessage("Searching weather for " + cityName + "...");

    // Fetch both current weather and forecast
    m_weatherService->fetchWeather(cityName);
    m_weatherService->fetchForecast(cityName);
}

void MainWindow::onWeatherDataReady(const WeatherData &data)
{
    m_currentWeather = data;
    updateWeatherDisplay(data);
    ui->addFavoritesPushButton->setEnabled(true);

    setStatusMessage("Weather data loaded successfully");

    // Download weather icon
    if (!data.iconCode().isEmpty()) {
        downloadWeatherIcon(data.iconCode());
    }
}

void MainWindow::onForecastDataReady(const ForecastData &data)
{
    updateForecastDisplay(data);
}

void MainWindow::onWeatherError(const QString &error)
{
    QMessageBox::critical(this, "Weather Error",
                          "Failed to fetch weather data:\n" + error);
    setStatusMessage("Error: " + error);
}

void MainWindow::updateWeatherDisplay(const WeatherData &data)
{
    // City name and country
    ui->cityLabel->setText(QString("%1, %2")
                               .arg(data.cityName())
                               .arg(data.country()));

    // Temperature
    ui->temperatureLabel->setText(QString("%1°C")
                                      .arg(data.temperature(), 0, 'f', 1));

    // Description (capitalize first letter)
    QString description = data.description();
    if (!description.isEmpty()) {
        description[0] = description[0].toUpper();
    }
    ui->descriptionLabel->setText(description);

    // Feels like
    ui->feelsLikeLabel->setText(QString("%1°C")
                                    .arg(data.feelsLike(), 0, 'f', 1));

    // Humidity
    ui->humidityLabel->setText(QString("%1%")
                                   .arg(data.humidity()));

    // Wind speed
    ui->windLabel->setText(QString("%1 m/s")
                               .arg(data.windSpeed(), 0, 'f', 1));
}

void MainWindow::updateForecastDisplay(const ForecastData &data)
{
    // Clear table
    ui->forecastTableWidget->setRowCount(0);

    // Populate table with forecast data
    int row = 0;
    for (const ForecastItem &item : data.items()) {
        if (row >= 5) break; // Limit to 5 days

        ui->forecastTableWidget->insertRow(row);

        // Date
        QString dateStr = item.dateTime().toString("ddd, MMM dd");
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateStr);
        dateItem->setTextAlignment(Qt::AlignCenter);
        ui->forecastTableWidget->setItem(row, 0, dateItem);

        // Icon placeholder
        QTableWidgetItem *iconItem = new QTableWidgetItem("☁️");
        iconItem->setTextAlignment(Qt::AlignCenter);
        iconItem->setData(Qt::UserRole, item.iconCode());
        ui->forecastTableWidget->setItem(row, 1, iconItem);

        // Temperature range
        QString tempStr = QString("%1° / %2°")
                              .arg(qRound(item.tempMin()))
                              .arg(qRound(item.tempMax()));
        QTableWidgetItem *tempItem = new QTableWidgetItem(tempStr);
        tempItem->setTextAlignment(Qt::AlignCenter);
        ui->forecastTableWidget->setItem(row, 2, tempItem);

        // Description
        QString desc = item.description();
        if (!desc.isEmpty()) {
            desc[0] = desc[0].toUpper();
        }
        QTableWidgetItem *descItem = new QTableWidgetItem(desc);
        ui->forecastTableWidget->setItem(row, 3, descItem);

        row++;
    }

    // Adjust row heights
    for (int i = 0; i < ui->forecastTableWidget->rowCount(); ++i) {
        ui->forecastTableWidget->setRowHeight(i, 40);
    }
}

void MainWindow::downloadWeatherIcon(const QString &iconCode)
{
    if (iconCode.isEmpty()) {
        return;
    }

    QString iconUrl = QString("https://openweathermap.org/img/wn/%1@2x.png").arg(iconCode);

    QNetworkRequest request(iconUrl);
    request.setAttribute(QNetworkRequest::User, "current");
    m_iconManager->get(request);
}

void MainWindow::onIconDownloaded(QNetworkReply *reply)
{
    if (!reply) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();

        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

            if (requestType == "current") {
                // Escalar para 64x64 mantendo proporção
                QPixmap scaledPixmap = pixmap.scaled(64, 64,
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);

                ui->weatherIconLabel->setPixmap(scaledPixmap);
                ui->weatherIconLabel->setScaledContents(false);
            }
        }
    }

    reply->deleteLater();
}

void MainWindow::onAddFavoritesClicked()
{
    if (!m_currentWeather.isValid()) {
        QMessageBox::warning(this, "No Data",
                             "Please search for a city first");
        return;
    }

    // Get full city name from CitySearchWidget (includes state/country)
    CityResult selectedCity = m_citySearchWidget->selectedCity();
    QString fullCityName = selectedCity.fullName();

    // Fallback if no city was selected via autocomplete
    if (fullCityName.isEmpty()) {
        fullCityName = m_currentWeather.cityName() + ", " + m_currentWeather.country();
    }

    if (m_locationManager->isFavorite(fullCityName)) {
        QMessageBox::information(this, "Already Favorite",
                                 "This city is already in your favorites");
        return;
    }

    m_locationManager->addLocation(fullCityName);
    setStatusMessage(fullCityName + " added to favorites");
}

void MainWindow::onLoadFavoriteClicked()
{
    QListWidgetItem *item = ui->favoritesListWidget->currentItem();

    if (!item) {
        QMessageBox::warning(this, "No Selection",
                             "Please select a city from favorites");
        return;
    }

    QString fullCity = item->text();

    // Extract just the city name (before first comma)
    QString cityName = fullCity.split(",").first().trimmed();

    m_citySearchWidget->setText(cityName);
    m_currentCity = cityName;

    setStatusMessage("Searching weather for " + cityName + "...");
    m_weatherService->fetchWeather(cityName);
    m_weatherService->fetchForecast(cityName);
}

void MainWindow::onRemoveFavoriteClicked()
{
    QListWidgetItem *item = ui->favoritesListWidget->currentItem();

    if (!item) {
        QMessageBox::warning(this, "No Selection",
                             "Please select a city to remove");
        return;
    }

    QString city = item->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Removal",
                                  "Remove " + city + " from favorites?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_locationManager->removeLocation(city);
        setStatusMessage(city + " removed from favorites");
    }
}

void MainWindow::onClearClicked()
{
    // Limpar o campo de busca
    m_citySearchWidget->clear();

    // Limpar todos os resultados
    clearResults();

    // Reset current city
    m_currentCity.clear();
    m_currentWeather = WeatherData();

    setStatusMessage("Search cleared. Enter a city name to search.");
}

void MainWindow::clearResults()
{
    // Limpar clima atual
    ui->cityLabel->setText("--");
    ui->temperatureLabel->setText("--°C");
    ui->descriptionLabel->setText("--");
    ui->feelsLikeLabel->setText("--");
    ui->humidityLabel->setText("--");
    ui->windLabel->setText("--");
    ui->weatherIconLabel->clear();

    // Limpar tabela de previsao
    ui->forecastTableWidget->setRowCount(0);

    // Desabilitar botao de adicionar favoritos
    ui->addFavoritesPushButton->setEnabled(false);
}

void MainWindow::loadFirstFavorite()
{
    QStringList favorites = m_locationManager->getFavorites();

    if (favorites.isEmpty()) {
        setStatusMessage("Enter a city name to search for weather data");
        return;
    }

    // Pegar primeira cidade da lista (nome completo)
    QString firstFavorite = favorites.first();

    // Extrair nome da cidade (antes da primeira vírgula) para busca na API
    QString cityName = firstFavorite.split(",").first().trimmed();

    m_citySearchWidget->setText(firstFavorite);
    m_currentCity = cityName;

    // Buscar clima automaticamente
    setStatusMessage("Loading weather for " + cityName + "...");
    m_weatherService->fetchWeather(cityName);
    m_weatherService->fetchForecast(cityName);
}

void MainWindow::onFavoritesChanged()
{
    updateFavoritesList();
}

void MainWindow::updateFavoritesList()
{
    ui->favoritesListWidget->clear();
    QStringList favorites = m_locationManager->getFavorites();
    ui->favoritesListWidget->addItems(favorites);
}

void MainWindow::setStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
