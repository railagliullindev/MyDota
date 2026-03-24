# Replication Graph — Оптимизация репликаций и защита от читеров

<a name="begin"></a>

## С чего всё начиналось

Была стандартная репликация с `IsNetRelevantFor`. Всё работало, но была одна беда: когда враг заходил за дерево, он продолжал светиться на экране ещё 3-4 секунды.  

Всё работало, но была одна проблема — в мультиплеерных играх всегда есть риск, 
что читеры смогут получить данные о том, что им не положено знать. 
В MOBA это особенно критично: позиции врагов, их HP, мана, способности — 
всё это даёт нечестное преимущество.

Мне нужно было решить сложную задачу:

- **Свои герои** — всегда видны
- **Вражеские герои** — только если их видно через туман войны
- **Туман войны** — свой для каждой команды
- **Камера игрока** — вообще не нужна другим

---

## Содержание
> - [С чего всё начиналось](#begin)
> - [Replication Graph — что это и как я его использую](#replication-graph)
> - [Особенность FogManager](#fog-manager-feature)
> - [Как реплицируются герои](#hero-replicates)
> - [Структура узлов ReplicationGraph](#replication-graph-structure)
> - [Последовательность действий](#sequence-of-actions)
> - [Ключевые моменты](#key-points)
> - [Чего я добился?](#what-i-have-achieved)

---

<a name="replication-graph"></a>
## Replication Graph — что это и как я его использую

[MD_ReplicationGraph.h](../Source/MyDota/Public/Network/MD_ReplicationGraph.h)  
[MD_ReplicationGraph.cpp](../Source/MyDota/Private/Network/MD_ReplicationGraph.cpp)

**Replication Graph** — это низкоуровневый механизм Unreal Engine,
который позволяет контролировать репликацию каждый кадр. 
В отличие от стандартной системы, где решение "реплицировать или нет" 
принимается раз в несколько секунд, здесь мы можем фильтровать данные мгновенно.

Я построил архитектуру, где каждый клиент получает только ту информацию,
которую должен видеть именно сейчас:

- **Глобальные данные** (`GameState`, `PlayerState`) — получают все
- **Своя команда** (герои, их состояния) — всегда
- **Вражеская команда** — только через туман войны

---

**СЕРВЕР**  
├── FogManager (Radiant, TeamID=1) — лежит на сцене  
├── FogManager (Dire, TeamID=2) — лежит на сцене  
│  
├── ConnectionManager (Radiant игрок)  
│ ├── FogNode → хранит FogManager(Radiant)  
│ └── TeamConnectionNode → герои Radiant  
│  
├── ConnectionManager (Dire игрок)  
│ ├── FogNode → хранит FogManager(Dire)  
│ └── TeamConnectionNode → герои Dire  
│  
└── Герои (Radiant и Dire) — привязаны к своим ConnectionManager

**РЕПЛИКАЦИЯ**  
├── Свои герои → всегда  
├── Враги → только если IsCellVisible() = true  
└── FogManager → только своей команде

---

<a name="fog-manager-feature"></a>

## Особенность FogManager

[FogOfManager](FogOfWar.md) — это актор, который хранит карту тумана войны для своей команды. 
Он должен быть только у тех, кому положен. 
Если бы он попал к врагу, тот бы видел всю карту (читерство).

Поэтому я сделал отдельный узел репликации `UReplicationGraphNode_FogOfWarManager`, 
который добавляет `FogManager` только для соединений своей команды. 
Даже если на него есть ссылки из других объектов, он не уедет к врагу.

---

**КЛИЕНТ (Radiant)**  
├── FogManager (Radiant) ✓  
└── FogManager (Dire) ✗  

**КЛИЕНТ (Dire)**  
├── FogManager (Dire) ✓  
└── FogManager (Radiant) ✗  

---

<a name="hero-replicates"></a>

## Как реплицируются герои

**Герои** — это отдельные акторы, не привязанные напрямую к `PlayerController`.
У них есть `Owner` (контроллер игрока), но нет своего `NetConnection`.

Чтобы найти нужное соединение для героя, я построил цепочку: `герой` → `его владелец (PlayerController)` → его `NetConnection` → наш `ConnectionManager`.

Дальше работает простая логика:

- Свои герои — всегда в списке репликации
- Вражеские герои — проверяем через `FogManager->IsCellVisible()`.   
Если позиция врага видна на карте — реплицируем, нет — пропускаем

Камера игрока (`CameraPawn`) вообще не реплицируется. Даже если на неё есть ссылки, мы явно блокируем её добавление в граф репликации.

---

Каждый кадр для каждого клиента  
│  
├── 1. Собираем своих героев  
│ └── TeamConnectionNode → всегда в списке  
│  
├── 2. Собираем врагов  
│ ├── Получаем FogManager своей команды  
│ ├── Проверяем позицию врага: IsCellVisible(EnemyLocation)  
│ ├── Если true → добавляем в список  
│ └── Если false → пропускаем  
│  
└── 3. Отправляем клиенту только собранный список

---

<a name="replication-graph-structure"></a>

## Структура узлов ReplicationGraph

**UMD_ReplicationGraph**  
│  
├── **AlwaysRelevantNode**  
│   └── GameState, PlayerState → всем клиентам  
│  
├── **UMD_ConnectionManager** (на каждого игрока)  
│  
├── **AlwaysRelevantForConnectionNode**  
│   └── Акторы с bOnlyRelevantToOwner = true → только владельцу  
│  
├── **TeamConnectionNode**  
│   └── Герои своей команды → всегда  
│  
├── **FogNode**  
│   └── FogManager своей команды → только своей команде

---

<a name="sequence-of-actions"></a>

## Последовательность действий

1. **Уровень загружается**  
   └── FogManager (Radiant) и FogManager (Dire) уже на сцене

2. **Подключается Radiant игрок**  
   ├── GameMode назначает команду (Team=1)  
   ├── SetTeamForPlayerController(Team=1)  
   ├── UpdateTeamForConnection()  
   └── Ищем FogManager на сцене через TActorIterator  
   └── Нашли FogManager(Team=1) → кладём в FogNode

3. **Подключается Dire игрок**  
   ├── GameMode назначает команду (Team=2)  
   ├── SetTeamForPlayerController(Team=2)  
   ├── UpdateTeamForConnection()  
   └── Ищем FogManager на сцене через TActorIterator  
   └── Нашли FogManager(Team=2) → кладём в FogNode

4. **Репликация**  
   ├── FogNode → отдаёт FogManager только если TeamId совпадает  
   ├── TeamConnectionNode → отдаёт героев своей команды  
   └── Враги → только если IsCellVisible() = true

---

<a name="key-points"></a>

##  Ключевые моменты

_Почему FogManager не уезжает к врагу?_

1. FogManager есть на сцене, но NetLoadOnClient = false
2. Мы не добавляем его в RouteAddNetworkActorToNodes
3. При назначении команды ищем его через TActorIterator
4. Кладём только в FogNode своего ConnectionManager
5. FogNode отдаёт FogManager только если TeamId совпадает
6. Вражеский ConnectionManager не получит чужой FogManager

---

<a name="what-i-have-achieved"></a>

## Чего я добилися

1. **Защита от читеров** — клиент физически не получает данные о том, что не должен видеть. 
Даже если читер перехватит сетевой трафик, там не будет информации о врагах вне зоны видимости.

2. **Мгновенная реакция** — как только враг скрылся в тумане, он перестаёт реплицироваться в следующем кадре.
Никаких задержек.

3. **Экономия трафика** — клиент получает только то, что действительно нужно для отображения.

4. **Масштабируемость** — легко добавить новые типы акторов (башни, крипы, руны) с нужными правилами видимости.





 