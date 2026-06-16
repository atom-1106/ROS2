# ROS2 Applications

Any ROS2 research and development applicaions can be maintained over here.

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