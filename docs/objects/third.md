<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Объекты → Сторонние</h3></td>
    </tr>
  </table>
</div>

---

## Сторонние объекты

Сторонние объекты — это все остальные объекты SDK, кроме [игрока](/docs/objects/player.md) и [игры](/docs/game.md):

- **Готовые объекты** — всегда доступны, не требуют создания: `PSE.gun`, `PSE.flashlight`.
- **Создаваемые объекты** — создаются командами вида `PSE.create...()` через fluent-цепочку.

## Готовые объекты

### Основной объект:

```lua
PSE.gun
```

### `PSE.gun:setEnabled(b)` / `PSE.gun:getEnabled()`

Включает/выключает гравитационную пушку.

```lua
PSE.gun:setEnabled(false) --- Выключить пушку.
PSE.gun:getEnabled()      --- Значение | true/false.
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|  `b`   |Включено   |`true`|

### `PSE.gun:use()` / `PSE.gun:release()` / `PSE.gun:throw()`

Действия пушки: взять, отпустить, бросить.

```lua
PSE.gun:use()
PSE.gun:release()
PSE.gun:throw()
```

### Основной объект:

```lua
PSE.flashlight
```

### `PSE.flashlight:setEnabled(b)` / `PSE.flashlight:getEnabled()`

Включает/выключает фонарик.

```lua
PSE.flashlight:setEnabled(true)
PSE.flashlight:getEnabled()
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|  `b`   |Включено   |`true`|

### `PSE.flashlight:setState(b)` / `PSE.flashlight:getState()`

Устанавливает/получает состояние фонарика.

```lua
PSE.flashlight:setState(1) --- 0 или 1.
PSE.flashlight:getState()
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|  `b`   |Состояние  |  `1` |

## Создание

Все создаваемые объекты используют fluent-цепочку: каждый метод возвращает объект, поэтому порядок не важен.

### `PSE.createMeshObject(opts)` — Меш-объект

```lua
local mesh = PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_BLACK")
    :scale(24, 24, 1)
    :position(0, 0, -1600)
    :create()
```

### `PSE.createElement(class, opts)` — Универсальное создание

Создаёт элемент любого класса.

```lua
local laser = PSE.createElement("LASER_TX", { state = 1 })
    :position(0, 0, -100)
    :create()
```

### Готовые обёртки `PSE.create...`

|Команда|Класс|
|---|---|
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

### Команды цепочки

```lua
local obj = PSE.createLaserTx()
    :name("Laser_1")                  --- Имя для PSE.get("Laser_1").
    :position(0, 0, -100)             --- Позиция {x, y, z}.
    :rotation(0, 0, 0, 1)             --- Кватернион {x, y, z, w}.
    :scale(1, 1, 1)                   --- Размеры {x, y, z}.
    :state(1)                         --- Состояние | 0 или 1.
    :visible(true)                    --- Видимость.
    :register(1, 255)                 --- Регистр | 0-7.
    :registers({ 1, 0, 0 })           --- Несколько регистров сразу.
    :onChange(function(ev) ... end)   --- Событие при изменении.
    :create()                         --- Создать в мире.
```

|Название|Обозначение|Пример|
|---|---|---|
|`name`|Название переменной, советуется уникальное чтобы делать код предсказумее.|`laser`|
|`commands`|Команды вида цепочки Lua|`name`, `position`|

### После создания доступны `set...` методы

```lua
obj:setState(1)
obj:setPosition(0, 0, 0)
obj:setDegree(0, 90, 0)
obj:setScale(1, 1, 1)
obj:setRegister(1, 255)
obj:setVisibility(false)
obj:destroy()
```

## Пример

```lua
PSE.initialize()

PSE.gun:setEnabled(false)
PSE.flashlight:setEnabled(true)

local laser = PSE.createLaserTx()
    :name("Laser_1")
    :position(0, 0, -100)
    :state(1)
    :register(1, PSE.color(255, 128, 0))
    :create()

Logger.info("MAIN", "laser guid = %s", laser.guid)
```

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/objects/player.md">← Назад: Игрок</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/events.md">Далее: События →</a>
    </td>
  </tr>
</table>
</div>
