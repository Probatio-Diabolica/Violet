# Violet
Violet is a lightweight, Redis-inspired in-memory key–value store supporting strings, lists, and hashes with optional persistence. Ideal for caching, pub-sub systems, or prototyping distributed workflows.

---

## Supported Commands
works in upper and lower cases.
### Common
* **STATUS**: `STATUS` → returns connection status
* **ECHO**: `ECHO <msg>` → `<msg>`
* **FLUSHALL**: `FLUSHALL` → clear all data

### Key/Value
* **SET**: `SET <key> <value>` → store string
* **GET**: `GET <key>` → retrieve string or nil
* **KEYS**: `KEYS *` → list all keys
* **TYPE**: `TYPE <key>` → `string`/`list`/`hash`/`none`
* **DEL/UNLINK**: `DEL <key>` → delete key
* **EXPIRE**: `EXPIRE <key> <seconds>` → set TTL
* **RENAME**: `RENAME <old> <new>` → rename key

### Lists
* **LGET**: `LGET <key>` → all elements
* **LLEN**: `LLEN <key>` → length
* **LPUSH/RPUSH**: `LPUSH <key> <v1> [v2 ...]` / `RPUSH` → push multiple
* **LPOP/RPOP**: `LPOP <key>` / `RPOP <key>` → pop one
* **LREM**: `LREM <key> <count> <value>` → remove occurrences
* **LINDEX**: `LINDEX <key> <index>` → get element
* **LSET**: `LSET <key> <index> <value>` → set element

### Hashes
* **HSET**: `HSET <key> <field> <value>`
* **HGET**: `HGET <key> <field>`
* **HEXISTS**: `HEXISTS <key> <field>`
* **HDEL**: `HDEL <key> <field>`
* **HLEN**: `HLEN <key>` → field count
* **HKEYS**: `HKEYS <key>` → all fields
* **HVALS**: `HVALS <key>` → all values
* **HGETALL**: `HGETALL <key>` → field/value pairs
* **HMSET**: `HMSET <key> <f1> <v1> [f2 v2 ...]`
---

## Build and Run

To get Violet and its client, Red, up and running:

* **Full Build (Server and Client):**
    * Clone the Violet repository.
    * Run the `buildall.sh` script from the root directory. This will compile both the Violet server and automatically clone and build the Red client, placing its executable in Violet's `build/` directory.

    ```bash
    ./buildall.sh
    ```

* **Server-Only Build:**
    * If you only need the Violet server, you can use the dedicated build script:

    ```bash
    ./build.sh
    ```

* **Launching the Server and Client:**
    * After building, navigate to the `build/` directory.
    * You can then launch the Violet server and the Red client. Ensure you specify the same port if you want them to communicate.

    ```bash
    ./violet <port [optional]>
    ./red <port [same as server, drop if none]>
    ```

---

# Dependencies

* **[Red](https://github.com/Probatio-Diabolica/Red)**: The custom client for Violet.

---

# Requirements
To build and run, you need:
>* Unix/Unix-like system (Linux/macOS/WSL)
>* gcc or clang
>* cmake