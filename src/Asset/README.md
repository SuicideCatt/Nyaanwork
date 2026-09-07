# Nyaanwork.Asset

## Kconfig
- ``CONFIG_NYAANWORK_ASSET``
- ``CONFIG_NYAANWORK_ASSET_IMAGE``
- ``CONFIG_NYAANWORK_ASSET_TILE``
- ``CONFIG_NYAANWORK_TESTS_ASSET``

## CMake variables
- ``NYAANWORK_ASSET``
- ``NYAANWORK_ASSET_IMAGE``
- ``NYAANWORK_ASSET_TILE``

## Depends
### Nyaanwork modules
- Core

## C++ modules
- ``Nyaanwork.Asset.Exception``
- ``Nyaanwork.Asset.Image`` if ``NYAANWORK_ASSET_IMAGE``
- ``Nyaanwork.Asset.Tile`` if ``NYAANWORK_ASSET_IMAGE`` and ``NYAANWORK_ASSET_TILE``

## Defines
All defines generates from CMake variables

File: ``Nyaanwork/Asset.hpp``
- ``NYAANWORK_ASSET``
- ``NYAANWORK_ASSET_IMAGE``
- ``NYAANWORK_ASSET_TILE``
