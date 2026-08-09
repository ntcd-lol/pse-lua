<div align="center">
  <table width="100%">
    <tr>
      <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="50"/></td>
      <td align="right"><h3>Документация → Инициализация</h3></td>
    </tr>
  </table>
</div>

---

## REPL

### Что это:

> **REPL - метод**, при котором запускается интерпритатор, где каждая команда исполняется сразу же, а не обычный запуск скрипта 

### Как запустить и использовать:

Все очень просто. Достаточно запустить PSE SDK Lua без скрипта:

```bash
./bin/pse-lua 
# Или в режиме MOCK:
./bin/pse-sdk --mock
```

Терминал в ответ вас попривествует:

```bash
  [bridge] pse.dll loaded: path/to/pse.dll
:09:15:58:08.961 [INFO] (PSE) >>> PSE SDK Lua 0.1.0 loaded (pse.dll: path/to/pse.dll)

  .-=:[ PSE SDK Lua ]:=-.   simplified PSE SDK + interpreter
  version 0.1.0   (embedded Lua 5.4, pse.dll via dynamic load)
  type 'help' for commands, 'exit' to quit

pse>
```

В строку `pse>` вводите команды, они будут сразу исполняться, а при создании объекта, ввод будет ожидать того как вы допишите до команды `:create()`:

```bash
pse> local door = PSE.createDoor()
...     :setPosition(0, 0, 0)
...     :name("Door_1")
...     :create()
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
      <a href="/docs/game/index.md">Далее: Работа с игрой →</a>
    </td>
  </tr>
</table>
</div>