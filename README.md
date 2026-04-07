# JRON
A Ripoff of Aeron in C; maybe more like Kafka running on QUIC. 

# Architecture:
Supports Multiple writers and readers to a topic. All readers are broadcasted and update.
## Components:

1. The Server
    - Connections
    - Topics:
      - Create
      - Destroy
2. Data Structures:
    - Ring Buffer for a Topic; Define a size and the topic never grows larger, 
    client either connects from last known place, or the full buffer is played
    - Connection Manager -> HashMap with subName as key -> values a struct of: last index commited, last timestamp of message
3. Protocol:
   - PUBLISH
   - SUBSCRIBE
   - COMMIT_OFFSET
   - MSG Format:
     - Header:
       - offset
       - timestamp; unix epoch
       - bytes
     - payload

## Quick build

From the project root run:

```bash
cmake -S . -B build
cmake --build build --config Debug
# run the binary
build/bin/jron_app
```

This creates a static `jron` library from `src/` and an example executable `jron_app` that links it. The code is organized under `src/` and public headers under `include/`.
