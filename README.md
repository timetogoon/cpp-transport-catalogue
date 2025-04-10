**Установка Google protobuf v21.12**  

Скачать можно по ссыле с официального [репозитория]( 
https://github.com/protocolbuffers/protobuf/releases/tag/v21.12). Достатоно скачать **cpp** релиз.  

Необходимо создать там же, куда скачан и разархивирован архив, создать папки **build-debug** и **build-release**. Также необходимо создать папку package в том месте, где будут храниться файлы готовых библиотек и вспомогательных файлов (назовем path/to/protobuf/package).
Далее последует пример для *debug* сборки. Для *release* все аналогично. Устанавливать обе сборки можно в одну папку.

Из папки **build-debug** вызвать команды:  

> cmake ../ -DCMAKE_BUILD_TYPE=Debug  ^  
-Dprotobuf_BUILD_TESTS=OFF ^  
-DCMAKE_INSTALL_PREFIX=path/to/package  
> cmake --build .  
> cmake --install .

Для сборки под Visual Studio другая последовательность: 

> cmake ../ -DCMAKE_BUILD_TYPE=Debug  ^  
-Dprotobuf_BUILD_TESTS=OFF ^  
-Dprotobuf_MSVC_STATIC_RUNTIME=OFF ^  
-DCMAKE_INSTALL_PREFIX=path/to/protobuf/package  
> cmake --build . --config Debug    
> cmake --install . --config Debug 

**Программа Транспортный справочник**

На вход подаются данные JSON-формата и выдается ответ в виде SVG-файла с визуализацией остановок и маршрутов.  
Находит кратчайший маршрут между остановками.  
Реализована сериализация и десериализация базы справочника через Google Protobuf v21.12

**Установка**  

В архиве имеется файл *CMakeLists* для установки через *cmake*. Необходимо лишь создать папки **build-debug** и **build-release** там же, где сачан и разархивирован архив. Далее последует пример для *debug* сборки. Для *release* аналогично.

Из папки **build-debug** вызвать команды:  

> cmake ../ -DCMAKE_BUILD_TYPE=Debug ^  
-DCMAKE_PREFIX_PATH=path/to/protobuf/package
> cmake --build . --config Debug   

**Запуск и работа программы**

Запуск осуществляется через командную строку (cmd). Программа имеет два режима работы: создание базы данных в файл (make_base) и загрузка базы с отработкой запросов (process_requests). 

Пример запуска программы для заполнения базы:  
transport_catalogue.exe make_base <base.json

Пример запуска программы для выполнения запросов к базе:  
transport_catalogue.exe process_requests <requests.json >out.txt


**Описание формата входных данных**

Входные данные поступают в формате JSON-объектов в 2 этапа («make_base» и «process_requests»), которые имеют следующую структуру:

1) **Создание базы данных «make_base»:**

{  
  "serialization_settings": { ... },  
  "render_settings": { ... },  
  "routing_settings": { ... },  
  "base_requests": [ ... ],  
}

2) **Запросы к базе данных «process_requests»:**

{  
  "serialization_settings": { ... },  
  "stat_requests": [ ... ]  
}

**Расшифровка ключей:**
1) serialization_settings — настройки сериализации;
2) render_settings — настройки для отрисовки изображения;
3) routing_settings — настройки скорости автобусов и времени ожидания на остановке;
4) base_requests — описание автобусных маршрутов и остановок;
5) stat_requests — запросы к транспортному справочнику.


**Заполнение базы транспортного справочника**

**Сериализация базы данных**

В ключе file указывается название файла, в который и из которого нужно считать сериализованную базу при создании базы и обработки запросов соотвественно:  
      "serialization_settings":  
      {  
          "file": "transport_catalogue.db"  
            }  
            
**Пример описания остановки:**

{  
  "type": "Stop",  
  "name": "Ривьерский мост",  
  "latitude": 43.587795,  
  "longitude": 39.716901,  
  "road_distances": {  
  "Морской вокзал": 850,  
  "Гостиница Сочи": 1740  
  }  
} 

**Описание остановки — словарь с ключами:**
1) type — строка со словом "Stop", означает, что описывается остановка;
2) name — название остановки;
3) latitude и longitude задают координаты остановки - широту и долготу;
4) road_distances — словарь, задающий расстояние до соседних остановок. Ключ — название остановки, значение — целое число в метрах.

**Пример описания автобусного маршрута:**

{  
  "type": "Bus",  
  "name": "24",  
  "stops": [  
      "Улица Докучаева",  
      "Параллельная улица",  
      "Электросети",  
      "Санаторий Родина"  
  ],  
  "is_roundtrip": false   
}
 
Описание автобусного маршрута — словарь с ключами:
1) type — строка со словом  "Bus", означающая, что описывается автобусный маршрут;
2) name — название маршрута;
3) stops — массив с названиями остановок, через которые проходит автобусный маршрут. У кольцевого маршрута название последней остановки дублирует название первой. Например: ["stop1", "stop2", "stop3", "stop1"];
4) is_roundtrip — значение типа bool. Указывает, кольцевой маршрут или нет.

Структура словаря render_settings:  
{  
  "width": 1200,  
  "height": 500,  
  "padding": 50,  
  "stop_radius": 5,  
  "line_width": 14,  
  "bus_label_font_size": 20,  
  "bus_label_offset": [  
      7,  
      15  
  ],  
  "stop_label_font_size": 18,  
  "stop_label_offset": [  
       7,  
       -3  
  ],  
  "underlayer_color": [  
      255,  
      255,  
      255,  
      0.85  
  ],  
  "underlayer_width": 3,  
  "color_palette": [  
      "green",  
      [  
          255,  
          160,  
          0  
      ],  
          "red"  
      ]  
} 

*width и height* — ключи, которые задают ширину и высоту в пикселях. Вещественное число в диапазоне от 0 до 100000. Пользователь должен самостоятельно учитывать масштабирование экрана на своём рабочем столе иначе картинка не прорисуется полностью.  
*padding* — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.  
*line_width* — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.  
*stop_radius* — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.  
*bus_label_font_size* — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.  
*bus_label_offset* — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double. Задаёт значения свойств *dx* и *dy* SVG-элемента text. Элементы массива — числа в диапазоне от –100000 до 100000.  
*stop_label_font_size* — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.  
*stop_label_offset* — смещение названия остановки относительно её координат на карте. Массив из двух элементов типа double. Задаёт значения свойств *dx* и *dy* SVG-элемента *text*. Числа в диапазоне от –100000 до 100000.  
*underlayer_color* — цвет подложки под названиями остановок и маршрутов.  
*underlayer_width* — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута *stroke-width* элемента <text>. Вещественное число в диапазоне от 0 до 100000.  
*color_palette* — цветовая палитра. Непустой массив.  
*Цвет можно указать:*  
• в виде строки, например, "red" или "black";  
• в массиве из трёх целых чисел диапазона [0, 255]. Они определяют r, g и b (красный, зеленый, синий соотвественно) компоненты цвета в формате svg::Rgb. Цвет [255, 16, 12] нужно вывести как rgb(255, 16, 12);  
• в массиве из четырёх элементов: три целых числа в диапазоне от [0, 255] и одно вещественное число в диапазоне от [0.0, 1.0]. Они задают составляющие red, green, blue и opacity (степень непрозрачности цвета от 0.0 (абсолютно прозрачно) до 1.0 (абсолютно непрозрачный цвет)) формата svg::Rgba. Цвет, заданный как [255, 200, 23, 0.85], должен быть выведен как rgba(255, 200, 23, 0.85).


**Структура словаря routing_settings**

"routing_settings": {  
      "bus_wait_time": 3,  
      "bus_velocity": 30  
} 

*bus_wait_time* — время ожидания автобуса на остановке, в минутах. Считается, что человек будет ждать любой автобус в точности указанное количество минут. Значение — целое число от 1 до 1000.  
*bus_velocity* — скорость автобуса, в км/ч. Считается, что скорость любого автобуса постоянна и в точности равна указанному числу. Время стоянки на остановках не учитывается, время разгона и торможения тоже. Значение — вещественное число от 1 до 1000.  
Данный пример задаёт время ожидания, равное 3 минутам, и скорость автобусов, равной 30 километров в час.

**Запросы к базе транспортного справочника**

**Запрос на получение информации об автобусном маршруте:**

{  
  "id": 80146768,  
  "type": "Bus",  
  "name": "13"  
}

По названию маршрута *name* выводится информация в виде словаря:

{  
  "curvature": 1.26723,  
  "request_id": 80146768,  
  "route_length": 5540,  
  "stop_count": 7,  
  "unique_stop_count": 4  
}  

В словаре содержатся ключи: *curvature* — число типа double, задающее извилистость маршрута. Извилистость равна отношению длины дорожного расстояния маршрута к длине географического расстояния;  
*request_id* — целое число, равное id соответствующего запроса Bus;  
*route_length* — целое число, равное длине маршрута в метрах;  
*stop_count* — количество остановок на маршруте;  
*unique_stop_count* — количество уникальных остановок на маршруте.  
На кольцевом маршруте, заданном остановками A, B, C, A, количество остановок равно четырём, а количество уникальных остановок равно трём.  
На некольцевом маршруте, заданном остановками A, B и C, количество остановок равно пяти (A, B, C, B, A), а уникальных — равно трём.  

Запрос на получение информации об автобусной остановке:

{  
  "id": 1571442892,  
  "type": "Stop",  
  "name": "Морской вокзал"  
}   

Название остановки name выводит информацию об автобусах, которые останавливаются на ней:

{  
  "buses": [  
      "23",  
      "36"  
  ],  
  "request_id": 1571442892  
}   

Запрос на получение изображения в виде карты формата SVG:

{  
  "id": 684758285,  
  "type": "Map"  
}  

Ответ на запрос, выеден не полностью так как занимает много места:

{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">...\n</svg>",
  "request_id": 684758285
} 

Ниже представлен ответ на данный запрос в виде картинки:

![](/examples/example_map_2.jpg)
