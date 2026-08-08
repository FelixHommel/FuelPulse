# FuelPulse

![Build](https://github.com/FelixHommel/FuelPulse/actions/workflows/build.yml/badge.svg)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2bb4b705edb946539e555f2f78e86446)](https://app.codacy.com/gh/FelixHommel/FuelPulse/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

## Motivation

Given the recent fuel prices in Germany (i.e., 2.30 Euros for one Liter of Diesel on 7/14/2026) I was wondering at which
one of my local gas stations and when I should go to refuel. That got me thinking: Why don't I make something that can
track the gas prices during the day and then performs some sort of statistical analysis on the data.

## Architecture

To see an overview of the architecture see [this](./docs/architecture/Architecture.md).

## Building

### With CMake Workflow

> In the following ```<platform>``` can be any of ```gcc```, ```clang```, and ```windows``` and ```<type>``` can be
> either ```release```, or ```debug```.

1. Run workflow
  ```bash
  cmake --workflow --preset <platform>-<type>
  ```
2. Run the Application
  ```bash
  ./build/<type>/src/app/FuelPulseApp
  ```

### With CMake Preset

1. Configure CMake
  ```bash
  cmake --preset <platform>-<type>
  ```
2. Build the Application
  ```bash
  cmake --build --preset <platform>-<type>
  ```
3. Run the Application
  ```bash
  ./build/<type>/src/app/FuelPulseApp
  ```

## Credits / Acknowledgments
