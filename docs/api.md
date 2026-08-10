<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Сводка API</h3></td>
    </tr>
  </table>
</div>

---

## Сводка API

Полный справочник функций, объектов, ошибок и констант SDK. Здесь собрано всё — для деталей переходите по ссылкам в конце разделов.

## Глобальные таблицы

|Глобал|Что это|
|------|-------|
|`PSE`|Основное API (он же `pse`)|
|`Logger`|Логирование (`sdk.logger`)|
|`core`|Низкоуровневый C-мост (raw)|
|`PSE.Core`|Lua-обёртка над `core`|

## Функции `PSE.*`

### Сессия

|Функция|Описание|
|-------|--------|
|`PSE.version`|Версия SDK (`"0.1.0"`)|
|`PSE.initialize()`|Подключение к игре (Shared Memory). Возвращает `true` = live, `false` = mock (при недоступности игры включает mock автоматически)|
|`PSE.deinitialize()`|Отключение от игры / сброс mock|
|`PSE.synchronize()`|Синхронизация с игрой|
|`PSE.millis()`|Миллисекунды с запуска процесса|
|`PSE.sleep(ms)`|Пауза на `ms` миллисекунд|

### Хелперы

|Функция|Описание|
|-------|--------|
|`PSE.vec(x, y, z)`|Вектор `{x, y, z}` (или из таблицы `{x[1], x[2], x[3]}`)|
|`PSE.quat(x, y, z, w)`|Кватернион `{x, y, z, w}` (по умолчанию `w = 1`)|
|`PSE.color(r, g, b)`|Цвет → `u32` `0xRRGGBB`|
|`PSE.guid(v)`|GUID из объекта / таблицы с `.guid` / числа|
|`PSE.register(name, guidOrEntity)`|Регистрация объекта в реестре по имени|
|`PSE.get(key)`|Получение объекта по имени или GUID|

### Константы

|Поле|Содержит|
|----|--------|
|`PSE.Core`|Модуль `sdk.core` (Lua-обёртка)|
|`PSE.Logger`|Модуль `sdk.logger`|
|`PSE.Registers`|Модуль `sdk.registers`|
|`PSE.Mesh`|Перечисление `MESH`|
|`PSE.Material`|Перечисление `MATERIAL`|
|`PSE.Class`|Перечисление `CLASS`|
|`PSE.Command`|Перечисление `COMMAND`|
|`PSE.Event`|Перечисление `EVENT`|
|`PSE.Result`|Перечисление `RESULT`|
|`PSE.palette`|Именованные цвета (`PSE.palette.orange` и т.д.)|
|`PSE.names.mesh(v)`|Имя меша по значению|
|`PSE.names.material(v)`|Имя материала по значению|
|`PSE.names.class(v)`|Имя класса по значению|
|`PSE.names.command(v)`|Имя команды по значению|
|`PSE.names.result(v)`|Имя результата по значению|

### Создание объектов

|Функция|Описание|
|-------|--------|
|`PSE.createMeshObject(opts)`|Новый меш-объект (fluent-цепочка)|
|`PSE.createStaticMesh(opts)`|Статичный меш (без GUID, нельзя трансформировать)|
|`PSE.createElement(class, opts)`|Элемент любого класса|
|`PSE.spawnMeshObject(opts)`|Создать меш и сразу заспавнить|
|`PSE.spawnStaticMesh(opts)`|Создать статик и сразу заспавнить|
|`PSE.spawnElement(class, opts)`|Создать элемент и сразу заспавнить|

### Готовые обёртки `PSE.create...`

|Команда|Класс|
|-------|-----|
|`PSE.createButton(opts)`|`BUTTON`|
|`PSE.createDoor(opts)`|`DOOR`|
|`PSE.createLamp(opts)`|`LAMP`|
|`PSE.createTrigger(opts)`|`TRIGGER`|
|`PSE.createWeightCube(opts)`|`WEIGHT_CUBE`|
|`PSE.createLaserTx(opts)`|`LASER_TX`|
|`PSE.createLaserRx(opts)`|`LASER_RX`|
|`PSE.createLaserRelay(opts)`|`LASER_RELAY`|
|`PSE.createLaserPanel(opts)`|`LASER_PANEL`|
|`PSE.createFaithPlate(opts)`|`FAITH_PLATE`|
|`PSE.createIndicator(opts)`|`INDICATOR`|
|`PSE.createPedestalButton(opts)`|`PEDESTAL_BUTTON`|
|`PSE.createSolverButton(opts)`|`SOLVER_BUTTON`|
|`PSE.createWindow(opts)`|`WINDOW`|

### События

|Функция|Описание|
|-------|--------|
|`PSE.on(eventName, fn)`|Регистрация обработчика события|
|`PSE.onElementChanged(fn)`|Сокращение для `ELEMENT_CHANGED`|
|`PSE.poll()`|Одно событие из очереди (вызывает обработчики)|
|`PSE.pollAll()`|Все события из очереди списком|
|`PSE.run(duration)`|Цикл событий (секунды; `nil` = бесконечно)|

### Mock

|Функция|Описание|
|-------|--------|
|`PSE.mock.emit(target, state)`|Мок-событие `ELEMENT_CHANGED` по имени или GUID|
|`PSE.mock.emitRaw(guid, callbackId, state)`|Мок-событие напрямую по GUID и `callbackId`|

## Объекты-контейнеры

### `PSE.player`

|Метод|Описание|
|-----|--------|
|`getPosition()`|Позиция `{x, y, z}`|
|`setPosition(x, y, z)`|Переместить игрока|
|`getRotation()`|Поворот (кватернион `{x, y, z, w}`)|
|`setRotation(x, y, z, w)`|Установить кватернион|
|`setDegree(pitch, yaw, roll)`|Поворот в градусах (pitch = Y, yaw = Z, roll = X)|
|`getDegree()`|Поворот в градусах → `pitch, yaw, roll`|
|`spawn()`|Заспавнить игрока|
|`kill()`|Убить игрока|

### `PSE.game`

|Метод|Описание|
|-----|--------|
|`initialize()` / `deinitialize()`|Поднять / опустить уровень SDK|
|`setCheatsEnabled(b)` / `getCheatsEnabled()`|Чит-режим|
|`setNoclip(b)` / `getNoclip()`|Ноуклип (нужен чит-режим)|
|`setGravity(g)` / `getGravity()`|Гравитация|
|`checkGuid(guid)`|Проверка валидности GUID|

### `PSE.gun`

|Метод|Описание|
|-----|--------|
|`setEnabled(b)` / `getEnabled()`|Включить / выключить пушку|
|`use()`|Взять объект|
|`release()`|Отпустить объект|
|`throw()`|Бросить объект|

### `PSE.flashlight`

|Метод|Описание|
|-----|--------|
|`setEnabled(b)` / `getEnabled()`|Включить / выключить фонарик|
|`setState(b)` / `getState()`|Состояние фонарика|

## Методы `Element` (создаваемые объекты)

### Команды fluent-цепочки (до `:create()`)

|Метод|Описание|
|-----|--------|
|`position(x, y, z)`|Позиция|
|`rotation(x, y, z, w)`|Поворот (кватернион)|
|`scale(x, y, z)`|Масштаб|
|`transform(t)`|Трансформация целиком (`quat`/`location`/`scale`)|
|`state(s)`|Начальное состояние|
|`visible(b)`|Видимость|
|`onChange(fn)`|Callback при изменении|
|`register(i, v)`|Регистр `i` = `v`|
|`registers(t)`|Несколько регистров сразу|
|`name(n)`|Имя (для `PSE.get`)|
|`create()`|Создать в игре (возвращает объект с GUID)|

### Методы после создания

|Метод|Описание|
|-----|--------|
|`applyTransform()`|Применить сохранённый transform|
|`setState(s)` / `getState()`|Состояние|
|`setVisibility(b)` / `getVisibility()`|Видимость|
|`setPosition(x, y, z)`|Переместить|
|`setRotation(x, y, z, w)`|Поворот (кватернион)|
|`setDegree(pitch, yaw, roll)`|Поворот в градусах|
|`setScale(x, y, z)`|Масштаб|
|`setTransform(t)` / `getTransform()`|Трансформация|
|`getDegree()`|Углы → `pitch, yaw, roll`|
|`setClass(c)` / `getClass()`|Класс элемента|
|`setRegister(i, v)` / `getRegister(i)`|Регистр|
|`setRegisters(t)` / `getRegisters()`|Все регистры|
|`setCallback(fn)`|Callback `fn(state, guid)`|
|`destroy()`|Уничтожить элемент|

### Свойства

`guid` — GUID в игре · `name` — имя · `class` — класс · `registers` — регистры `[1..8]`.

## Методы `MeshObject`

### Команды fluent-цепочки

|Метод|Описание|
|-----|--------|
|`position` / `rotation` / `scale` / `transform`|Трансформация|
|`geometry(m)`|Меш (геометрия)|
|`texture(m)`|Материал (текстура)|
|`visible(b)`|Видимость|
|`name(n)`|Имя|
|`create()`|Создать в игре|

### Методы после создания

`applyTransform()` · `setMaterial(m)` · `setMesh(m)` · `setVisibility(b)` / `getVisibility()` · `setPosition` · `setRotation` · `setDegree` · `setScale` · `setTransform(t)` / `getTransform()` · `getDegree()` · `destroy()`.

> [!WARNING]
> У статичных мешей нет GUID — их нельзя трансформировать (`applyTransform` вызовет ошибку) и нельзя уничтожить.

## Классы элементов (`CLASS`)

|Имя|Код|Имя|Код|
|----|----|----|----|
|`ENTRY_ELEVATOR`|`0x0000`|`LASER_RX`|`0x0009`|
|`EXIT_ELEVATOR`|`0x0001`|`LASER_RELAY`|`0x000A`|
|`DOOR`|`0x0002`|`LASER_PANEL`|`0x000B`|
|`BUTTON`|`0x0003`|`LASER_CUBE`|`0x000C`|
|`ANTI_EXPROPRIATION_FIELD`|`0x0004`|`WEIGHT_CUBE`|`0x000D`|
|`PEDESTAL_BUTTON`|`0x0005`|`FAITH_PLATE`|`0x000E`|
|`SOLVER_BUTTON`|`0x0006`|`PANEL`|`0x000F`|
|`INDICATOR`|`0x0007`|`STAIRS`|`0x0010`|
|`LASER_TX`|`0x0008`|`CUBE_DROPPER`|`0x0011`|

|Имя|Код|Имя|Код|
|----|----|----|----|
|`SOLVER_GUN_PEDESTAL`|`0x0012`|`WINDOW`|`0x0014`|
|`FLASHLIGHT`|`0x0013`|`TRIGGER`|`0x0015`|

|Имя|Код|
|----|----|
|`LAMP`|`0x0016`|

## Меши (`MESH`)

|Имя|Код|Имя|Код|
|----|----|----|----|
|`PLANE`|`0x0000`|`U_OUTER`|`0x000A`|
|`FACE`|`0x0001`|`DOOR_FRAME`|`0x000B`|
|`CUBE`|`0x0002`|`LASER_FRAME`|`0x000C`|
|`CUP_INNER`|`0x0003`|`LASER_FRAME_SHIFTED`|`0x000D`|
|`CUP_OUTER`|`0x0004`|`PLANE_Z_SHIFTED`|`0x000E`|
|`II_INNER`|`0x0005`|`O_INNER`|`0x0007`|
|`II_OUTER`|`0x0006`|`O_OUTER`|`0x0008`|
|`U_INNER`|`0x0009`|`—`|`—`|

## Материалы (`MATERIAL`)

|Имя|Код|Имя|Код|
|----|----|----|----|
|`WALL_WHITE_SMALL`|`0x0000`|`FLOOR_WHITE`|`0x0008`|
|`WALL_WHITE_MEDIUM`|`0x0001`|`FLOOR_BLACK`|`0x0009`|
|`WALL_WHITE_DOUBLE`|`0x0002`|`WINDOW_METAL_GRID`|`0x000A`|
|`WALL_WHITE_BIG`|`0x0003`|`WINDOW_GLASS_METAL_GRID`|`0x000B`|
|`WALL_WHITE_ABSOLUTE_SCIENCE`|`0x0004`|`WALL_YELLOW_1_0`|`0x000C`|
|`WALL_BLACK_SMALL`|`0x0005`|`WALL_YELLOW_1_5`|`0x000D`|
|`WALL_BLACK_MEDIUM`|`0x0006`|`—`|`—`|
|`WALL_BLACK_BIG`|`0x0007`|`—`|`—`|

## Ошибки (`RESULT`)

Код результата команды. `0` — успех, всё остальное — ошибка.

|Код|Значение|Описание|
|----|--------|--------|
|`0x00000000`|`SUCCESS`|Успех|
|`0x00000001`|`ERROR_COMMAND_NOT_FOUND`|Команда не найдена|
|`0x00000002`|`ERROR_COMMAND_NOT_IMPLEMENTED`|Команда не реализована|
|`0x00000003`|`ERROR_EXECUTION_FAILED`|Ошибка выполнения|
|`0x00000004`|`ERROR_ARRAY_INDEX_OUT_OF_RANGE`|Индекс вне диапазона|
|`0x00000005`|`ERROR_GUID_NOT_FOUND`|GUID не найден|
|`0x00000006`|`ERROR_INVALID_OBJECT_TYPE`|Неверный тип объекта|
|`0x00000007`|`ERROR_GAME_NOT_INITIALIZED`|Игра не инициализирована|
|`0xFFFFFFFF`|`NONE`|Нет результата|

### Поведение ошибок

```lua
--- Обычный вызов: при ошибке кидает Lua-исключение.
PSE.game:setGravity(-5)          --- успех
door:getState()                  --- ошибка -> 'Core.call: ELEMENT_GET_STATE failed -> ERROR_GUID_NOT_FOUND (0x00000005)'

--- PSE.Core.call с opts.assert = false: возвращает nil, имя ошибки, таблицу.
local ok, rname = PSE.Core.call("ELEMENT_GET_STATE", { guid = 999 }, true, { assert = false })
```

Ошибки в обработчиках событий и callback'ах не падают — логируются через `Logger.error` (`HANDLER` / `CALLBACK`).

## События (`EVENT`)

|Имя|Код|Поля события|
|----|----|-----------|
|`ELEMENT_CHANGED`|`0x00000000`|`event`, `guid`, `state`|
|`GAME_TICK_OVERFLOW`|`0x00000001`|`event`|
|`NONE`|`0xFFFFFFFF`|—|

## Команды (`COMMAND`)

### Игра — `0x0010xxxx`

|Команда|Код|
|-------|----|
|`GAME`|`0x00100000`|
|`GAME_SET_CHEATS_ENABLED`|`0x00100001`|
|`GAME_GET_CHEATS_ENABLED`|`0x00100002`|
|`GAME_SET_CHEATS_NOCLIP`|`0x00100003`|
|`GAME_GET_CHEATS_NOCLIP`|`0x00100004`|
|`GAME_SET_GRAVITY`|`0x00100005`|
|`GAME_GET_GRAVITY`|`0x00100006`|
|`GAME_CHECK_GUID_IS_VALID`|`0x00100007`|
|`GAME_INITIALIZE`|`0x00100008`|
|`GAME_DEINITIALIZE`|`0x00100009`|

### Игрок — `0x0020xxxx`

|Команда|Код|
|-------|----|
|`PLAYER`|`0x00200000`|
|`PLAYER_SET_LOCATION`|`0x00200001`|
|`PLAYER_GET_LOCATION`|`0x00200002`|
|`PLAYER_SET_ROTATION`|`0x00200003`|
|`PLAYER_GET_ROTATION`|`0x00200004`|
|`PLAYER_SPAWN`|`0x00200005`|
|`PLAYER_KILL`|`0x00200006`|

### Пушка — `0x0030xxxx`

|Команда|Код|
|-------|----|
|`SOLVER_GUN`|`0x00300000`|
|`SOLVER_GUN_SET_ENABLED`|`0x00300001`|
|`SOLVER_GUN_GET_ENABLED`|`0x00300002`|
|`SOLVER_GUN_ACTION_USE`|`0x00300003`|
|`SOLVER_GUN_ACTION_RELEASE`|`0x00300004`|
|`SOLVER_GUN_ACTION_THROW`|`0x00300005`|

### Фонарик — `0x0040xxxx`

|Команда|Код|
|-------|----|
|`FLASHLIGHT`|`0x00400000`|
|`FLASHLIGHT_SET_ENABLED`|`0x00400001`|
|`FLASHLIGHT_GET_ENABLED`|`0x00400002`|
|`FLASHLIGHT_SET_STATE`|`0x00400003`|
|`FLASHLIGHT_GET_STATE`|`0x00400004`|

### Статичный меш — `0x0050xxxx`

|Команда|Код|
|-------|----|
|`STATIC_MESH`|`0x00500000`|
|`STATIC_MESH_CREATE`|`0x00500001`|
|`REPLACE` (служебная)|`0x05062025`|

### Динамический меш — `0x0060xxxx`

|Команда|Код|
|-------|----|
|`DYNAMIC_MESH`|`0x00600000`|
|`DYNAMIC_MESH_CREATE`|`0x00600001`|
|`DYNAMIC_MESH_SET_MATERIAL`|`0x00600002`|
|`DYNAMIC_MESH_SET_MESH`|`0x00600003`|
|`DYNAMIC_MESH_SET_VISIBILITY`|`0x00600004`|
|`DYNAMIC_MESH_GET_VISIBILITY`|`0x00600005`|
|`DYNAMIC_MESH_SET_TRANSFORM`|`0x00600006`|
|`DYNAMIC_MESH_GET_TRANSFORM`|`0x00600007`|
|`DYNAMIC_MESH_DESTROY`|`0x00600008`|

### Элементы — `0x0070xxxx`

|Команда|Код|
|-------|----|
|`ELEMENT`|`0x00700000`|
|`ELEMENT_CREATE`|`0x00700001`|
|`ELEMENT_SET_CALLBACK`|`0x00700002`|
|`ELEMENT_GET_CALLBACK`|`0x00700003`|
|`ELEMENT_SET_STATE`|`0x00700004`|
|`ELEMENT_GET_STATE`|`0x00700005`|
|`ELEMENT_SET_VISIBILITY`|`0x00700006`|
|`ELEMENT_GET_VISIBILITY`|`0x00700007`|
|`ELEMENT_SET_TRANSFORM`|`0x00700008`|
|`ELEMENT_GET_TRANSFORM`|`0x00700009`|
|`ELEMENT_SET_CLASS`|`0x0070000A`|
|`ELEMENT_GET_CLASS`|`0x0070000B`|
|`ELEMENT_SET_REGISTER`|`0x0070000C`|
|`ELEMENT_GET_REGISTER`|`0x0070000D`|
|`ELEMENT_SET_ALL_REGISTERS`|`0x0070000E`|
|`ELEMENT_GET_ALL_REGISTERS`|`0x0070000F`|
|`ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS`|`0x00700010`|
|`ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS`|`0x00700011`|
|`ELEMENT_DESTROY`|`0x00700012`|
|`NONE`|`0xFFFFFFFF`|

## `core` — низкий уровень (raw)

|Функция|Описание|
|-------|--------|
|`core.initialize()`|Инициализация shared memory (0 = успех)|
|`core.deinitialize()`|Завершение|
|`core.synchronize()`|Синхронизация|
|`core.push(code, payload)`|Отправить команду без ожидания|
|`core.push_and_wait(code, payload)`|Команда с ожиданием → `code, data`|
|`core.poll_event()`|Событие → `header, data` или `nil`|
|`core.mock_emit(header, payload)`|Положить событие в очередь mock|
|`core.sleep(ms)`|Пауза|
|`core.millis()`|Миллисекунды|
|`core.dll_path()`|Путь к загруженной pse-dll (или `(not found)`)|
|`core.loaded()`|Загружен ли мост|
|`core.mock()`|Режим mock?|
|`core.set_mock(b)`|Включить/выключить mock|

## `PSE.Core` — Lua-обёртка

|Функция|Описание|
|-------|--------|
|`PSE.Core.call(command, in, out, opts)`|Команда с ожиданием результата|
|`PSE.Core.push(command, in)`|Команда без ожидания|
|`PSE.Core.poll()` / `PSE.Core.pollAll()`|События из очереди|
|`PSE.Core.registerCallback(fn)`|Зарегистрировать callback → id|
|`PSE.Core.releaseCallback(id)`|Удалить callback|
|`PSE.Core.invokeCallback(id, guid, state)`|Вызвать callback вручную|
|`PSE.Core.synchronize()` / `PSE.Core.millis()`|Синхронизация / время|
|`PSE.Core.initialize()` / `PSE.Core.deinitialize()`|Сессия|
|`PSE.Core.Result` / `PSE.Core.Event` / `PSE.Core.Command`|Перечисления|
|`PSE.Core.result(v)` / `PSE.Core.event(v)` / `PSE.Core.command(v)`|Резолверы имён|
|`PSE.Core.byValue(map, value)`|Имя по значению|

## `Logger`

|Метод|Описание|
|-----|--------|
|`Logger.info(tag, fmt, ...)`|Информация|
|`Logger.debug(...)`|Отладка|
|`Logger.warn(...)` / `Logger.warning(...)`|Предупреждение|
|`Logger.error(...)`|Ошибка|
|`Logger.critical(...)`|Критическая ошибка|
|`Logger.setLevel(level)` / `setTag(tag)` / `getTag()`|Настройки|
|`Logger.setFile(path)` / `close()`|Запись в файл|
|`Logger.setColors(b)`|ANSI-цвета вкл/выкл|
|`Logger.paint(text, color, style)`|Цветной текст `&CD..&..&R&`|
|`Logger.color(c)`|Код ANSI-цвета (имя или число)|
|`Logger.Levels` / `Logger.Palette`|Константы|

## `PSE.Registers` — регистры

|Метод|Описание|
|-----|--------|
|`packRgb(r, g, b)` / `unpackRgb(v)`|Цвет ↔ `0xRRGGBB`|
|`getBit` / `setBit` / `getBits` / `setBits`|Бит-операции|
|`LAYOUT`|Раскладки регистров по классам|
|`build(class, regIndex, fields)`|Сборка регистра из полей|
|`extract(class, regIndex, reg)`|Разбор регистра в поля|
|`COLORS`|Именованные цвета (см. `PSE.palette`)|

Подробнее — в [Регистры](/docs/registers.md).

## Запуск

```
bin\pse_lua.exe [--mock] [script.lua]
```

- Без аргументов — REPL (`help`, `exit`, Tab-дополнение, многострочные билдеры).
- `script.lua` — выполнение скрипта.
- `--mock` — офлайн-режим (без игры).
- `PSE.initialize()` — подключение к игре; `core.set_mock(true)` — ручной mock.

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/registers.md">← Назад: Регистры</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/README.md">Далее: README →</a>
    </td>
  </tr>
</table>
</div>
