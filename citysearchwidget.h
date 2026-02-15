#ifndef CITYSEARCHWIDGET_H
#define CITYSEARCHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

struct CityResult {
    QString name;
    QString state;
    QString country;
    double lat;
    double lon;

    QString displayName() const {
        QString display = name;
        if (!state.isEmpty()) {
            display += ", " + state;
        }
        display += ", " + country;
        return display;
    }

    QString fullName() const {
        return displayName();
    }
};

class CitySearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CitySearchWidget(QWidget *parent = nullptr);
    QString text() const;
    void setText(const QString &text);
    void clear();
    CityResult selectedCity() const { return m_selectedCity; }

signals:
    void citySelected(const QString &cityName, double lat, double lon);

private slots:
    void onTextChanged(const QString &text);
    void onSearchTimeout();
    void onSearchFinished(QNetworkReply *reply);
    void onSuggestionClicked(QListWidgetItem *item);

private:
    QLineEdit *m_lineEdit;
    QListWidget *m_suggestionsList;
    QNetworkAccessManager *m_networkManager;
    QTimer *m_searchTimer;
    QList<CityResult> m_results;
    CityResult m_selectedCity;
    bool m_ignoreTextChange;

    void searchCities(const QString &query);
    void hideSuggestions();
    QString apiKey() const;

    const QString GEO_URL = "https://api.openweathermap.org/geo/1.0/direct";
};

#endif // CITYSEARCHWIDGET_H
