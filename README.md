# crypto-c
This repository contains c implementations of crypto algorithms.

## Symmetric Key
* DES (note: DES is not considered secure due to its small key size)

### DES
This repository contains test vectors taken from [NBS Special Publication 500-20](https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nbsspecialpublication500-20e1977.pdf).
When validating the implementation provided in this repository an error was discovered (p. 28) in test vectors provided by the document.

It reports the test vector `0101010101010101 E1652C6B138C64A5 0000000040000000`. The error is in the cipher text, which should be `0000000400000000`, as validated by the [PyCryptodome](https://github.com/Legrandin/pycryptodome) implementation of DES.

This change is reflected in the test vectors provided.
