# Copyright (C) Caterpillar Inc. All Rights Reserved.

Feature: Middleware Parameter Service Performance Tests
  These tests are ran to measure the Middleware Parameter Service API resource usage during
  runtime to determine if our performance goals meet system resource requirements.
  [Goals]
  - 1000 channels at 100 ms rates

  [System]
  The host machine owns and manages a list of Virtual Machines.
  - Top-level: Host Machine
  - Sub-level: Virtual Machines

  [System Boundaries]
  - One subscription per parameter name, per Middleware Parameter Service instance
  - One publisher per parameter name, per Middleware Parameter Service instance
  - One publisher per parameter name, per complete system (all VMs)

  ############ Same Virtual Machine - Shared Memory ###############
  Background:
    Given I have launched a Gen 7 Virtual Machine called "devvm1"
    And I am able to run software applications inside all Virtual Machines
    And all software applications running for performance testing will be using Middleware Parameter Service
    And any listed publishing applications will only be capable of publishing parameters
    And any listed subscribing applications will only be capable of subscribing to parameters


  Scenario: [SHM] 1000 Subscriptions matched to 1000 Publishers at 100ms publishing rate
    Given I have a publishing application running in "devvm1" with many parameters
      | Parameter Count | Rate  | Duration     |
      | 1000            | 100ms | milliseconds |
    When I have a subscribing application running in "devvm1" with a subscription matched to every available publisher
    Then all resource usage in the Virtual Machines will be in acceptable ranges
    And the Middleware Parameter Service will still be functional


  Scenario: [SHM] 1000 Subscriptions matched to 1000 Publishers at 100ms publishing rate, disconnect-reconnect
    Given I have a publishing application running in "devvm1" with many parameters
      | Parameter Count | Rate  | Duration     |
      | 1000            | 100ms | milliseconds |
    And I have a subscribing application running in "devvm1" with a subscription matched to every available publisher
    When the publishing application restarts
    Then all resource usage in the Virtual Machines will be in acceptable ranges
    And the Middleware Parameter Service will still be functional


  Scenario: [SHM] 1000 Subscriptions matched to 1000 Publishers at 100ms publishing rate, multiple applications
    Given I have a publishing application running using Middleware Parameter Service with many parameters
      | Parameter Count | Rate  | Duration     |
      | 1000            | 100ms | milliseconds |
    And I have subscribing application "Alpha" running in "devvm1" with a subscription matched to every available publisher
    And I have subscribing application "Bravo" running in "devvm1" with a subscription matched to every available publisher
    When I have subscribing application "Delta" running in "devvm1" with a subscription matched to every available publisher
    Then all resource usage in the Virtual Machines will be in acceptable ranges
    And the Middleware Parameter Service will still be functional


  ############ Multiple Virtual Machines - UDP ###############
  Background:
    Given I have launched a Gen 7 Virtual Machine called "devvm1"
    And I have launched a Gen 7 Virtual Machine called "devvm2"
    And I have launched a Gen 7 Virtual Machine called "devvm3"
    And I have launched a Gen 7 Virtual Machine called "devvm4"
    And I have launched a Gen 7 Virtual Machine called "devvm5"
    And I am able to run software applications inside all Virtual Machines
    And all software applications running for performance testing will be using Middleware Parameter Service
    And any listed publishing applications will only be capable of publishing parameters
    And any listed subscribing applications will only be capable of subscribing to parameters

  # Note: For clarity, subscribing VMs will match to 25% of the publishers from the publishing VM
  Scenario: [UDP] 1000 Subscriptions matched to 1000 Publishers at 100ms publishing rate
    Given I have a publishing application running in "devvm1" with many parameters
      | Parameter Count | Rate  | Duration     |
      | 1000            | 100ms | milliseconds |
    And I have a subscribing application running in "devvm2" with a subscription matched to 250 unmatched publishers
    And I have a subscribing application running in "devvm3" with a subscription matched to 250 unmatched publishers
    And I have a subscribing application running in "devvm4" with a subscription matched to 250 unmatched publishers
    When I have a subscribing application running in "devvm5" with a subscription matched to 250 unmatched publishers
    Then all resource usage in the Virtual Machines will be in acceptable ranges
    And the Middleware Parameter Service will still be functional


  Scenario: [UDP] 1000 Subscriptions matched to 1000 Publishers at 100ms publishing rate, disconnect-reconnect
    Given I have a publishing application running in "devvm1" with many parameters
      | Parameter Count | Rate  | Duration     |
      | 1000            | 100ms | milliseconds |
    And I have a subscribing application running in "devvm2" with a subscription matched to 250 unmatched publishers
    And I have a subscribing application running in "devvm3" with a subscription matched to 250 unmatched publishers
    And I have a subscribing application running in "devvm4" with a subscription matched to 250 unmatched publishers
    And I have a subscribing application running in "devvm5" with a subscription matched to 250 unmatched publishers
    When the publishing application restarts
    Then all resource usage in the Virtual Machines will be in acceptable ranges
    And the Middleware Parameter Service will still be functional

