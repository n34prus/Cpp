# Cpp
C and C++ home projects

///////////////////////////////////

OpenGLES Application:  
домашний проект, песочница с opengl и кроссплатформенной мобильной разработкой.  
Цель - освоить OpenGL и особенности разработки под андроид на С++, реализовав физический 2д движок.  
Судя по всему, OpenGL гораздо удобнее было бы осваивать на Win OS :(  
На данный момент приложение представляет собой экран, на котором создаются по тапу треугольники и падают вниз (в зависимости от
угла наклона телефона). При столкновении с "точкой" в нижней половине экрана срабатывает коллизия с законом сохранения импульса.  
Треугольники ориентированы паралельно вектору скорости. Треугольники удаляются при вылете за зону видимости или по истечению времени жизни.  

\OpenGLESApp1.Shared\customfig.h - классы примитивов, физики и глобальных настроек  
\OpenGLESApp1.Android.NativeActivity\main.cpp - нагрузка на андроид, основная точка входа  

//////////////////////////////////

ПО Тепловая завеса воздушная ТЗВ-3500:
коммерческий проект, прошивка для устройства на ядре AVR (контроллер AT90CAN128).  
Устройство - воздушная завеса на верях трамвая. Умеет принимать команды по CAN, отслеживать обороты 10 вентиляторов,
измерять напряжение собственного питания, следить за состоянием линии термозащиты, следить за температурой нагревательного блока,
имеет различную реакцию на нештатные ситуации.

\AtmelStudio\TZV_CAN\TZV_CAN\  
CANdrv.h , CANdrv.cpp - драйверы обмена данными по сети CAN  
NTCdrv.h - драйвер работы с термодатчиком  
defines.h - глобальные константы  
main.cpp - логика работы устройства, основная точка входа  

//////////////////////////////////

test_container:  
тестовое задание: Большое количество однотипных записей загружается в некоторый контейнер.  
После чего выполняется небольшая серия удалений и вставок записей по ссылке на следующий\предыдущий элемент,
а так же запросы к порядковым индексам данных по прямой ссылке.  
После чего данные выгружаются обратно.  
Создайте класс контейнера, оптимизированного для работы в описанном режиме.  

реализация:  
Класс-контейнер, реализованный в виде двусвязного списка с возможностью обращения к элементам  
как по абсолютному индексу `[i]`, так и через указатель `List<T>::Iterator`.  
Методы и операторы контейнера:  
`
    push_front  
    push_back  
    pop_front  
    pop_back  
    insert  
    remove  
    clear  
    get_size  
    []  
    begin  
    last  
    end  
`  
Методы и операторы итератора:  
`
    ++
    --
    +=
    -=
    *
    ==
    !=
`

//////////////////////////////////

test_network:

тестовое задание:
Создайте сеть абстрактных узлов, и метод ее обновления, работающий по следующему принципу:
- Каждый узел сети может выполнить одно из следующих действий:
  - Узел сети может создать событие несущее некоторые данные, в рамках тестового задания: случайное число
  - Узел сети может подписаться на события любого соседа любого своего соседа, кроме самого себя
  - Узел сети может отписаться от любого своего соседа
  - Узел может создать новый узел и подписаться на него, при этом новый созданный узел не участвует в текущем обновлении
  - Узел не выполняет никаких действий
- При подписке узел назначает один из следующих обработчиков события:
  - Вывод в консоль "имя_отправителя -> имя_получателя: S = сумма_всех_чисел_полученных_от_этого_отправителя"
  - Вывод в консоль "имя_отправителя -> имя_получателя: N = число_событий_полученных_от_этого_отправителя"
- Если узел лишается всех соседей, то он уничтожается в конце процедуры обновления сети
Узлы считаются соседями, если один их них подписан на события другого.
В случае невозможности выполнения выбранного узлом действия, попыток выполнить другое действие не осуществляется.

Обновление узлов сети должно происходить синхронно, то есть изменения, созданные в процессе обновления уже обработанными узлами, не должны учитываться узлами которые еще не обработаны.
Вероятность каждого события задается вручную при запуске программы.
Стартовая сеть создается при запуске случайным образом с произвольными параметрами, которые вы можете по желанию задавать с клавиатуры либо задавать случайно.
Программа должна продолжать симуляцию до вырождения сети, либо до явной остановки симуляции.

//////////////////////////////////

PathFinding

домашний проект. реализация алгоритма Дейкстры на консольном cpp.
TODO: добавить A*, добавить импорт матрицы из .BMP, перенести на QT, добавить вкладку "рисование"
