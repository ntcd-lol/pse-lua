<h1 align="center">PSE SDK Lua</h1>

![PSE SDK Lua](docs/content/banner.png)

---

### Что это:
Это обвязка Fluent API поверх оригинального [PSE SDK](https://github.com/RootTool0/pse-sdk), чтобы облегчить создание уровней и логики. Используется Lua 5.4 Embedded для того, чтобы

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

 - Подключение
   - Альтернативные методы
   - MOCK-режим
   - REPL
 - Работа с игрой
 - Объект Player
 - Создание сторонних объектов
 - События
 - Регистры
