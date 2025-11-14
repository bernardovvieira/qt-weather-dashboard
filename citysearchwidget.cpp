#include "citysearchwidget.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>

CitySearchWidget::CitySearchWidget(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(new QLineEdit(this))
    , m_suggestionsList(new QListWidget(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_searchTimer(new QTimer(this))
    , m_ignoreTextChange(false)
{
    // Setup layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Setup line edit
    m_lineEdit->setPlaceholderText("Type city name...");
    layout->addWidget(m_lineEdit);

    // Setup suggestions list
    m_suggestionsList->setMaximumHeight(150);
    m_suggestionsList->hide();
    layout->addWidget(m_suggestionsList);

    // Setup timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(300);

    // Connections
    connect(m_lineEdit, &QLineEdit::textChanged,
            this, &CitySearchWidget::onTextChanged);
    connect(m_searchTimer, &QTimer::timeout,
            this, &CitySearchWidget::onSearchTimeout);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &CitySearchWidget::onSearchFinished);
    connect(m_suggestionsList, &QListWidget::itemClicked,
            this, &CitySearchWidget::onSuggestionClicked);
}

QString CitySearchWidget::text() const
{
    return m_lineEdit->text();
}

void CitySearchWidget::setText(const QString &text)
{
    m_ignoreTextChange = true;
    m_lineEdit->setText(text);
    m_ignoreTextChange = false;
}

void CitySearchWidget::clear()
{
    m_ignoreTextChange = true;
    m_lineEdit->clear();
    m_ignoreTextChange = false;
    hideSuggestions();
    m_selectedCity = CityResult();
}

void CitySearchWidget::onTextChanged(const QString &text)
{
    if (m_ignoreTextChange) {
        return;
    }

    m_searchTimer->stop();

    if (text.trimmed().length() < 3) {
        hideSuggestions();
        return;
    }

    m_searchTimer->start();
}

void CitySearchWidget::onSearchTimeout()
{
    QString query = m_lineEdit->text().trimmed();
    if (query.length() >= 3) {
        searchCities(query);
    }
}

void CitySearchWidget::searchCities(const QString &query)
{
    QUrl url(GEO_URL);
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    urlQuery.addQueryItem("limit", "5");
    urlQuery.addQueryItem("appid", API_KEY);
    url.setQuery(urlQuery);

    QNetworkRequest request(url);
    m_networkManager->get(request);
}

void CitySearchWidget::onSearchFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        hideSuggestions();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isArray()) {
        hideSuggestions();
        reply->deleteLater();
        return;
    }

    QJsonArray array = doc.array();
    m_results.clear();
    m_suggestionsList->clear();

    for (const QJsonValue &value : array) {
        if (!value.isObject()) continue;

        QJsonObject obj = value.toObject();
        CityResult result;
        result.name = obj["name"].toString();
        result.state = obj["state"].toString();
        result.country = obj["country"].toString();
        result.lat = obj["lat"].toDouble();
        result.lon = obj["lon"].toDouble();

        m_results.append(result);
        m_suggestionsList->addItem(result.displayName());
    }

    if (!m_results.isEmpty()) {
        m_suggestionsList->show();
    } else {
        hideSuggestions();
    }

    reply->deleteLater();
}

void CitySearchWidget::onSuggestionClicked(QListWidgetItem *item)
{
    int index = m_suggestionsList->row(item);
    if (index >= 0 && index < m_results.size()) {
        const CityResult &result = m_results[index];
        m_selectedCity = result;

        setText(result.displayName());

        hideSuggestions();
        emit citySelected(result.name, result.lat, result.lon);
    }
}

void CitySearchWidget::hideSuggestions()
{
    m_suggestionsList->hide();
    m_suggestionsList->clear();
    m_results.clear();
}
