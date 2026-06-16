# ROS2 Applications

Any ROS2 research and development applicaions can be maintained over here.

## Baseline pub/sub migration

The middleware-cpp baseline publisher/subscriber apps are migrated to ROS2 in the `cat_apps` package.

**Full migration guide (architecture, file-by-file changes, testing):**
[baseline-pubsub-migration.md](../docs/baseline-pubsub-migration.md)

### Quick run with launch files (recommended)

After build, start subscriber and publisher together with one command:

```bash
source install/setup.bash
ros2 launch cat_apps baseline_pubsub.launch.yaml
```

Optional launch arguments:

```bash
# Multi-threaded publisher mode
ros2 launch cat_apps baseline_pubsub.launch.yaml thread_mode:=1

# Custom XML config
ros2 launch cat_apps baseline_pubsub.launch.yaml config_file:=/path/to/config.xml

# Run subscriber or publisher alone
ros2 launch cat_apps baseline_subscriber.launch.yaml
ros2 launch cat_apps baseline_publisher.launch.yaml thread_mode:=0
```

Launch files live in `src/porting/cat_apps/launch/` and install to `share/cat_apps/launch/`.

### Quick run with executables (manual)

```bash
source install/setup.bash
ros2 run cat_apps BaselineSubscriber $(ros2 pkg prefix cat_apps)/share/cat_apps/config-example.xml
ros2 run cat_apps BaselinePublisher 0 $(ros2 pkg prefix cat_apps)/share/cat_apps/config-example.xml
```

### Automated Docker build + smoke test

After starting GuestOS-Base (see below):

```bash
# From host
docker exec -it GuestOS-Base-${USER} bash /app/ros2_ws/scripts/build-and-test-baseline.sh

# Build only (no smoke test)
docker exec -it GuestOS-Base-${USER} bash -c 'RUN_SMOKE_TEST=0 /app/ros2_ws/scripts/build-and-test-baseline.sh'
```

The script builds `cat_msgs`, `pugixml`, `cat_apps`, and `process_launcher`, then runs:
1. Manual subscriber + publisher smoke test (5 seconds)
2. `ros2 launch cat_apps baseline_pubsub.launch.yaml` smoke test (8 seconds)

## How to compile/build/test

Make sure below docker images in the virtual machines
``` bash
$ docker images
ros2-cust-jazzy:latest
ros:jazzy-ros-base
ros:jazzy-ros-core
```

Run docker containers from docker folder using script
``` bash
$ cd ws4-mw/ros2/docker
$ ./launch-guest-os.sh
```

The script launch three containers by adding suffix as USER name to the container.
- **ros:jazzy-ros-base**: Able to compile/build/test/introspect the applications, the container name **GuestOS-Base-<user_name>**.
  Check the ROS2 documentation (Jazzy Distro) for any kind of analysis.
- **ros:jazzy-ros-core**: Able to test/introspection the applications, the container name **GuestOS-Core-<user_name>**.
- **ros2-cust-jazzy:latest**: Its a custom docker ROS2 jazzy image. Able to test only applications, the container name **GuestOS-Cust-<user_name>**.

- <user_name> : user_name ($USER) is automatically picked from host machine, when you launch containers through script.


NOTE: Don't modify anything in **docker-compose.yml** file once you launched all the containers.

Destroy docker containers if work is done for the day, from docker folder using script
``` bash
$ cd ws4-mw/ros2/docker
$ ./destroy-guest-os.sh
```

To launch specifc docker container shell environment
```bash
$ docker exec -it <container_name> bash
```

- <container_name> : Name of the container name running on the host machine.