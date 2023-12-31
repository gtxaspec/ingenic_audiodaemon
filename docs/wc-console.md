# `wc-console`: WebSocket Audio Streaming Utility

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Usage](#usage)
- [Examples](#examples)
- [Requirements](#requirements)
- [Building](#building)
- [Sample Code](#sample)
- [Credits](#credits)

## Overview

`wc-console` is a utility tailored for the capture and streaming of audio data from web browsers. It's designed for testing and analysis of the websocket audio stream.

## Features

- **WebSocket Server**: Sets up a WebSocket server to handle incoming audio.
- **Audio Capture to STDOUT**: Designed to capture audio from browsers and directly stream the data to the standard output (`stdout`).
- **IPv4 & IPv6 Support**: Dual-stack support ensures compatibility with both IPv4 and IPv6 network configurations.
- **Silent Mode**: An optional mode for quiet operation without log outputs, for use with applications that take audio in via (`stdin`).
- **Custom IP & Port Configuration**: By default, it listens on the local IP and port 8089, but offers flexibility with custom IP addresses and ports.
- **libwebsockets Integration**: Utilizes the robust `libwebsockets` library to manage WebSocket connections and data transfer.

## Usage

```bash
wc-console [OPTIONS]
```

### Options:

- `-s`: Run in silent mode, suppressing all log outputs.
- `-p <port>`: Specify a custom port for the WebSocket server (default is 8089).
- `-i <ip_address>`: Specify a custom IP address for the WebSocket server (defaults to the local IP).
- `-h`: Display the help message and exit.

## Examples

1. **Default Execution**:
   Running `wc-console` without arguments starts the WebSocket server on the local IP and the default port (8089).

   ```bash
   wc-console
   ```

2. **Silent Mode**:
   For silent operation:

   ```bash
   wc-console -s
   ```

3. **Custom IP & Port**:
   To use a specific IP and port:

   ```bash
   wc-console -i 192.168.1.100 -p 9000
   ```

4. **wc-console & iac**:
   To use `wc-console` together with `iac` to stream audio to `iad`:

   ```bash
   wc-console -s | ./iac -s
   ```

## Requirements

- **libwebsockets**: Ensure the `libwebsockets` library is properly installed and linked.

## Building

```
make deps
make wc-console
```

## Sample Code

- An example demonstrating streaming of audio captured from a web browser to wc-console using WebSockets. Refer to the directory `www-example-root/` for details.

## Credits

This project is inspired by the [simple-recorderjs-demo](https://github.com/addpipe/simple-recorderjs-demo/tree/master) from addpipe and uses the [Recorderjs](https://github.com/mattdiamond/Recorderjs) library from mattdiamond.
