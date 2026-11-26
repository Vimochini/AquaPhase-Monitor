# AquaPhase Monitor — Clausius–Clapeyron Verification

A low-cost IoT experiment that logs the temperature and pressure of water vapor
in a sealed steamer with an ESP32 and a BMP280 sensor, then checks the data
against the Clausius–Clapeyron relation for water.

**Project page:** open [`index.html`](index.html) in any modern browser.
The page includes the theory, setup photos, wiring diagram, firmware, the demo
video, the logged data with a fitted plot, and the conclusions. Chart.js is
loaded from a CDN, so an internet connection is needed for the plot.

## Contents

| Path | What it is |
|------|-----------|
| `index.html` | Main project page (single file, no build step) |
| `firmware/esp32_bmp280_logger.ino` | ESP32 firmware: reads BMP280, logs to CSV, serves the file over Wi-Fi |
| `data/bmp280_data.csv` | Logged readings: `Timestamp, Temperature_C, Pressure_hPa` (10 rows, 2025-08-31) |
| `assets/images/` | Figures used on the page (wiring, setup photos, derivation, equation, boiling point) |
| `assets/images/original/` | Source images before upscaling |
| `assets/video/clip.mp4` | Demo clip of the experiment; `poster.png` is its first frame |
| `docs/report.docx` | Written report with background, method and discussion |
| `archive/earlier-version.html` | Earlier draft of the project page, kept for reference |

## The experiment

1. Water is heated in an airtight steamer. The BMP280 sits under the lid, inside the sealed container.
2. The ESP32 reads temperature and pressure every 10 seconds and timestamps each reading via NTP (GMT+5:30).
3. Readings are appended to `bmp280.csv` on the ESP32's SPIFFS storage.
4. The CSV is downloadable from the ESP32 at `http://<esp32-ip>/bmp280.csv`.

Hardware: ESP32-WROOM-32, BMP280 (I²C, SDA = GPIO 21, SCL = GPIO 22, address 0x76), 3.3 V supply.
Wiring is shown in `assets/images/v.jpg`.

## Method

The Clausius–Clapeyron relation in logarithmic form is

    ln P = −(ΔH_vap / R) · (1/T) + C

So a plot of ln(P) against 1/T should be a straight line with slope −ΔH_vap / R.
The page converts the logged values to Pa and K, fits a least-squares line, and
reports the slope, R², and the latent heat implied by the slope (ΔH_vap = −b·R).

## Results (current dataset)

| Quantity | Value |
|---------|-------|
| Readings | 10, from 50.11 °C to 119.56 °C |
| Fitted slope | −155.4 K |
| R² | 0.9972 |
| Latent heat from fit | 1.29 kJ/mol |
| Accepted ΔH_vap of water at 100 °C | ≈ 40.7 kJ/mol |

The ln(P) vs 1/T data are linear, so the qualitative trend matches the theory.
The fitted latent heat is about 30 times smaller than the accepted value, so this
dataset does **not** yet verify the magnitude of ΔH_vap. Likely causes:

- The BMP280 reads total pressure in the steamer, including air, not the vapor pressure alone.
- The temperature range is narrow (about 70 K, with small pressure changes).
- The sensor accuracy is about ±1 °C and ±1 hPa.

## Running the firmware

1. Install the Arduino IDE with the ESP32 board package.
2. Install the `Adafruit BMP280 Library` from the Library Manager.
3. Open `firmware/esp32_bmp280_logger.ino` and set your Wi-Fi details:

   ```cpp
   const char* ssid     = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

4. Upload to the ESP32. Open the Serial Monitor at 115200 baud to see the IP address and each logged reading.
5. Browse to `http://<esp32-ip>/bmp280.csv` to download the log.

The Wi-Fi credentials are placeholders in this repository. Do not commit real ones.

## Reproducing the analysis

1. Open `index.html`. The plot and fit are computed from the readings embedded in the page's script.
2. To use new data, replace the `READINGS` array in the script with your rows from `bmp280.csv`.
   Keep the same units: temperature in °C, pressure in hPa.

## Notes

- Images in `assets/images/` were upscaled from small originals (see `assets/images/original/`).
  Replace them with full-resolution files to sharpen the page.
- The report and the page describe the same sealed-steamer setup.
- Authors: Vimochini.
