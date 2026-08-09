<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Инициализация → MOCK-режим</h3></td>
    </tr>
  </table>
</div>

---

## MOCK-режим

## Что это:

MOCK-mode - режим симуляции игры, для быстрого тестирования кода и логики, без запуска игры на большом Unreal Engine 5.5.

### Как запустить PSE SDK Lua в этом режиме?

Используйте во время запуска скрипта флаг `--mock`:

```bash
./bin/pse-lua ./examples/init.lua --mock

# Или просто для REPL*:
./bin/pse-lua --mock
```

## Доп. команды в режиме MOCK

### PSE.mock.emit(name, state)

Самый простой способ симуляции.

Использование:

|Аргумент|Обозначение|Пример|
|---|---|---|
|`name`|Название объекта|`"my_button"`|
|`state`|Состояние|`1`|

```lua
PSE.mock.emit("my_button", 1) --- Активация кнопки
```

### PSE.mock.emitRaw(guid, callbackid, state)

Низкоуровневый вариант. Используй, когда нужно работать напрямую с GUID и Callback ID.

|Аргумент|Обозначение|Пример|
|---|---|---|
|`guid`|GUID объекта, который можно получить используя `obj.guid`, где на месте obj - переменная объекта|`btn.guid`|
|`callbackid`|Callback ID объекта, который можно получить так же используя `obj.callbackid`, где вместо obj - переменная объекта|`btn.callbackid`|
|`state`|Состояние|`1`|

_* - см. в [Инициализация -> REPL](/docs/init/repl.md)_

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/init/index.md">← Назад: Инициализация</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/init/repl.md">Далее: REPL →</a>
    </td>
  </tr>
</table>
</div>