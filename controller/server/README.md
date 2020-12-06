# BASS Server

## Local development

### Create .env

```sh
cp .env.example .env
```

Fill with appropriate values.

### Run mqtt broker

Eg. mosquitto using docker:

```sh
docker run -it --name mosquitto -p 1883:1883 eclipse-mosquitto
```

Then start/stop using:

```sh
docker start mosquitto
docker stop mosquitto
```

To send mqtt messages use:

```sh
docker exec -it mosquitto /bin/sh
```

And then `mosquitto_pub` and `mosquitto_sub`.

### Run with pipenv

```sh
pipenv install
pipenv shell
python run.py
```

The server should run on 0.0.0.0:3000.

Hot reload is disabled due to a problem with the background MQTT thread.
