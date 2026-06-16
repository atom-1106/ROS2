# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

For information related to Middleware, see [README.md](README.md)

## Unreleased
_This tracks content changes that are currently on master but not yet officially released_


## 2.1.0 (2026-05-04)

### Added
- [Middleware build process uses cate_conan_validate](https://dev.azure.com/cat-ciss/G7/_workitems/edit/33642)

### Changed
- [Upgrade cate-conan-validate to 0.5.3](https://dev.azure.com/cat-ciss/G7/_workitems/edit/33641)
- **[BREAKING]** [Add can port to diagnostic protobuf](https://dev.azure.com/cat-ciss/G7/_workitems/edit/33974)
  - There is a new enum field called `can_port` added to `J1939Address` in [middleware_diagnostics.proto](https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/middleware-proto/-/blob/master/middleware_diagnostics.proto?ref_type=heads).
  - In [middleware_diagnostics.proto](https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/middleware-proto/-/blob/master/middleware_diagnostics.proto?ref_type=heads), the `UpdateReason` field has been deprecated.

### Fixed
- [Middleware: Max Payload for Read/Write is not set](https://dev.azure.com/cat-ciss/G7/_workitems/edit/33533)


## 2.0.1 (2026-04-08)

### Fixed
- [Middleware defaults to DataLinkEngine for reads/writes but still needs the parameter to be configured](https://dev.azure.com/cat-ciss/G7/_workitems/edit/33426)


## 2.0.0 (2026-03-25)

### Added
- [Testing: TestHarness websocket API wrapper](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30871/)
- [Publish Middleware 1.0.0 binaries using CONAN_CONFIG_0.8.0 profiles](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31460/)
- [Desired onboard middleware pub/sub integration tests list](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31880/)
- [MiddlewareTestHarness: Support Read/Write](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31585/)
- [Fix Middleware ParameterService Unit Testability](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32425)
- [Fix Middleware DiagnosticService Unit Testability](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32426/)
- [Middleware CI Publishes Package Properties to Artifactory](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31233/)
- [Testing: TestHarness Websocket API Wrapper - Add Read Support](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32262/)
- [Middleware Tests:  Pub/Sub Regression Tests Framework](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32220/)
- [Add Middleware Parameter Service API Unit Tests](https://dev.azure.com/cat-ciss/G7/_workitems/edit/25288/)
- [Middleware adoption of CatEValidation](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30382)

### Changed
- [Switch Middleware from export_sources to Conan Git sources](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30381/)
- [Upgrade the CAT Global Conan Profiles and Tools to latest 0.11.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32653)
- [Update Middleware to use new open source Fast DDS 3.4.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31275)
  - This upgrade from fastdds 3.3.0 to 3.4.0 brings only the support of fastdds RPC IDL support. Should not be breaking.
- **[BREAKING]** [Middleware adoption of Fast DDS RPC](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31287/)
  - This change required dead and problematic code in Diagnostics and Parameters API to change.
    - [CONCEPT] IMiddlewareClient is specific to a single server endpoint compared to previously being multiple server endpoints.
    - [CONCEPT] IMiddlewareClient handles `server not available` as any other request. Request times out if no reply is sent from Server.
    - [CONCEPT] IMiddlewareClient is asynchronous via its own thread pool, whose size is configurable, but managed by Middleware's use of asio.
    - [CONCEPT] IMiddlewareServer is asynchronous via its own thread pool, whose size is configurable, but managed via eProsima Fast DDS.
    - [FRAMEWORK] IMiddlewareClient no longer requires source as an argument for Read/Write functions and no longer returns a boolean.
    - [FRAMEWORK] IMiddlewareServer completely changed due to RPC responsibility of being just Run and Stop.
    - [API] Parameter Service client and server thread pool size is hard-coded to 5, with client request timeout duration hard-coded to 10 seconds.
    - [API] Diagnostic Service client and server thread pool size is hard-coded to 5, with client request timeout duration hard-coded to 10 seconds.
    - [API] Parameter Service's IParameterService interface requires the `Data` argument to be `const&` instead of `by-value`.
    - [API] Diagnostic Service's ICodeServer interface requires the `AdditionalInformation` argument to be `const&` instead of `&`.
    - [API] Diagnostic Service's IDiagnosticService interface requires the callback's `AdditionalInformation` argument to be `const&` instead of `&`.
- **[BREAKING]** [Change the Middleware Conan Package Name to pass validation](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32664)
  - Previous name `middleware-cpp` will continue to exist in [Artifactory](https://stuff.ecorp.cat.com/ui/repos/tree/General/cat-e-conan-rel-local/_) but will no longer receive updates.
  - New name `cate_middleware` will contain all future updates and implementations.
  - This includes the removal of deprecated package component `MiddlewareSupporting`, requiring clients to remove in from their library lists if used. This means there is no `cate_middleware::MiddlewareSupporting` component.
  - Usage:
    - When using cmake's find_package the usage is now `find_package(cate_middleware CONFIG REQUIRED)`
    - When linking the package libraries/components via cmake there are a few options:
        - cate_middleware::cate_middleware -> Gives access to Parameters and Diagnostics service libraries and includes
        - cate_middleware::ParameterService -> Gives access to only Parameters service libraries and includes
        - cate_middleware::DiagnosticsService -> Gives access to only Diagnostics service libraries and includes
        - cate_middleware::ServiceHeaders -> Contains no libraries, heads-only for Parameters, Diagnostics, and protobuf generated code. Generally expected for unit test mocking.
### Fixed
- [CI builds are failing everywhere due to foonathan](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31453/)
  - This change required an update to use the latest CAT Global Conan Profiles and Settings from [CONAN_CONFIG.0.8.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31395/).
- [Bosko done crashed at startup](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31443)
  - These changes are not a guaranteed fix, just extra checks around possible code blocks that could be related to the issue.


## 1.0.1 (2026-02-18)

_Core changes on [branch: master_1.x.x_maintenance]. Not all changes can be returned to master line 2.x.x_

### Changed
- [Middleware: Reads/Writes should default to DataLinkEngine](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32177/)

### Fixed
- [PGN Reads sometimes never work](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32157/)
  - Fixed due to the `Middleware: Reads/Writes should default to DataLinkEngine` story
- [Middleware: Async Read crashes the server](https://dev.azure.com/cat-ciss/G7/_workitems/edit/31974/)


## 1.0.0 (2025-12-10)

_This release starts the adoption of the officiated team semantic versioning format_

### Changed
- [Update Middleware to new SDK arm64_guest_os_2025.4.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30586)
  - This includes all published binaries moving from GCC 13.3.0 to 15.2.0 for all platforms.
- **[BREAKING]** [Parameter Service API - Change subscriptions to receive additional metadata on callbacks](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30609)
  - Added new `IConfigBuilder::AddSubscriber` overload function.
    - Existing client Mock classes of `IConfigBuilder` need updated.
  - Previous API calls are active but marked deprecated for removal in the future.
  - Removed `middleware_parameter_unit.proto` in favor of C++ enum type `::middleware::parameter::Unit`
    - See new file [ParameterServiceUnits.h](https://gitgis.ecorp.cat.com/es-csf-core-info/info-services/middleware-cpp/-/blob/master/lib/parameters/ParameterServiceUnits.h?ref_type=heads)

### Fixed
- [middleware-cpp: clean ninja builds fail in protoc](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30099)


## 2025.4.1 (2025-11-07)

### Added
- [Define Gen 7 Conan Package Release Process](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30209/)
- [Republish middleware 2025.4.1 to dev](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30218/)
- [Package 3 Middleware Release Notes](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30216/)

### Changed
- [Bump boost from 1.86.0 to 1.87.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30239/)

### Removed
- [eProsima Fast-DDS and Friends remove revision locks](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30207/)
- [Remove simple-websocket-server middleware dependency](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30252/)

### Fixed
- [Middleware Build Profile Not Setting Tools Shared](https://dev.azure.com/cat-ciss/G7/_workitems/edit/30208/)


## 2025.4.0 (2025-10-28)

_This release has a declared process to support only conan going forward for releases. Overall process is subject to change._

### Added
- [Middleware: Use common conan profiles](https://dev.azure.com/cat-ciss/G7/_workitems/edit/29928/)
- [Release middleware 2025.4.0](https://dev.azure.com/cat-ciss/G7/_workitems/edit/29929/)
- [Measure Middleware latency through a mapper](https://dev.azure.com/cat-ciss/G7/_workitems/edit/29932/)

### Changed
- [Update Gen 7 project shared library requirements update for Package 3](https://dev.azure.com/cat-ciss/G7/_workitems/edit/29789/)

### Removed
- [CI: Remove unused conan recipes, packages, and binaries](https://dev.azure.com/cat-ciss/G7/_workitems/edit/29813/)

### Fixed
- [Clients crash for NOT_FOUND requests to server](https://dev.azure.com/cat-ciss/G7/_workitems/edit/28721/)


## 2025.3.0 (2025-08-11)

_This prototype release process uploads to conan and Git but no MRs are available._

### Added
- New Middleware verity that launches the DiscoverServer application for increased Middleware discovery speeds

### Changed
-  Bump fast-dds from 3.2.1 to 3.3.0  

### Removed
- Removed need for Middleware Parameter Service API Publishers and Subscribers to require PARAMETER_SERVICE_TEST_MODE.xml

### Fixed
- Fixed bug related to seg fault on MessageWriter and MessageReader Debug logging
- Fixed bug related to Diagnostics Additional Info client request callback not being stored in memory.


## 2025.2.0 (2025-06-23)

_This prototype release process uploads to conan and Git but no MRs are available._

### Added
- Automatic cleanup of /dev/shm for stale DDS memory chunks

### Changed
- Switched to Multicast discovery for connection endpoints

### Fixed
- Fixed an issue with large data publishes being capped at 249 bytes


## 2025.1.1 (2025-05-29)

_This prototype release process uploads to conan and Git but no MRs are available._

### Fixed
- Added required libMiddlewareSupporting.so to package list.


## 2025.1.0 (2025-05-28)

_This prototype release process uploads to conan and Git but no MRs are available._

### Added
- Support for Middleware Diagnostic Service API
- Additional logging and verification around "PARAMETER_SERVICE_TEST_MODE.xml"
- Middleware full adoption of Conan with Package Support and Publishing

### Changed
-  Bump fast-dds from 2.14.1 to 3.2.1
-  Data Policy updates for readers and writers
-  **Breaking:** PARAMETER_SERVICE_TEST_MODE.xml has moved from being required in "/opt" to "/opt/appdata/Middleware/PARAMETER_SERVICE_TEST_MODE.xml"

### Fixed
- Fixed R/W concurrency between framework clients and servers
- Significant performance increases for all middleware entities due to system fixes
- Fixed Data transmission lockups

## 2025.0.0 (2025-03-04)

_This prototype release process uploads to conan and Git but no MRs are available._

### Changed
-  Bump boost from 1.67.0 to 1.86.0

## 2024.4.0 (2024-12-11)

_This release adopts a prototype process with content uploaded to conan and Git but no MRs are available._

### Added
- Added logic to not start the internal server instance when the Parameter Service object does not have any server actions configured.
- Made updates to the framework to wrap the Fast DDS objects to be able to perform unit testing on the Middleware Framework.
- Added logging through boost for users to use in testing. Severity levels and stream targets can be set via calling "_::middleware::parameter::Factory::SetLogLevel(<ostream>, <1,2,3,4>)_".
- The Middleware Framework is 90% unit tested

### Changed
- The "On Server Write" callback definition changed to returning a protobuf object instead of using an out parameter.
- The "On Server Read" callback definition changed to returning a protobuf object instead of using an out parameter.

## 2024.4.0 (2024-08-01)

_This release is conceptual and not expected to be used in production. Build with conan, release to Git. No MRs are available._

### Added
- Added a new document that explains the Parameter Service API's software and usages.

### Changed
-  The Parameter Service configuration was changed to be handled by a builder that will not expose the configuration structure. Now the Parameter Service Factory takes the builder as an argument instead of the a configuration structure.
- Coding format changes were implemented and ran on the Parameter Service API.
- Quality of life improvements to the internal middleware layer.
- Middleware framework library re-structured its repository.


### Fixed
- Fixed the 2nd round Blitz review comment fixes.
- Application parameter-service-tester-app fixed a bug where the server test action would get stuck.


## 2024.3.0 (2024-03-01)

_This release is conceptual and not expected to be used in production. Build with conan, release to Git. No MRs are available._

### Added
- ReadParameter and WriteParameter gained a new function of their API called "WaitForServerAvailable". This call blocks until the client gets a heartbeat from the server.
- Fast DDS middleware objects have been given a new logging system to track connections. There are levels that can be set but for now it is set by going to the header and selecting a value then building it with that value. See "middleware-framework/lib/common/HermesLog.h". LVL1 is basic errors and warnings. LVL2 is additional info and LVL3 is in depth debugging. Beware the info logged can be a lot and can be sandwiched with other logs when used across multiple threads.

### Changed
-  SubscriberParameter objects from the API no longer call subscribe on construction. Users must call the new function Subscribe for subscriptions to work.
-  Bump Fast DDS from 2.14.0 to 2.14.3
- There has been a format change to both the PARAMETER_SERVICE_TEST_MODE.xml and the tester application configuration examples. There was a need to add on domain ids for grouping and control. I also added a new tester action called "publish_periodic". This new action consumes a list of values and publishes them at a periodic rate. See config XML comment block for details. Same for the override file PARAMETER_SERVICE_TEST_MODE

### Fixed
- The missing async behavior on the Server side API has been fixed and reworked. This change drove changes across the test app and the internal behavior of the API.


## 2024.2.0 (2024-02-01)

_This release is conceptual and not expected to be used in production. Build with conan, release to Git. No MRs are available._

### Added
-   Added a configurable tester application for users to test their applications using the middleware API.
-   Added the ability to override the parameter source catalog that dictates owners and connections of parameters. See "PARAMETER_SERVICE_TEST_MODE.xml" in each platforms "test" directory. It contains a comment block explaining what to do. This change adds pugixml as a dependency for users.


### Fixed
- Fixed the crash made by the API when users do not have any server actions to perform.

## 2024.1.0 (2024-01-01)

_This release is conceptual and not expected to be used in production. Build with conan, release to Git. No MRs are available._

### Added
-   Added Middleware Framework and parameters project prototypes to GitLab.
