<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Инициализация → REPL</h3></td>
    </tr>
  </table>
</div>

---

## REPL

### Что это:

> **REPL - метод**, при котором запускается интерпретатор, где каждая команда исполняется сразу же, а не обычный запуск скрипта.

### Как запустить и использовать:

Все очень просто. Достаточно запустить PSE SDK Lua без скрипта:

```cmd
bin\pse_lua.exe

# Или в режиме MOCK:
bin\pse_lua.exe --mock
```

Терминал в ответ вас поприветствует:

```
  [bridge] pse-sdk connected: third_party/pse-sdk (statically linked)

  .-=:[ PSE SDK Lua ]:=-.   simplified PSE SDK + interpreter
  version 0.1.0   (embedded Lua 5.4, pse-sdk statically linked)
  type 'help' for commands, 'exit' to quit

pse>
```

В строку `pse>` вводите команды, они исполняются сразу же. Многострочные конструкции продолжаются с приглашением `...>` и завершаются пустой строкой или командой `:create()`:

```
pse> local door = PSE.createDoor()
...>     :position(0, 0, 0)
...>     :name("Door_1")
...>     :create()
pse>
```

---

<div align="center">
<table width="100%">
  <tr>
    <td align="left" width="45%">
      <a href="/docs/init/mock.md">← Назад: MOCK-режим</a>
    </td>
    <td align="center" width="10%">
      &nbsp;
    </td>
    <td align="right" width="45%">
      <a href="/docs/game.md">Далее: Управление игрой →</a>
    </td>
  </tr>
</table>
</div>
