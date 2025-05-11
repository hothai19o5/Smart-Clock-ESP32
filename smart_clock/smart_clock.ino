#include <WiFi.h>
#include <WebServer.h>   // Thư viện WebServer cho ESP32, cung cấp các hàm để tạo web server như on, begin, send, v.v.
#include <DNSServer.h>   // Thư viện DNSServer cho ESP32, cung cấp các hàm để tạo DNS server như start, stop, v.v.
#include <Preferences.h> // Thư viện Preferences cho ESP32, cung cấp các hàm để lưu trữ và truy xuất dữ liệu trong bộ nhớ flash như begin, putString, getString, v.v.
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <icon.h>

String ssid;                // SSID của mạng WiFi, cấu hình từ Web Server, lưu trong bộ nhớ flash
String password;            // Password của mạng WiFi, cấu hình từ Web Server, lưu trong bộ nhớ flash
String location;            // Địa điểm để lấy dữ liệu thời tiết, lưu trong flash
bool formatTime12 = false;  // 12 Hour Time Format ?
bool temperatureInC = true; // Temperature in Celius or F

unsigned long lastUpdateScreen = 0;

String city;
String localTimeStr;
float temperature;
String condition;
float humidity;
String hour;
String minute;
String day_month_year;
int clouds;

const char *apSSID = "ESP32_Config"; // Tên mạng WiFi Access Point (AP) khi không kết nối được WiFi, tên mạng là ESP32_Config
const byte DNS_PORT = 53;            // Cổng DNS, mặc định là 53, cổng này sẽ được sử dụng để tạo DNS server cho ESP32 khi ở chế độ Access Point (AP)

WebServer server(80); // Khởi tạo WebServer trên cổng 80, cổng này sẽ được sử dụng để tạo web server cho ESP32, cổng này sẽ được sử dụng để nhận thông tin cấu hình từ người dùng
DNSServer dns;        // Đối tượng dns để tạo DNS server cho ESP32, cổng này sẽ được sử dụng để chuyển hướng tất cả các request lạ về trang cấu hình của ESP32

TFT_eSPI tft = TFT_eSPI();

Preferences preferences; // Khởi tạo đối tượng Preferences để lưu trữ và truy xuất dữ liệu trong bộ nhớ flash

HTTPClient http;

const char *apiKey = "9360c650915a41aa9ea124221252104";
String url = "http://api.weatherapi.com/v1/current.json?key=";

/**
 * @brief Hàm xử lý trang chính của web server, sẽ hiển thị trang cấu hình cho người dùng
 */
void handleRoot()
{
    String page = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">

  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Settings</title>
      <style>
          /* Reset default margins and padding */
          * {
              margin: 0;
              padding: 0;
              box-sizing: border-box;
          }

          /* Body styling */
          body {
              font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
              background-color: #f4f7fa;
              color: #333;
              line-height: 1.6;
              display: flex;
              justify-content: center;
              align-items: center;
              min-height: 100vh;
              padding: 20px;
          }

          /* Main container */
          .container {
              background-color: #fff;
              padding: 30px;
              border-radius: 10px;
              box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
              max-width: 500px;
              width: 100%;
          }

          /* Heading */
          h1 {
              font-size: 2rem;
              color: #2c3e50;
              margin-bottom: 20px;
              text-align: center;
          }

          /* Form styling */
          form {
              display: flex;
              flex-direction: column;
              gap: 15px;
          }

          /* Input fields */
          input[type="text"],
          input[type="password"] {
              padding: 10px;
              font-size: 1rem;
              border: 1px solid #ddd;
              border-radius: 5px;
              width: 100%;
              transition: border-color 0.3s ease;
          }

          input[type="text"]:focus,
          input[type="password"]:focus {
              border-color: #2196F3;
              outline: none;
              box-shadow: 0 0 5px rgba(33, 150, 243, 0.3);
          }

          /* Labels */
          label {
              font-size: 1rem;
              color: #555;
              margin-bottom: 5px;
          }

          /* Select dropdown */
          select {
              padding: 10px;
              font-size: 1rem;
              border: 1px solid #ddd;
              border-radius: 5px;
              width: 100%;
              transition: border-color 0.3s ease;
              background-color: #CCCCCC;
              -webkit-appearance: none;
          }

          select:focus {
              border-color: #2196F3;
              outline: none;
              box-shadow: 0 0 5px rgba(33, 150, 243, 0.3);
          }

          /* Switch styling (unchanged) */
          .switch {
              position: relative;
              display: inline-block;
              width: 60px;
              height: 34px;
          }

          .switch input {
              opacity: 0;
              width: 0;
              height: 0;
          }

          .slider {
              position: absolute;
              cursor: pointer;
              top: 0;
              left: 0;
              right: 0;
              bottom: 0;
              background-color: #ccc;
              transition: .4s;
          }

          .slider:before {
              position: absolute;
              content: "";
              height: 26px;
              width: 26px;
              left: 4px;
              bottom: 4px;
              background-color: white;
              transition: .4s;
          }

          input:checked+.slider {
              background-color: #2196F3;
          }

          input:focus+.slider {
              box-shadow: 0 0 1px #2196F3;
          }

          input:checked+.slider:before {
              transform: translateX(26px);
          }

          .slider.round {
              border-radius: 34px;
          }

          .slider.round:before {
              border-radius: 50%;
          }

          /* Submit button */
          input[type="submit"] {
              background-color: #2196F3;
              color: #fff;
              padding: 12px;
              border: none;
              border-radius: 5px;
              font-size: 1rem;
              cursor: pointer;
              transition: background-color 0.3s ease;
              margin-top: 10px;
          }

          input[type="submit"]:hover {
              background-color: #1976D2;
          }

          /* Form row for better alignment */
          .form-row {
              display: flex;
              flex-direction: column;
              gap: 5px;
          }

          /* Checkbox row */
          .checkbox-row {
              display: flex;
              align-items: center;
              gap: 10px;
              margin-top: 0px;
          }

          /* Toggle switch row */
          .toggle-row {
              display: flex;
              justify-content: space-between;
              align-items: center;
          }

          /* Responsive design */
          @media (max-width: 600px) {
              .container {
                  padding: 20px;
              }

              h1 {
                  font-size: 1.5rem;
              }
          }
      </style>
  </head>

  <body>
      <div class="container">
          <h1 style="font-size: 32px; color:#2196F3">Smart Clock</h1>
          <form action="/submit" method="POST">
              <div class="form-row">
                  <label for="wifi">WiFi</label>
                  <input type="text" id="wifi" name="ssid" required>
              </div>
              <div class="form-row">
                  <label for="password">Password</label>
                  <input type="password" id="password" name="password" required>
              </div>
              <div class="checkbox-row">
                  <input type="checkbox" style="padding: 5px; margin-left: 5px; width: 16px; height: 16px;" id="showPassword" onclick="myFunction()">
                  <label for="showPassword">Show Password</label>
              </div>
              <div class="toggle-row">
                  <span>Show Temperature in Celsius</span>
                  <label class="switch">
                      <input type="checkbox" name="celsius" checked>
                      <span class="slider round"></span>
                  </label>
              </div>
              <div class="toggle-row">
                  <span>12-Hour Time Format</span>
                  <label class="switch">
                      <input type="checkbox" name="time_format">
                      <span class="slider round"></span>
                  </label>
              </div>
              <div class="form-row">
                  <label for="location">Location</label>
                  <select id="location" name="location" required>
                      <option value="Hanoi">Hà Nội</option>
                      <option value="Thanh%20Hoa">Thanh Hóa</option>
                      <option value="Da%20Nang">Đà Nẵng</option>
                      <option value="Ho%20Chi%20Minh">Hồ Chí Minh</option>
                  </select>
              </div>
              <input type="submit" value="Submit">
          </form>
      </div>
  </body>

  <script>
      function myFunction() {
          var x = document.getElementById("password");
          if (x.type === "password") {
              x.type = "text";
          } else {
              x.type = "password";
          }
      }
  </script>

  </html>
  )rawliteral";
    server.send(200, "text/html", page);
}

/**
 * * Hàm xử lý khi người dùng gửi thông tin cấu hình từ trang web, sẽ nhận thông tin SSID, Password và Location, ...  từ người dùng
 */
void handleSubmit()
{
    /**
     * Hàm arg(const char* name) trong WebServer là hàm để lấy thông tin từ request POST, ở đây là lấy thông tin SSID, Password và IP Address từ người dùng
     */
    ssid = server.arg("ssid");         // Nhận thông tin SSID từ người dùng
    password = server.arg("password"); // Nhận thông tin Password từ người dùng
    location = server.arg("location"); // Nhận thông tin địa chỉ IP của Server từ người dùng
    formatTime12 = (server.arg("time_format") == "on") ? true : false;
    temperatureInC = (server.arg("celsius") == "on") ? true : false;

    /**
     * Hàm send(int code, const char* content_type, const char* content) trong WebServer là hàm để gửi trang thông báo kết nối thành công
     * @param {int} code là mã trạng thái HTTP, 200 là OK
     * @param {const char*} content_type là kiểu nội dung, ở đây là "text/html"
     * @param {const char*} content là nội dung trang thông báo kết nối thành công
     */
    server.send(200, "text/html", "<h1 style='font-size: 48px; color:#2196F3; margin: 48px;'>Connecting...</h1>");

    /**
     * Hàm begin() trong Preferences là hàm để khởi tạo đối tượng Preferences, mở file cấu hình wifi trong bộ nhớ flash
     * @param {const char*} namespace là tên file cấu hình wifi, có thể là bất kỳ tên nào, ở đây là "wifi"
     * @param {bool} readOnly là tham số để chỉ định chế độ mở file, nếu là true thì chỉ mở file để đọc, nếu là false thì mở file để ghi
     */
    preferences.begin("storage", false);
    /**
     * Hàm putString(const char* key, const char* value) trong Preferences là hàm để lưu trữ dữ liệu vào file cấu hình wifi trong bộ nhớ flash
     * @param {const char*} key là tên khóa để lưu trữ dữ liệu, có thể là bất kỳ tên nào, ở đây là "ssid", "password", "server"
     * @param {const char*} value là giá trị cần lưu trữ, ở đây là ssid, password, websockets_server_host
     */
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("location", location);
    // Chuyển 2 biến sang kiểu String để lưu trong flash
    String ftStr = String(formatTime12);
    String tempStr = String(temperatureInC);
    preferences.putString("formatTime12", ftStr);
    preferences.putString("temperatureInC", tempStr);

    // Hàm end() trong Preferences là hàm để đóng file cấu hình wifi trong bộ nhớ flash
    preferences.end();

    delay(1000);

    ESP.restart(); // Khởi động lại ESP32 để áp dụng cấu hình mới
}

void updateScreen(String city, String hour, String minute, String day_month_year, float temperature, String condition, float humidity, int clouds)
{
    
    // Debug info
    Serial.println("Displaying values:");
    Serial.println("City: [" + city + "]");
    Serial.println("Condition: [" + condition + "]");
    Serial.println("Day: [" + day_month_year + "]");
    Serial.println("Temperature: " + String(temperature));
    Serial.println("Humidity: " + String(humidity));

    // Xóa
    tft.fillScreen(TFT_BLACK);
    // font 2 = 16px
    // font 4 = 26px
    // font 6 = 40px
    // font 7 = 60px
    
    // Thành phố - kiểm tra city không rỗng
    if (city.length() > 0) {
        tft.setTextColor(TFT_GREENYELLOW);
        tft.drawString(city, 25, 10, 4);
    }

    // Điều kiện thời tiết - kiểm tra condition không rỗng
    if (condition.length() > 0) {
        tft.setTextColor(TFT_WHITE);
        tft.drawString(condition, 25, 36, 4);
    }

    tft.pushImage(170, 0, 50, 50, myBitmapArray[0]); // Vẽ icon thời tiết lên

    // Giờ và phút
    tft.setTextColor(TFT_YELLOW);
    tft.drawString(hour, 10, 70, 7);
    tft.setTextColor(TFT_RED);
    tft.drawString(minute, 80, 70, 7);

    // Mây
    tft.fillRect(150, 70, 85, 30, TFT_WHITE);
    tft.setTextColor(TFT_BLACK); // Đổi màu chữ thành đen để hiển thị trên nền trắng
    tft.drawString("Clouds", 155, 75, 4);
    tft.setTextColor(TFT_BLUE);
    tft.drawString(String(clouds), 165, 105, 6);

    // Ngày
    if (day_month_year.length() > 0) {
        tft.setTextColor(TFT_WHITE);
        tft.drawString(day_month_year, 10, 155, 4);
    }
    
    // Ngày trong tuần (hard-coded "Sun" - cần cải thiện)
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Mon", 155, 155, 4);

    tft.pushImage(10, 185, 24, 24, myBitmapArray[2]);
    tft.pushImage(10, 215, 24, 24, myBitmapArray[1]);

    // Nhiệt độ - kiểm tra giá trị hợp lệ
    if (!isnan(temperature)) {
        tft.setTextColor(TFT_WHITE);
        if (temperatureInC) {
            tft.drawString(String(temperature, 1) + " C", 110, 185, 4);
        } else {
            float tempF = (temperature * 9.0 / 5.0) + 32.0;
            tft.drawString(String(tempF, 1) + " F", 110, 185, 4);
        }
    }

    // Độ ẩm - kiểm tra giá trị hợp lệ
    if (!isnan(humidity)) {
        tft.setTextColor(TFT_WHITE);
        // Chuyển humidity thành int để tránh hiển thị số thập phân
        int humidityInt = (int)humidity;
        tft.drawString(String(humidityInt) + " %", 110, 215, 4);
    }
}

void setup()
{
    Serial.begin(115200);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    preferences.begin("storage", true); // Mở flash ở chỉ đọc
    // Lấy dữ liệu từ flash
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    location = preferences.getString("location", "");
    formatTime12 = preferences.getString("formatTime12", "0") == "1";
    temperatureInC = preferences.getString("temperatureInC", "0") == "1";

    preferences.end(); // Đóng flash

    WiFi.mode(WIFI_AP_STA); // Bật cả AP và STA

    // Cấu hình chế độ Station (kết nối vào WiFi)
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0; // Biến đếm số lần thử kết nối
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nCan't connect to Wifi.");
    }

    // Cấu hình chế độ Access Point (phát WiFi)
    WiFi.softAP(apSSID);
    IPAddress myIP = WiFi.softAPIP(); // Lấy địa chỉ IP của ESP32 khi ở chế độ Access Point (AP)

    Serial.println(myIP);

    /**
     * Hàm start(uint16_t port, const char* hostname, IPAddress ip) trong DNSServer là hàm để khởi động DNS server
     * @param {uint16_t} port là cổng DNS server, mặc định là 53
     * @param {const char*} hostname là tên miền của DNS server, có thể là bất kỳ tên nào, ở đây là "*"
     * @param {IPAddress} ip là địa chỉ IP của ESP32 khi ở chế độ Access Point (AP), địa chỉ IP này sẽ được sử dụng để chuyển hướng tất cả các request lạ về trang cấu hình của ESP32
     */
    dns.start(DNS_PORT, "*", myIP);

    /**
     * Hàm on(const char* uri, WebServer::THandlerFunction handler) trong WebServer là hàm để đăng ký hàm xử lý cho request GET tới trang chính của web server
     * @param {const char*} uri là đường dẫn của trang web, ở đây là "/"
     * @param {WebServer::THandlerFunction} handler là hàm xử lý cho request GET tới trang chính của web server, ở đây là handleRoot
     */
    server.on("/", handleRoot);
    /**
     * Hàm on(const char* uri, HTTPMethod method, WebServer::THandlerFunction handler) trong WebServer là hàm để đăng ký hàm xử lý cho request POST tới trang cấu hình của web server
     * @param {const char*} uri là đường dẫn của trang web, ở đây là "/submit"
     * @param {HTTPMethod} method là phương thức HTTP, ở đây là HTTP_POST, nếu không có thì mặc định là GET
     * @param {WebServer::THandlerFunction} handler là hàm xử lý cho request POST tới trang cấu hình của web server, ở đây là handleSubmit
     */
    server.on("/submit", HTTP_POST, handleSubmit);

    /**
     * Hàm onNotFound(WebServer::THandlerFunction handler) trong WebServer là hàm để đăng ký hàm xử lý cho request không tìm thấy trang web
     */
    server.onNotFound([]()
                      {
    /**
     * Hàm sendHeader(const char* name, const char* value, bool first) trong WebServer là hàm để gửi header cho response, header để chuyển hướng về trang chính của web server
     * @param {const char*} name là tên header, ở đây là "Location"
     * @param {const char*} value là giá trị header, ở đây là "/" để chuyển hướng về trang chính của web server
     * @param {bool} first là tham số để chỉ định header đầu tiên hay không, nếu là true thì là header đầu tiên, nếu là false thì không phải, header đầu tiên là Location, header thứ 2 là Content-Type
     */
    server.sendHeader("Location", "/", true);
    /**
     * Hàm send(int code, const char* content_type, const char* content) trong WebServer là hàm để gửi trang thông báo không tìm thấy trang web, với mã trạng thái 302 thì trang web sẽ tự động chuyển hướng về trang chính của web server
     * @param {int} code là mã trạng thái HTTP, 302 là chuyển hướng, chuyển hướng tới trang chính của web server vì đã gửi header Location
     * @param {const char*} content_type là kiểu nội dung, ở đây là "text/plain"
     * @param {const char*} content là nội dung trang thông báo không tìm thấy trang web
     */
    server.send(302, "text/plain", ""); });

    // Khởi động web server trên cổng 80
    server.begin();
}

void loop()
{
    /**
     * Hàm dns.processNextRequest() trong ESP32 là hàm để xử lý các yêu cầu DNS trong chế độ AP
     * @note Hàm này sẽ được gọi trong chế độ AP để xử lý các yêu cầu DNS từ client
     */
    dns.processNextRequest();
    /**
     * Hàm server.handleClient() trong ESP32 là hàm để xử lý các yêu cầu HTTP từ client
     * @note Hàm này sẽ được gọi trong chế độ AP để xử lý các yêu cầu HTTP từ client
     */
    server.handleClient();

    preferences.begin("storage", true); // Mở flash ở chỉ đọc
    // Lấy dữ liệu từ flash
    location = preferences.getString("location", "");
    formatTime12 = preferences.getString("formatTime12", "0") == "1";
    temperatureInC = preferences.getString("temperatureInC", "0") == "1";

    preferences.end(); // Đóng flash

    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateScreen >= 10000)
    {
        lastUpdateScreen = currentMillis;
        if (location.length() > 0)
        {
            String urlQuery = url + apiKey + "&q=" + location;
            http.begin(urlQuery);

            int httpCode = http.GET();

            if (httpCode > 0)
            {
                String payload = http.getString();

                // Tạo vùng nhớ cho JSON parser
                const size_t capacity = 1024;
                DynamicJsonDocument doc(capacity);
                DeserializationError error = deserializeJson(doc, payload);

                if (!error)
                {
                    // Lấy dữ liệu từ JSON
                    city = doc["location"]["name"].as<String>();
                    localTimeStr = doc["location"]["localtime"].as<String>(); // "2025-05-11 20:50"
                    temperature = doc["current"]["temp_c"].as<float>();
                    condition = doc["current"]["condition"]["text"].as<String>();
                    humidity = doc["current"]["humidity"].as<float>();
                    clouds = doc["current"]["cloud"].as<int>();

                    if (localTimeStr.length() >= 16)
                    {
                        hour = localTimeStr.substring(11, 13);          // "20"
                        minute = localTimeStr.substring(14, 16);        // "50"
                        day_month_year = localTimeStr.substring(0, 10); // "2025-05-11"

                        if (formatTime12 && hour.length() == 2)
                        {
                            int hourInt = hour.toInt();
                            if (hourInt == 0)
                            {
                                hour = "12";
                            }
                            else if (hourInt > 12)
                            {
                                hour = String(hourInt - 12);
                            }
                        }
                    }
                    else
                    {
                        Serial.println("Invalid localTimeStr format: " + localTimeStr);
                    }

                    Serial.println("City: " + city);
                    Serial.println("Hour: " + hour);
                    Serial.println("Minute: " + minute);
                    Serial.println("Local Time: " + day_month_year);
                    Serial.println("Temperature: " + (String)temperature);
                    Serial.println("Condition: " + condition);
                    Serial.println("Humidity: " + (String)humidity);
                    Serial.println("Clouds: " + (String)clouds);
                }
                else
                {
                    Serial.print("JSON parse error: ");
                    Serial.println(error.c_str());
                    Serial.println(payload);
                }
            }
            else
            {
                Serial.println("HTTP request failed");
            }

            http.end();
        }
        else
        {
            Serial.println("Location is empty, skipping API request");
        }

        updateScreen(city, hour, minute, day_month_year, temperature, condition, humidity, clouds);
    }

    delay(100);
}
