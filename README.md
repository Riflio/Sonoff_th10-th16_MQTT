# Прошивка - простой MQTT клиент для Sonoff th10/th16 с конфигурацией через веб интерфейс.
## Sonoff th10/th16 MQTT simple client, configured by web intrerfase
Управляем рэле по MQTT протоколу. Написано в Arduino IDE.
![общий вид устройства](https://github.com/Riflio/Sonoff_th10-th16_MQTT/raw/master/doc/imgs/deviceImg1.png)

## Что умеет
* Подписка на рассылку и включение/выключение рэле в зависимости от значения топика
* Автопереподключение при разрыве связи
* Ручное переключение рэле по нажатию кнопки на устройстве
* Настройка через WEB интерфейс

## Переход в режим настройки
Зажать и удерживать кнопку, когда синий светодиод начнёт быстро моргать отпустить.
В списке WiFi сетей найти сеть "Sonoff_settings", подключиться, пароль "PRIDE1488";
Открыть браузер и перейти по адресу "http://192.168.1.1".
Вот так примерно выглядит:
![пример веб интерфейса](https://raw.githubusercontent.com/Riflio/Sonoff_th10-th16_MQTT/master/doc/imgs/settingsImg1.png)
После изменения настроек не забудьте нажать кнопку "SAVE SETTINGS".
Для выхода из режима настроек однократно нажать кнопку на корпусе устройства.
Настройки применяются сразу, перезагрузка не нужна.
Через пару секунд начнётся подключение к сети, об успешности сигнализирует синий светодиод.

## Сигналы синего светодиода
* Не светиться совсем - нет питания или глюкнул
* Медленно мигает раз в секунду - пытается подключится к WiFi
* Дважды быстро мигает в течении секунды - пытается подключится к MQTT серверу
* Моргает быстро и постоянно - в режиме настройки
* Светится постоянно - успешно подключился к WiFi и MQTT серверву, в рабочем режиме короче

## Сигналы красного светодиода
* не светится - рэле выключено
* светится - рэле включено
* светится, но не тем цветом - прикольно

## Установка
* Необходима Arduino IDE c платами расширений Esp8266 
* Подключённые библиотеки из списка зависимостей
* USB-UART конвертер
* Разобрать девайс, припаяять гребёнку и подключить к конвертеру

## Зависимости
* [pubsubclient](https://github.com/Imroy/pubsubclient) - сам MQTT клиент
* [ArduinoButtons](https://github.com/Riflio/ArduinoButtons) - обработка нажатий кнопки

## Нужно сделать
- [ ] Поддержку датчика температуры и влажности
- [ ] Рассылку при включении в сеть устройства для возможности обновления текущего статуса рэле
- [ ] Выбор реакции при потери связи с MQTT сервером/WiFi
- [ ] Настройку делать рассылку или нет при нажатии кнопки на устройстве
- [ ] Выложить готовый бинарник
- [ ] Инструкцию по прошивке из готового бинарника

## License
MIT

## Автор
[PavelK.ru](http://PavelK.ru)

