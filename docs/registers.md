<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Регистры</h3></td>
    </tr>
  </table>
</div>

---

## Регистры

У каждого элемента есть **8 регистров** (индексы `0`-`7`), каждый — 32-битное число (`u32`). Регистры хранят настройки элемента: цвета, флаги, размеры, таймеры и т.д.

Внутри одного регистра могут жить несколько значений — по битам или по полубайтам.

## Методы элемента

### `Element:setRegister(i, v)` / `Element:getRegister(i)`

Устанавливает/получает один регистр.

```lua
laser:setRegister(1, PSE.color(255, 0, 0))
local v = laser:getRegister(1)
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|`i`|Индекс регистра `0`-`7`|`1`|
|`v`|32-битное значение|`16711680`|

### `Element:setRegisters(t)` / `Element:getRegisters()`

Устанавливает/получает несколько регистров сразу.

```lua
laser:setRegisters({ 0, 16711680, 0 })
local regs = laser:getRegisters() --- {r0, r1, r2, ...}
```

### Команды цепочки

```lua
PSE.createLaserTx()
    :register(1, PSE.color(255, 128, 0)) --- Один регистр.
    :registers({ 0, 255 })               --- Несколько сразу.
    :create()
```

## Цвета

Цвет кодируется в битах `8..23` регистра как `RRGGBB` (по 8 бит на канал).

```lua
--- 0xRRGGBB
local v = PSE.color(255, 128, 0) --- Оранжевый.
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|`r`|Красный канал `0`-`255`|`255`|
|`g`|Зелёный канал `0`-`255`|`128`|
|`b`|Синий канал `0`-`255`|`0`|

### `PSE.Registers.packRgb(r, g, b)` / `PSE.Registers.unpackRgb(v)`

То же, что `PSE.color()`, и обратная операция.

```lua
local r, g, b = PSE.Registers.unpackRgb(laser:getRegister(1))
```

### `PSE.palette` — Готовые цвета

```lua
PSE.palette.white
PSE.palette.black
PSE.palette.orange
PSE.palette.blue
PSE.palette.red
PSE.palette.green
PSE.palette.yellow
PSE.palette.pink
PSE.palette.purple
```

```lua
laser:setRegister(1, PSE.palette.orange)
```

## Бит-операции

`PSE.Registers` предоставляет удобные бит-операции.

```lua
local reg = 0
reg = PSE.Registers.setBit(reg, 0, 1)        --- Бит 0 = 1.
local bit0 = PSE.Registers.getBit(reg, 0)    --- 1.
reg = PSE.Registers.setBits(reg, 8, 8, 255)  --- 8 бит с 8-го по 15-й.
local val = PSE.Registers.getBits(reg, 8, 8) --- 255.
```

|Метод|Обозначение|
|-----|-----------|
|`getBit(reg, offset)`|Получить 1 бит с `offset`|
|`setBit(reg, offset, value)`|Установить 1 бит с `offset`|
|`getBits(reg, offset, count)`|Получить `count` бит с `offset`|
|`setBits(reg, offset, count, value)`|Установить `count` бит с `offset`|

## Раскладки по классам

Для большинства классов регистры расшифрованы в `PSE.Registers.LAYOUT`.

### Лазеры

**`LASER_TX`**:

|Регистр|Поля|Обозначение|
|-------|----|-----------|
|`0`|`Shifted` (бит 0)|Лазер со сдвигом|
|`1`|`laserColor`|Цвет лазера (RGB)|

**`LASER_RX`**:

|Регистр|Поля|Обозначение|
|-------|----|-----------|
|`0`|`Shifted` (бит 0)|Лазер со сдвигом|
|`1`|`colorActivated`|Цвет при активации|
|`2`|`colorDeactivated`|Цвет при деактивации|
|`3`|`V` (бит 24), `R` (23), `G` (15), `B` (7)|Флаг + цвет|

### Двери

**`DOOR`**:

|Регистр|Поля|Обозначение|
|-------|----|-----------|
|`0`|`C` (бит 0), `A` (бит 1)|Флаги двери|
|`1`|`colorActivated`|Цвет при открытии|
|`2`|`colorDeactivated`|Цвет при закрытии|

### Окна

**`WINDOW`**:

|Регистр|Поля|Обозначение|
|-------|----|-----------|
|`0`|`Material` (16), `CornerBR/BL/TR/TL`, `EdgeR/L/B/T`, `Center`, `PassSolver`, `PassLaser`|Материал и части окна|
|`1`|`X`, `Y`|Размер|

### Остальные классы

Полный список раскладок — в `PSE.Registers.LAYOUT`: `BUTTON`, `ANTI_EXPROPRIATION_FIELD`, `PEDESTAL_BUTTON`, `SOLVER_BUTTON`, `INDICATOR`, `LASER_RELAY`, `LASER_PANEL`, `WEIGHT_CUBE`, `FAITH_PLATE`, `PANEL`, `STAIRS`, `CUBE_DROPPER`, `WINDOW`, `TRIGGER`, `LAMP`. Для классов без полей (`FLASHLIGHT`, `LASER_CUBE`, `SOLVER_GUN_PEDESTAL`) раскладка пустая. `ENTRY_ELEVATOR` и `EXIT_ELEVATOR` наследуют раскладку `DOOR`.

## Сборка и разбор

### `PSE.Registers.build(class, regIndex, fields)`

Собирает значение регистра из полей.

```lua
local v = PSE.Registers.build("LASER_TX", 1, { rgb = PSE.color(255, 0, 0) })
laser:setRegister(1, v)
```

### `PSE.Registers.extract(class, regIndex, reg)`

Разбирает регистр обратно в поля.

```lua
local fields = PSE.Registers.extract("WINDOW", 0, window:getRegister(0))
print(fields.Center, fields.PassLaser)
```

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/events.md">← Назад: События</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/api.md">Далее: Сводка API →</a>
    </td>
  </tr>
</table>
</div>
