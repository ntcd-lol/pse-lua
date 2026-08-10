<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Инициализация</h3></td>
    </tr>
  </table>
</div>

---

## Инициализация

### `PSE.initialize()`

Активирует PSE SDK и Shared Memory, после чего подключается к запущенной игре. Если игра не отвечает — режим автоматически переключается в MOCK-режим*, поэтому скрипт продолжит работу офлайн:

```lua
local live = PSE.initialize()
Logger.info("MAIN", "session mode: %s", live and "LIVE" or "MOCK")
```

Возвращает `true` при подключении к игре и `false`, когда включён MOCK-режим.

### `PSE.game:initialize()`

После `PSE.initialize()` будит игру — перемещает игрока на SDK-уровень:

```lua
PSE.initialize()
PSE.game:initialize()
```

## Отключение

### `PSE.deinitialize()`

Даже если прервать код с помощью Ctrl+C, с игрой ничего страшного не произойдёт, но Shared Memory может остаться открытой. Используйте стандартный метод `PSE.deinitialize()`, а перед ним — `PSE.game:deinitialize()`, чтобы вернуться в главное меню:

```lua
PSE.game:deinitialize()
PSE.deinitialize()
```

_* - см. в [Инициализация -> MOCK-режим](/docs/init/mock.md)_

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/README.md">← Назад: README</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/init/mock.md">Далее: MOCK-режим →</a>
    </td>
  </tr>
</table>
</div>
