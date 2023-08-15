# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
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
