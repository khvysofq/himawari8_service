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
High Definition Earth-Viewing System
```

## 任务列表

1. 写一个程序，能够自动抓取`himawari8`的`1 * 1 (550 * 550)`, `2 * 2 (1100 * 1100)`, `4 * 4 (2200 * 2200)`, `8 * 8 (4400 * 4400)`图片，并且合成一张大图

```
https://github.com/dandelany/animate-earth/blob/master/scrape.js#L22
http://www.jma.go.jp/en/gms/imgs_c/6/visible/0/201512200430-00.png
```