# Violet
Violet is an in-memory key–value database, which can be used as a distributed cache and message broker, with optional durability.

---

## Misc Commands

* **STATUS**: Returns server status (e.g., running or shutting down).
* **ECHO <message>**: Returns the message back—used for testing connectivity.
* **FLUSHALL**: Clears all keys from all databases.

---

## Key–Value Operations

* **SET <key> <value>**: Stores the value against the specified key.
* **GET <key>**: Retrieves the value for a given key.
* **KEYS**: Lists all keys currently stored.
* **TYPE <key>**: Returns the data type of the given key (string, list, hash, or none).

---

## List Operations

* **LGET <key>**: Returns all elements in the list.
* **LLEN <key>**: Returns the length of the list.
* **LPUSH <key> <value>**: Pushes a value to the left (front) of the list.
* **RPUSH <key> <value>**: Pushes a value to the right (back) of the list.
* **LPOP <key>**: Removes and returns the leftmost element.
* **RPOP <key>**: Removes and returns the rightmost element.
* **LREM <key> <count> <value>**: Removes elements equal to value (count > 0 from left, < 0 from right, 0 = all).
* **LINDEX <key> <index>**: Gets the list element at the specified index.
* **LSET <key> <index> <value>**: Sets the element at index to the new value.

---

## Hash Operations

* **HSET <key> <field> <value>**: Sets a field in the hash to a value.
* **HGET <key> <field>**: Gets the value of a field in the hash.
* **HEXISTS <key> <field>**: Checks if a field exists in the hash.
* **HDEL <key> <field>**: Deletes a field from the hash.
* **HGETALL <key>**: Returns all fields and values in the hash.
* **HKEYS <key>**: Returns all field names in the hash.
* **HVALS <key>**: Returns all field values in the hash.
* **HLEN <key>**: Returns the number of fields in the hash.
* **HMSET <key> <field1> <val1> ...**: Sets multiple field-value pairs in one go.

---

# Build
To build,
>* run the ```build.sh``` file.

---

# Dependencies


---

# Requirements
To build and run, you need:
>* Unix/Unix-like system or WSL
>* gcc or clang
>* cmake