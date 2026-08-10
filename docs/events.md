<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → События</h3></td>
    </tr>
  </table>
</div>

---

## События

События позволяют реагировать на изменения объектов в мире, не опрашивая их вручную.

### Типы событий

|Событие|Обозначение|
|---|---|
|`ELEMENT_CHANGED`|Элемент изменился (state / callback)|
|`GAME_TICK_OVERFLOW`|Переполнение тика игры|

Константы доступны через `PSE.Event`:

```lua
PSE.Event.ELEMENT_CHANGED    --- 0x00000000
PSE.Event.GAME_TICK_OVERFLOW --- 0x00000001
```

## Обработчики

### `PSE.on(eventName, fn)`

Регистрирует обработчик события. Возвращает `PSE`, поэтому можно регистрировать несколько подряд.

```lua
PSE.on("ELEMENT_CHANGED", function(ev)
    Logger.info("MAIN", "element %s changed, state = %d", ev.guid, ev.state)
end)
```

|Аргумент|Обозначение|Пример|
|--------|-----------|------|
|`eventName`|Название события|`"ELEMENT_CHANGED"`|
|`fn`|Функция-обработчик `function(ev)`|`function(ev) ... end`|

### `PSE.onElementChanged(fn)`

Сокращение для `PSE.on("ELEMENT_CHANGED", fn)`.

```lua
PSE.onElementChanged(function(ev) --- То же самое, что выше.
end)
```

## Обработка очереди

### `PSE.poll()`

Забирает одно событие из очереди и вызывает его обработчики. Возвращает событие или `nil`, если очередь пуста.

```lua
local ev = PSE.poll()
if ev then
    print(ev.event)
end
```

Поля события `ELEMENT_CHANGED`:

|Поле|Обозначение|Пример|
|----|-----------|------|
|`event`|Название события|`"ELEMENT_CHANGED"`|
|`guid`|GUID элемента|`18446744073709551615`|
|`state`|Новое состояние|`1`|

### `PSE.pollAll()`

Забирает все события из очереди и возвращает их списком.

```lua
local events = PSE.pollAll()
for _, ev in ipairs(events) do
    Logger.info("MAIN", "%s: %s", ev.event, tostring(ev.guid))
end
```

### `PSE.run(duration)`

Запускает цикл событий. Секунды — если `nil`, работает бесконечно.

```lua
PSE.run(10) --- Обрабатывать события 10 секунд.
```

## Callback объекта

### `Element:setCallback(fn)`

Привязывает callback к конкретному объекту. Вызывается автоматически при `PSE.poll()`, когда приходит событие с его `callbackId`.

```lua
local laser = PSE.createLaserRx()
    :onChange(function(ev) --- В цепочке.
        Logger.info("MAIN", "laser changed")
    end)
    :create()

--- Или после создания:
laser:setCallback(function(state, guid)
    Logger.info("MAIN", "laser state = %d", state)
end)
```

> [!TIP]
> `setCallback` заменит callback, заданный через `onChange` в цепочке — используйте что-то одно.

## Имитация в MOCK

> [!IMPORTANT]
> События генерирует игра. В MOCK-режиме их можно имитировать вручную.

### `PSE.mock.emit(target, state)`

Имитирует `ELEMENT_CHANGED` для объекта по имени или GUID.

```lua
PSE.initialize()
local laser = PSE.createLaserTx():name("Laser_1"):create()

PSE.onElementChanged(function(ev)
    Logger.info("MAIN", "mock event: %s -> %d", ev.guid, ev.state)
end)

PSE.mock.emit("Laser_1", 1)
PSE.pollAll() --- Обработать.
```

### `PSE.mock.emitRaw(guid, callbackId, state)`

То же самое, но напрямую по GUID и `callbackId`.

```lua
PSE.mock.emitRaw(1, 0, 1)
```

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/objects/third.md">← Назад: Сторонние объекты</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/registers.md">Далее: Регистры →</a>
    </td>
  </tr>
</table>
</div>
