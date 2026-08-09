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

## Читы

### `PSE.setCheatsEnabled(b)`

Включает/выключает чит-режим игры:

```lua
PSE.setCheatsEnabled(true)  --- включить
PSE.setCheatsEnabled(false) --- выключить
```

### `PSE.getCheatsEnabled()`

Возвращает текущее состояние чит-режима (`true`/`false`):

```lua
if PSE.getCheatsEnabled() then
    Logger.info("MAIN", "cheats are ON")
end
```

### `PSE.setNoclip(b)`

Включает/выключает ноуклип (полёт сквозь стены). Работает только при включённом чит-режиме:

```lua
PSE.setCheatsEnabled(true)
PSE.setNoclip(true)
```

### `PSE.getNoclip()`

Возвращает состояние ноуклипа (`true`/`false`).

## Гравитация

### `PSE.setGravity(g)`

Устанавливает значение гравитации:

```lua
PSE.setGravity(0)    --- невесомость
PSE.setGravity(-9.8) --- стандартная
```

### `PSE.getGravity()`

Возвращает текущее значение гравитации:

```lua
local g = PSE.getGravity()
Logger.info("MAIN", "gravity = %.1f", g)
```

## Игрок

### `PSE.setPlayerLocation(x, y, z)`

Телепортирует игрока в указанную точку:

```lua
PSE.setPlayerLocation(0, 0, 0)
```

### `PSE.getPlayerLocation()`

Возвращает позицию игрока как `{ x, y, z }`:

```lua
local pos = PSE.getPlayerLocation()
Logger.info("MAIN", "player at %.1f %.1f %.1f", pos[1], pos[2], pos[3])
```

### `PSE.setPlayerRotation(x, y, z, w)`

Задаёт поворот камеры игрока кватернионом. Удобно строить через `PSE.deg(pitch, yaw, roll)`:

```lua
PSE.setPlayerRotation(PSE.deg(0, 90, 0))
```

### `PSE.getPlayerRotation()`

Возвращает поворот камеры игрока как `{ x, y, z, w }` (кватернион).

### `PSE.spawnPlayer()`

Возрождает игрока в точке спавна.

### `PSE.killPlayer()`

Убивает игрока.

## Fluent-обёртки

Тот же функционал доступен в цепочечном виде через `PSE.game` и `PSE.player`:

```lua
PSE.game
    :setCheatsEnabled(true)
    :setGravity(-5)

PSE.player
    :setPosition(0, 0, 0)
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
