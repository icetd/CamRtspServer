## build
```
./scripts/build_x264.sh
git submodule update --init
mkdir build && cd build
cmake ..
make
```
## config
config with file configs/config.ini
```
[log]
level = NOTICE

[video]
width = 640
height = 480
fps = 30
method = CRF      ; [CRF | ABR]
bitrate = 1440    ; if ABR bitrate is efficient
rf_constant = 23  ; if CRF rf_constant is efficient  
format = YUY2 [YUY2 | MJPEG]
device = /dev/video0

[server]
rtsp_port = 8554
stream_name = unicast
max_buf_size = 200000
max_packet_size  = 1500
http_enable = false
http_port = 8000
bitrate = 1440
```
## function
- support mjpeg and yuy2 format usb camera.
- support h264 codec.
