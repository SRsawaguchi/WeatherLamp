# WeatherLamp

## 機能
明日の天気に応じて、LEDが点灯する。

## ターゲット
<https://www.switch-science.com/catalog/2500/>

## 説明
手元のESP-WROOM-02開発ボードを用いてインターネットから情報を取得するデバイスを作りました。
１.デバイスの設定情報をダウンロードする。（任意の場所においておく）
２.設定情報をもとに、APIのパラメタを組み立てる。
<http://openweathermap.org/>
３.OpenWeatherMapにアクセスし、明日の天気を取得する。
４.受信したJSONの中のiconの値から天気を判断し、対応するLEDを点灯させる。

