# Uruchomienie
## Kompilacja

```sh
gcc main.c
```

## Pozyskanie strumienia dzwięku

```sh
ffmpeg -y -i http://wms-15.streamsrus.com:11630 -f u16le -acodec pcm_s16le -ac 2 -ar 44100 /tmp/bassfifo
```

## Uruchomienie skryptu

```sh
./a.out
```

## Pomiar przepustowości

```sh
./a.out | vh > /dev/null
```

## Pomiar przepustowości + odtwarzanie

```sh
./a.out | pv | play -t raw -r 44100 -b 16 -c 2 -e signed --endian little -
```

Dźwięk powinien być czysty

# Szczegóły techniczne

## /tmp/bassfifo

Do pliku trafiją dane dzwiękowe (44100Hz, 16bitów, stereo, wartości signed)

## main.c

Co sekundę wysyła 44100 * 16 * 2 bitów danych

W jedym datagramie UDP znajduje się 512 bajtów danych
