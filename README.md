# WeatherLamp

## 機能
明日の天気に応じて、LEDが点灯する。

## ターゲット
ESP-WROOM-02開発ボード
<https://www.switch-science.com/catalog/2500/>

## 説明
手元のESP-WROOM-02開発ボードを用いてインターネットから情報を取得するデバイスを作りました。
1. デバイスの設定情報をダウンロードする。（任意の場所においておく）
2. 設定情報をもとに、APIのパラメタを組み立てる。
<http://openweathermap.org/>
3. OpenWeatherMapにアクセスし、明日の天気を取得する。
4. 受信したJSONの中のiconの値から天気を判断し、対応するLEDを点灯させる。

