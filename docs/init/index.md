<table width="100%">
    <thead>
        <tr>
            <td align="left"><img src="/docs/content/cube.png" alt="Cube" width="120"/></td>
            <td align="right" style="white-space: nowrap;" width="100%">Документация -> Инициализация</td>
        </tr>
    </thead>
</table>

---

## Инициализация

### `PSE.bootstrap()`

Пожалуй, самый **лучший** способ. Он активирует PSE SDK и Shared Memory, а после сразу же делает запрос к игре. На этом его работа заканчивается. И в скрипте просто в начале:

```lua
PSE.bootstrap() --- или, pse.bootstrap() - без разницы.
```

### `PSE.initialize()`

Инициализирует ТОЛЬКО PSE SDK и Shared Memory, но не пробуждает игру. Использовать с `PSE.initializeGame()`.

```lua
PSE.initialize()
PSE.initializeGame()
```

## Отключение

### `PSE.deinitialize()`

Несмотря на то, что даже прервав код с помощью Ctrl+C с игрой ничего страшного не произойдет, но Shared Memory может остаться не закрытой, из-за чего лучше использовать стандартный метод `PSE.deinitialize()`.