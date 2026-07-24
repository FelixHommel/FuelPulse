# ```FuelModule```

This document is supposed to detail the architecture and design choices of the ```FuelModule```.

## System Architecture

An overview of the entire ```FuelModule``` and how it connects to the Application itself represented by a class diagram.
The module is designed to implement the provided interface definitions from the core, to ensure generic compatibility.

```mermaid
classDiagram
  namespace Core {
    class Application{
      +registerModule(unique_ptr~IModule~)
      +run()
    }

    class Scheduler
    class EventBus
  }

  namespace Interfaces {
    class IModule {
     <<interface>>

     +registerWith(Application&)
    }

    class ICollector {
      <<interface>>

      +collect()
    }

    class IReport {
      <<interface>>

      +report()
    }

    class IAnalysis {
      <<interface>>

      +analyze()
    }
  }

  namespace FuelModuleImpl {
    class FuelModule {
      -FuelModuleConfig config
      -unique_ptr~IFuelRepository~ repository
      -unique_ptr~ICollector~ collector
      -unique_ptr~IAnalysis~ analyzer

      +registerWith(Application&)
    }

    class IFuelRepository {
      <<interface>>

      +store(Measurement)
      +storeStations(List~Station~)
      +loadStations() List~Station~
      +loadMeasurements(StationId, TimePoint from, TimePoint to) List~Measurement~
    }

    class SqliteFuelRepository {
      +store(Measurement)
      +storeStations(List~Station~)
      +loadStations() List~Station~
      +loadMeasurements(StationId, TimePoint from, TimePoint to) List~Measurement~
    }

    class ConfigLoader {
      +load(argc, argv) FuelModuleConfig
      +loadApiKeyFromEnv() string
    }

    class StationDiscoveryService {
      -string apiKey
      -size_t maxStations
    }

    class TankerKoenigCollector {
      -string apiKey
      -IFuelRepository& repository
      -List~Station~ stations

      +collect()
    }

    class JsonReport {
      -Path outputDir
      -List~StationAnalysis~ analyses

      +report()
    }

    class DailyAnalyzer {
      -IFuelRepository& repository
      -List~StationAnalysis~ lastResult

      +analyze()
    }

    class Station {
      +string id
      +string name
      +double latitude
      +double longitude
    }

    class Measurement {
      +string stationId
      +time_point timestamp
      +optional~int32~ e5
      +optional~int32~ e10
      +optional~int32~ diesel
    }

    class StationAnalysis {
      +BasicStats basic
      +AdvancedStats advanced
      +CheapestWindow cheapestWindow
    }
  }

  IModule <|.. FuelModule
  ICollector <|.. TankerKoenigCollector
  IFuelRepository <|.. SqliteFuelRepository
  IReport <|.. JsonReport
  IAnalysis <|.. DailyAnalyzer

  Application <.. FuelModule : registerWith

  FuelModule --> ConfigLoader
  FuelModule --> StationDiscoveryService
  FuelModule --> TankerKoenigCollector
  FuelModule --> JsonReport
  FuelModule --> DailyAnalyzer
  FuelModule --> IFuelRepository

  StationDiscoveryService --> Station

  TankerKoenigCollector --> IFuelRepository
  TankerKoenigCollector --> Measurement

  JsonReport --> StationAnalysis

  DailyAnalyzer --> StationAnalysis
  DailyAnalyzer --> IFuelRepository
```

## Runtime Diagrams

This section is supposed to detail the control flow within a module.

### Startup Sequence

This sequence diagram shows what the module goes through once it was started. It needs to load the configuration, start
scheduling recurring tasks, and subscribe to relevant events.

```mermaid
sequenceDiagram
  participant Application
  participant FuelModule
  participant ConfigLoader
  participant StationDiscoveryService
  participant SQLiteFuelRepository
  participant Scheduler
  participant EventBus

  Application ->> FuelModule: registerWith(app)

  FuelModule ->> ConfigLoader: load(argc, argv)
  ConfigLoader -->> FuelModule: FuelModuleConfig

  FuelModule ->> SQLiteFuelRepository: loadStations()
  alt no stations persisting yet
    FuelModule ->> StationDiscoveryService: discover(postalCode, radiusKm)
    StationDiscoveryService -->> FuelModule: List<Station>

    FuelModule ->> SQLiteFuelRepository: storeStations(stations)
  end

  FuelModule ->> Scheduler: scheduleEvery(interval, CollectionTick)

  FuelModule ->> EventBus: subscribe(DailyAnalysisRequested)
```

### Date Request

```mermaid
sequenceDiagram
  participant Scheduler
  participant EventBus
  participant TankerKoenigCollector
  participant TankerKoenig API
  participant SQLiteFuelRepository

  Scheduler ->> EventBus: CollectionTick

  EventBus ->> TankerKoenigCollector: collect()

  TankerKoenigCollector ->> TankerKoenig API: GET prices.php
  TankerKoenig API -->> TankerKoenigCollector: JSON

  TankerKoenigCollector ->> SQLiteFuelRepository: store(measurement) x N
```

### Analyzing Flow

This sequence diagram details the flow when the daily analyzing is triggered.

```mermaid
sequenceDiagram
  participant Scheduler
  participant EventBus
  participant DailyAnalyzer
  participant SQLiteFuelRepository
  participant JsonReport
  
  Scheduler ->> EventBus: DailyAnalysisRequest

  EventBus ->> DailyAnalyzer: analyze()

  DailyAnalyzer ->> SQLiteFuelRepository: loadMeasurements(station, yesterday)
  SQLiteFuelRepository -->> DailyAnalyzer: vector<Measurement>

  DailyAnalyzer ->> DailyAnalyzer: compute statistics

  DailyAnalyzer ->> JsonReport: report(analysis)

  JsonReport ->> JsonReport: write to disk
```
