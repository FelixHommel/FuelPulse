# Architecture

This document is intended to provide an overview of the entire system, its components, and how they communicate with
each other.

## System Overview

A rough overview of the system architecture and how the components are connected with each other. As evident, the
Event Bus is the center of the application. It is responsible to manage the communication between all the other
components.

```mermaid
flowchart LR

subgraph Core
  APP[Application]
  SCH[Scheduler]
  CMD[Command Interpreter]
  BUS[Event Bus]
  LOG[Logger]
end

subgraph Fuel Module
  COL[REST Collector]
  ANA[Analyzer]
  REP[Report Generator]
  DB[(SQLite)]
end

subgraph External
  API[TankerKoenig REST API]
  PY[Python Visualization]
end

SCH --> BUS
CMD --> BUS
APP --> BUS

BUS --> COL
BUS --> ANA

COL --> API
COL --> DB

ANA --> DB
ANA --> REP

REP --> PY
```

## System Architecture

The Architecture is designed to split the System into two parts:

1. Independent Core
2. Modular Plugin System

That way, it is easy to make the application track and record a completely different type of data (i.e., stock prices).
This architectural decision is reflected in the class diagram, the core itself is disconnected from everything and
the ```FuelModule``` itself just has to implement the predefined interfaces to make everything work together.

```mermaid
classDiagram
  namespace Core{
    class Application{
      +run()
      +shutdown()
      +registerModule()
    }

    class Scheduler{
      +schedule(Task)
    }

    class EventBus{
      +publish(Event)
      +subscribe()
    }

    class CommandRegistry{
      +registerCommand()
    }
  }

  namespace Interfaces{
    class IModule{
      <<interface>>
      +register(Application&)
    }

    class ICollector{
      <<interface>>
      +collect()
    }

    class IRepository{
      <<interface>>
      +store()
      +load()
    }

    class IAnalyzer{
      <<interface>>
      +analyze()
    }
  }

  namespace CustomModule{
    class FuelModule

    class TankerKoenigCollector

    class SQLiteRepository

    class DailyAnalyzer
  }

  Application --> Scheduler
  Application --> EventBus
  Application --> CommandRegistry

  IModule <|.. FuelModule

  ICollector <|.. TankerKoenigCollector
  IRepository <|.. SQLiteRepository
  IAnalyzer <|.. DailyAnalyzer

  FuelModule --> TankerKoenigCollector
  FuelModule --> SQLiteRepository
  FuelModule --> DailyAnalyzer
```

## System Interaction

This diagram shows the workflow of how the system collects a new set of data and saves it into persistent storage.

```mermaid
sequenceDiagram
  participant Scheduler
  participant EventBus
  participant Collector
  participant API
  participant Database

  Scheduler ->> EventBus: MinuteTick
  EventBus ->> Collector: MinuteTick
  Collector ->> API: GET /stations/postalcode
  API -->> Collector: JSON
  Collector ->> Database: Store measurements
  Collector ->> EventBus: Publish CollectionFinished
```

Here, the process of triggering the analysis event is emitted into the system which subsequently triggers the analysis
to start.

```mermaid
sequenceDiagram
  participant Scheduler
  participant EventBus
  participant Analyzer
  participant Database
  participant Reporter

  Scheduler ->> EventBus: DailyAnalysisEvent
  EventBus ->> Analyzer: AnalyzeYesterday
  Analyzer ->> Database: Load yesterday
  Database -->> Analyzer: Measurements
  Analyzer ->> Reporter: Generate Report
  Reporter -->> Analyzer: report.json
```

This diagram shows the process of a User entering a command via the CLI which then is processed by the system.

```mermaid
sequenceDiagram
  participant User
  participant CLI
  participant CommandRegistry
  participant EventBus

  User ->> CLI: start
  CLI ->> CommandRegistry: lookup("start")
  CommandRegistry ->> EventBus: Publish Start
```
