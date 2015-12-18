# himawari8_service

## How to downloading the Image File

process the blown url to getting the last update image time

```
http://himawari8.nict.go.jp/img/D531106/latest.json
```

It will return the last update information about the earth image, such as

```json
{"date":"2015-12-17 13:30:00","file":"PI_H08_20151217_1330_TRC_FLDK_R10_PGPFD.png"}
```

Process the blown url to getting the image

```
http://himawari8.nict.go.jp/img/D531106/{TILE_COUNT}d/550/2015/12/17/083000_{x}_{y}.png
```

- `TILE_COUNT`, should be either 1, 2, 4, or 16. each tile is `550px * 550px`, so 4 (default) will produnce an image `2200px * 2200px`.
- `X`,`Y`, is the image tile coordinate.

```
http://himawari8.nict.go.jp/
https://www.v2ex.com/t/241563
```