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

### Что это:

MOCK-режим - режим симуляции игры для быстрого тестирования кода и логики, без запуска игры на большом Unreal Engine 5.5.

### Как запустить PSE SDK Lua в этом режиме:

Используйте флаг `--mock` перед запуском скрипта:

```cmd
bin\pse_lua.exe --mock examples\init.lua

# Или просто для REPL*:
bin\pse_lua.exe --mock
```

Если игра не запущена, MOCK-режим включается автоматически — `PSE.initialize()` вернёт `false`, и скрипт продолжит работу офлайн.

## Доп. команды в режиме MOCK

### PSE.mock.emit(name, state)

Самый простой способ симуляции.

|Аргумент|Обозначение|Пример|
|---|---|---|
|`name`|Название объекта|`"my_button"`|
|`state`|Состояние|`1`|

```lua
PSE.mock.emit("my_button", 1) --- Активация кнопки
```

### PSE.mock.emitRaw(guid, callbackId, state)

Низкоуровневый вариант. Используй, когда нужно работать напрямую с GUID и Callback ID.

|Аргумент|Обозначение|Пример|
|---|---|---|
|`guid`|GUID объекта, который можно получить используя `obj.guid`, где на месте obj - переменная объекта|`btn.guid`|
|`callbackId`|Callback ID объекта, который можно получить используя `obj.callbackId`|`btn.callbackId`|
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
