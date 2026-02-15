# Weather Dashboard

A modern, desktop weather application built with **C++ and Qt 6 framework**. Features real-time weather data, intelligent city search with autocomplete, and persistent favorites management using the **OpenWeatherMap API**.

---

## ✒️ Author

- **Name:** Bernardo Vivian Vieira
- **Email:** 179835@upf.br
- **Course:** Ciência da Computação (UPF)
- **Period:** 2025/2

---

## ✨ Main Features

- **Smart City Search**: Real-time autocomplete with location details (City, State, Country)
- **Current Weather**: Temperature, feels like, humidity, wind speed, and weather icons
- **5-Day Forecast**: Daily predictions with temperature ranges and conditions
- **Favorites Management**: Save cities and auto-load your first favorite on startup
- **Clean Interface**: Fixed window size, clear button, and intuitive layout

---

## 🛠️ Technologies

- **C++17** with **Qt 6** (QtWidgets, QtNetwork)
- **OpenWeatherMap API** (Weather Data 2.5 & Geocoding)
- **CMake** build system

---

## 🚀 Prerequisites

- Qt 6.2+
- CMake 3.16+
- C++17 compatible compiler

---

## 🔑 API Key (required)

The app reads the OpenWeatherMap API key from the **environment variable** `OPENWEATHERMAP_API_KEY`. No key is included in the repository.

1. Get a free API key at [openweathermap.org/api](https://openweathermap.org/api).
2. Set the variable before running:

   **Linux/macOS (current shell):**
   ```bash
   export OPENWEATHERMAP_API_KEY="your-api-key-here"
   ./qt-weather-dashboard
   ```

   **Linux/macOS (persistent):** add to `~/.bashrc` or `~/.profile`:
   ```bash
   export OPENWEATHERMAP_API_KEY="your-api-key-here"
   ```

   **Windows (PowerShell):**
   ```powershell
   $env:OPENWEATHERMAP_API_KEY = "your-api-key-here"
   .\qt-weather-dashboard.exe
   ```

---

## 📦 Build & Run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./qt-weather-dashboard
```

(On Windows, run the `.exe` from the build directory; on macOS you may need to run the app from inside the bundle.)

---

## 📃 First Use

1. Set `OPENWEATHERMAP_API_KEY` (see above).
2. Type a city name.
3. Select from autocomplete suggestions.
4. Add to favorites for quick access.
5. Next launch: first favorite loads automatically!

---

## Citation

If you use this project in academic or applied work, please cite:

```bibtex
@software{vieira2025_qt_weather,
  author = {Vieira, Bernardo Vivian},
  title = {qt-weather-dashboard: A modern weather dashboard with C++ and Qt using OpenWeatherMap API},
  year = {2025},
  url = {https://github.com/bernardovvieira/qt-weather-dashboard},
  note = {Desktop application}
}
```

---

## 📧 Contact

- **Author:** Bernardo Vivian Vieira
- **Email:** 179835@upf.br

---

## License

MIT License. See [LICENSE](LICENSE).
