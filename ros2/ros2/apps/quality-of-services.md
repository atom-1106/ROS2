# Quality of Services (QoS)

- **The core data delivery attributes** (how data moves).
- **The temporal/monitoring attributes** (how time and node health are managed).

### 1. Data Delivery Attributes (The Core)

These attributes define the "Contract" between the Publisher and the Subscriber.

#### **History**
Defines how the middleware handles a queue of messages when the subscriber cannot keep up with the publisher.
* **KeepLast:** The middleware keeps only the $N$ most recent messages (where $N$ is the Depth). Older messages are discarded to make room for new ones.
* **KeepAll:** The middleware attempts to store every single message until the subscriber consumes it (high memory risk).

#### **Depth**
This is the integer value ($N$) associated with the `KeepLast` history policy.
* It defines the **size of the buffer**.
* *Example:* In `ParametersQoS`, a depth of `1000` means the system can buffer 1000 parameter changes before it starts dropping the oldest ones.

#### **Reliability**
Defines the guarantee of delivery.
* **Reliable:** Uses an acknowledgment system (ACK/NACK). If a packet is lost, it is re-sent. This ensures data integrity but increases latency.
* **Best Effort:** "Fire and forget." The publisher sends the data and doesn't care if it arrives. This is extremely fast and has low latency but allows for data loss.
* **Best Available:** A specialized mode where the middleware attempts to find the best possible match between a Reliable publisher and a Best Effort subscriber.

#### **Durability**
Defines what happens when a Subscriber joins the network *after* messages have already been sent.
* **Volatile:** The subscriber only receives messages sent *after* the moment it connected. It has no "memory" of the previous messages.
* **Transient Local:** The publisher saves the most recent messages in a local buffer. When a new subscriber connects, the publisher automatically "catches them up" by sending those stored messages.

---

### 2. Temporal & Monitoring Attributes (The Advanced)

These attributes are used for advanced system health monitoring and real-time constraints.

#### **Deadline**
Defines the maximum allowable time between two consecutive messages.
* If a publisher fails to send a message within the specified `Deadline`, a "Deadline Missed" event is triggered. This is used in safety-critical systems to detect if a sensor has frozen or a loop has crashed.

#### **Lifespan**
Defines the "shelf-life" of a single message.
* Once a message is created, it has a timer. If the message is not delivered or consumed within its `Lifespan`, it is considered expired and is discarded. This prevents a subscriber from processing "stale" data that is no longer relevant.

#### **Liveliness**
Defines how the system detects if a node is still "alive" and functioning.
* It monitors the "heartbeat" of a node. If a node stops communicating, the middleware can trigger an event to notify other parts of the system that the node has failed.

#### **Lease Duration**
This is the technical mechanism used to implement **Liveliness**.
* It is a time duration (lease) granted to a node. The node must "renew" its lease periodically. If the lease expires before renewal, the node is considered "dead" (Liveliness loss).

---

### 3. ROS2 QoS defined classes:
#### 3.1. **ClockQoS**
- History: Keep last,
- Depth: 1,
- Reliability: Best effort,
- Durability: Volatile,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.2. **SensorDataQoS**
- History: Keep last,
- Depth: 5,
- Reliability: Best effort,
- Durability: Volatile,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.3. **ParametersQoS**
- History: Keep last,
- Depth: 1000,
- Reliability: Reliable,
- Durability: Volatile,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.4. **ServiceQoS**
- History: Keep last,
- Depth: 10,
- Reliability: Reliable,
- Durability: Volatile,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.5. **ParameterEventsQoS**
- History: Keep last,
- Depth: 1000,
- Reliability: Reliable,
- Durability: Volatile,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.6. **RosoutQoS**
- History: Keep last,
- Depth: 1000,
- Reliability: Reliable,
- Durability: TRANSIENT_LOCAL,
- Deadline: Default,
- Lifespan: {10, 0},
- Liveliness: System default,
- Liveliness lease duration: default

#### 3.7. **SystemDefaultsQoS**
- History: System default,
- Depth: System default,
- Reliability: System default,
- Durability: System default,
- Deadline: Default,
- Lifespan: Default,
- Liveliness: System default,
- Liveliness lease duration: System default

#### 3.8. **BestAvailableQoS**
- History: Keep last,
- Depth: 10,
- Reliability: Best available,
- Durability: Best available,
- Deadline: Best available,
- Lifespan: Default,
- Liveliness: Best available,
- Liveliness lease duration: Best available

---

### 4. Summary of your Specific QoS Profiles
**`SensorDataQoS`**:
Designed for speed. Low depth (5), Best Effort (no retries), and Volatile (no history for latecomers).

**`ParametersQoS`**:
Designed for stability. High depth (1000) to ensure no parameter changes are lost, and Reliable to ensure the configuration is exact.

**`RosoutQoS`**:
Designed for logging. Uses **Transient Local** so that if a diagnostic tool connects late, it can still see the recent logs that were sent.

**`BestAvailableQoS`**:
A "smart" profile that tries to match the highest possible settings (Reliability/Durability) supported by both the publisher and subscriber.

---

### 5. Sample QoS settings between Publisher and Subscriber
The Publisher and Subscriber **do not need to have identical QoS settings**, but must follow **"Compatibility Rules."**

**NOTE**: Think of QoS as a negotiation. If the Publisher's requirements are too strict for the Subscriber, or if the Subscriber's requirements are too strict for the Publisher, the connection will fail (they will be "incompatible").

### 5.1. The Rules of Negotiation
#### **Reliability (The most common cause of failure)**
*   **Rule:** A **Best Effort** subscriber can listen to a **Reliable** publisher.
*   **Conflict:** A **Reliable** subscriber **CANNOT** listen to a **Best Effort** publisher.

#### **Durability (The "Latecomer" rule)**
*   **Rule:** A **Transient Local** subscriber can listen to a **Volatile** publisher.
*   **Rule:** A **Volatile** subscriber can listen to a **Transient Local** publisher.
*   **Conflict:** A **Transient Local** subscriber **CANNOT** listen to a **Volatile** publisher and expect to get old data.

#### **History and Depth**
*   **Rule:** These are generally compatible. The middleware manages the buffers. If the publisher has a depth of 10 and the subscriber has a depth of 100, the subscriber will simply wait for the 10 messages to arrive.

---

### 5.2. Compatibility Matrix Summary
| Subscriber Setting | Publisher Setting | **Compatible?** | Result |
| :--- | :--- | :--- | :--- |
| **Reliable** | **Reliable** | YES | Full reliability. |
| **Reliable** | **Best Effort** | **NO** | **Connection Fails.** |
| **Best Effort** | **Reliable** | YES | Subscriber gets data but ignores lost packets. |
| **Best Effort** | **Best Effort** | YES | Fast, low-overhead communication. |
| | | | |
| **Transient Local**| **Transient Local**| YES | Subscriber gets historical data. |
| **Transient Local**| **Volatile** | YES | Subscriber gets only *new* data (History is lost). |
| **Volatile** | **Transient Local**| YES | Subscriber gets historical data immediately. |
| **Volatile** | **Volatile** | YES | Standard real-time communication. |

---

### 5.3. Practical Usage
1.  **If try to build a Sensor Node (Publisher):** Use `SensorDataQoS`. It is "forgiving." It allows different types of subscribers to connect easily without breaking the system.
2.  **If try to build a Controller/Command Node (Publisher):** Use `SystemDefaultsQoS`. It is "strict." It ensures that if a subscriber is listening, it is actually receiving every single command.
