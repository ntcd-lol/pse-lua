<h1 align="center">PSE SDK Lua</h1>

![PSE SDK Lua](docs/content/banner.png)

---

> [!NOTE]
> Это обвязка Fluent API поверх оригинального [PSE SDK](https://github.com/RootTool0/pse-sdk), чтобы облегчить создание уровней и логики. Используется Lua 5.4 Embedded.

> [!IMPORTANT]
> PSE SDK Lua еще стабилен не полностью, будет приятно ждать issue!
### Сборка

**Windows** (MSVC Build Tools (желательно 2022) (Desktop development with C++)):
```bat
powershell -ExecutionPolicy Bypass -File build.ps1
```

**Linux/MacOS**:

```bash
cmake -S . -B build -A x64
cmake --build build --config Release
```

### Документация

 - [Инициализация](/docs/init/index.md)
   - [MOCK-режим](/docs/init/mock.md)
   - REPL
 - Работа с игрой
 - Объект Player
 - Создание сторонних объектов
 - События
 - Регистры

[Далее: Инициализация ->](/docs/init/index.md)