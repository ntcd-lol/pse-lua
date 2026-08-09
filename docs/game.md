<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Управление игрой</h3></td>
    </tr>
  </table>
</div>

---

## Управление игрой

Весь функционал управления игрой доступен через fluent-обёртки **`PSE.game`** и **`PSE.player`** — цепочки методов, которые возвращают сами себя:

```lua
PSE.game:setCheatsEnabled(true):setGravity(-5)
```

## Читы

### `PSE.game:setCheatsEnabled(b)`

Включает/выключает чит-режим игры:

```lua
PSE.game:setCheatsEnabled(true)  --- включить
PSE.game:setCheatsEnabled(false) --- выключить
```

### `PSE.game:getCheatsEnabled()`

Возвращает текущее состояние чит-режима (`true`/`false`):

```lua
if PSE.game:getCheatsEnabled() then
    Logger.info("MAIN", "cheats are ON")
end
```

### `PSE.game:setNoclip(b)`

Включает/выключает ноуклип (полёт сквозь стены). Работает только при включённом чит-режиме:

```lua
PSE.game:setCheatsEnabled(true)
PSE.game:setNoclip(true)
```

### `PSE.game:getNoclip()`

Возвращает состояние ноуклипа (`true`/`false`).

## Гравитация

### `PSE.game:setGravity(g)`

Устанавливает значение гравитации:

```lua
PSE.game:setGravity(0)    --- невесомость
PSE.game:setGravity(-9.8) --- стандартная
```

### `PSE.game:getGravity()`

Возвращает текущее значение гравитации:

```lua
local g = PSE.game:getGravity()
Logger.info("MAIN", "gravity = %.1f", g)
```

## Игрок

### `PSE.player:setPosition(x, y, z)`

Телепортирует игрока в указанную точку:

```lua
PSE.player:setPosition(0, 0, 0)
```

### `PSE.player:getPosition()`

Возвращает позицию игрока как `{ x, y, z }`:

```lua
local pos = PSE.player:getPosition()
Logger.info("MAIN", "player at %.1f %.1f %.1f", pos[1], pos[2], pos[3])
```

### `PSE.player:setRotation(x, y, z, w)`

Задаёт поворот камеры игрока кватернионом. Удобно строить через `PSE.deg(pitch, yaw, roll)`:

```lua
PSE.player:setRotation(PSE.deg(0, 90, 0))
```

### `PSE.player:getRotation()`

Возвращает поворот камеры игрока как `{ x, y, z, w }` (кватернион).

### `PSE.player:spawn()`

Возрождает игрока в точке спавна.

### `PSE.player:kill()`

Убивает игрока.

## Fluent-цепочки

Методы можно объединять в цепочку — каждый возвращает себя, поэтому порядок не важен:

```lua
PSE.game
    :setCheatsEnabled(true)
    :setNoclip(true)
    :setGravity(-5)

PSE.player
    :setPosition(0, 0, 0)
    :setRotation(PSE.deg(0, 90, 0))
    :spawn()
```

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/init/repl.md">← Назад: REPL</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/objects/index.md">Далее: Объекты →</a>
    </td>
  </tr>
</table>
</div>
