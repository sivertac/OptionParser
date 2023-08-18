# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Optional Required Components, parser will validate if required components are present. ()
    - `isComplete()` member function in `ParseContext` will return `false` if required components are missing for the current `ParseResult`.

## [v0.3.1] - 2023-8-17
### Fixed
- Fixed wrong suggestions when all parameters of a component has been consumed if the parameters has a custom suggestions function, now parameters will only be suggested if there are space for more parameters in the result. (https://github.com/sivertac/OptionParser/pull/11)

## [v0.3.0] - 2023-8-17
### Added
- Added `ParseContext` to `optionparser_v2`. (https://github.com/sivertac/OptionParser/pull/8)
    - `ParseContext` is used to store the state of the parser, allowing tokens to be fed to the parser one at a time.

### Changed
- Changed stored value type in `ParseResult` from `std::string_view` to `std::string` in `optionparser_v2`. (https://github.com/sivertac/OptionParser/pull/9)
    - This is to avoid dangling references to the original input string.

## [v0.2.1]
### Fixed
- Set correct project version in `CMakeLists.txt`.

## [v0.2.0] - 2023-8-15 [YANKED]
### Added
- Added `nextTokenSuggestionsMulti` function to `optionparser_v2`. (https://github.com/sivertac/OptionParser/pull/2)
- Added `CHANGELOG.md` file.

### Changed
- Changed name of `PositionalIdentifier` to `Command` in `optionparser_v2`. (https://github.com/sivertac/OptionParser/pull/3)


## [v0.1.0] - 2023-8-13
### Added
- Added `optionparser_v2`
    - Runtime capable parser.
    - Supports parse suggestions that can be used for tab completion.
    - Everything is new, nothing is shared with legacy `OptionParser`.
### Changed
- Tests now use `google-test` instead of `doctest`.


## [v0.0.1] - 2020-10-31
- Legacy `OptionParser`.
