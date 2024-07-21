# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] 2024-07-24 String Values in Features

### Added

- Allow string values in features
- Library logo

### Fixed

- Fixed bug in numeric attributes states

### Removed

- Catch2 git submodule
- iostream include

## [1.0.0] 2024-05-21 Initial Release

### Added

- Read Arff Files with last or first column as target
- Read Arff Files with determined target column
- Return X with a vector of vectors of dimensions (n_features, n_samples), this way each feature is a vector
- The class values a factorized to integers (factorize method can be used to factorize any vector of strings)
- Return y as a vector of integers
- Values marked as ? are replaced with -1
- Can return the original lines read as a vector of strings
- Can return the className and the classType
- Can return the features as a vector of strings (exluding the target)
