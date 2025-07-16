# Research-public-key
Preface
A Study on Puzzles: Software for Entertainment and Research
This software, previously used for research, is now being shared for entertainment and study. As the saying goes: As long as your thinking doesn't slip, there are always more solutions than problems.
Below is an introduction to the software's core functions. Some components are still being modified and tested and will be uploaded later.

1. displacement_fission
This function uses a "fission" method to narrow down the search range for a private key corresponding to a given public key. It leverages the linear (i.e., functional) relationship between public and private keys, progressively reducing the search space through "displacement" and "fission" (halving via division). The resulting transformed public key is then matched against known public keys in a Bloom filter. If a match is found, a result is obtained. The analogy is a wild kangaroo and a tame kangaroo colliding; based on their respective paths, we can trace back to the tame one's home, which represents the final private key.
2. findkeys
This function works by "displacement" (i.e., subtraction), where it subtracts certain values to generate a new public key, and then uses division to narrow the range. You can think of it as continuously subtracting from a numerical value until it becomes an even number, then performing division to reduce it further. Similarly, the Bloom filter can be pre-populated with the public keys corresponding to a small range of private keys (e.g., 1 to 100 million). The target's value is then continuously reduced until it matches an entry in the Bloom filter. The principle is the same as the kangaroo analogy: once they collide, we know their paths, and by following the path back, we find the home—the private key.
3. generate_keys
This function is for generating the "base points" needed for the Bloom filter, which are our "tame kangaroos." Specifically, these are the public keys corresponding to private keys ranging from 1 to 100 million. When the value of a target public key is reduced by the programs above to fall within this range, it can match one of these pre-computed keys, causing a "collision." You can also generate these points using a random cloner from a public key library, but only a public key generated from a known private key qualifies as a "tame kangaroo." Otherwise, it is of unknown origin and is considered "wild."
4. calc_hex and invert_fission
These are recovery tools. Based on the "path" recorded from a successful kangaroo collision, these tools can trace back to the "home" and restore the final private key.
5. search_divisible_ranged, extract_public, pkconvert
These are auxiliary tools:
search_divisible_ranged: This analyzes values divisible by 2. It determines how much of the prefix is divisible, while the suffix provides a reference for parameters like the required Bloom filter size or the number of iterations.
extract_public: This program removes the suffix from input public keys and outputs the result to a new file. It is significantly faster than using standard system commands for this task.
pkconvert: This tool converts public keys into a uniform format. For instance, it can convert uncompressed public keys extracted from a blockchain into the compressed format, or vice versa. It supports batch operations.
6. I need to buy the compressed password, why not provide it for free? Because I need financial support to develop more programs, I hope everyone understands, reserves a seat, and waits for subsequent updates...

# Required libraries

On Debian/Ubuntu systems:
```
sudo apt-get update
sudo apt-get install libgmp-dev libsecp256k1-dev
```
On Fedora/CentOS/RHEL systems:
```
sudo dnf install gmp-devel

libsecp256k1 may need to be compiled from source code, as it is not necessarily in the default repository. Visit https://github.com/bitcoin-core/secp256k1 and follow the instructions to compile and install
```
On macOS (using Homebrew):
```
brew install gmp
brew install libsecp256k1
```
# Compilation directives

```
g++ -o displacement displacement_fission.cpp -O3 -march=native -flto -lgmp -lgmpxx -lsecp256k1 -pthread
g++ -o findkeys findkeys.cpp -O3 -march=native -flto -lgmp -lsecp256k1 -pthread
g++ -o generate_keys generate_keys.cpp -O3 -march=native -flto -lsecp256k1 -pthread
g++ -o calc_hex calc_hex.cpp -O3 -lgmp
g++ -o invert_fission invert_fission.cpp -lgmp
g++ -o search_script search_divisible_ranged.cpp -lgmpxx -lgmp
g++ -o extract_public extract_public.cpp -O3 -march=native
g++ -o pkconvert pkconvert.cpp -O3 -march=native -lsecp256k1
```

# displacement Instructions and Tests
```
./displacement -h
Usage: ./displacement -p <pubkey/file> -s <start_hex> <end_hex> -n <depth> -t <threads> -f <compare_file>
  -p: File containing public keys, or a single public key.
  -s: Hexadecimal range for subtraction (start and end).
  -n: Depth of the fission/division process.
  -t: Number of threads.
  -f: File with public keys to compare against.
```
1. -s <start_hex> <end_hex> -s 0 1, represents the value to be subtracted (used for shifting to an even number), which can actually be controlled to execute in a limited area, for example: you can do this -s 1 10000000... The more areas, the slower it is.
2. -n is the number of iterative splits. It will start from the original public key and split 1.2.4.8.16.32.64.128.256.512... The more splits, the slower it is.
3. -f Bloom's base public key, which is the tamed kangaroo, allows the split to produce a collision between the wild and the tamed, and the result can be found by matching.
4. -p single public key or batch public key, you can use the cloner to copy a large number of public keys, and collide and match with the tamed kangaroo, which is the known public key.

# Run the instance to match the output
```
./displacement -p test_pu.txt -s 0 1 -n 20 -t 8 -f f4240.txt
Calculating lines in input file...
Total lines approximately: 2
Loading input public keys: [==================================================] 100 %
Successfully loaded 1 public keys to process.
Calculating lines in comparison file...
Total lines approximately: 1000001
Loading  comparison  file: [==================================================] 100 %
Successfully loaded 1000000 comparison keys (after deduplication).
Initializing Bloom filter: [==================================================] 100 %
Bloom filter initialized.

-------------------- Run Configuration --------------------
Input Source (-p): test_pu.txt (1 keys)
Comparison File (-f): f4240.txt (1000000 keys)
Subtraction Range (-s): 0x0 to 0x1
Fission Depth (-n): 20
Threads (-t): 8
---------------------------------------------------------

                                                                                                                                                                                   
Match found!
  Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
  Subtracted Scalar (k): 0x0
  Fission Depth: 15 / 20
  Fission Path (+1 choices): 010000110110100
  Matched Public Key: 0260e19bb8e275d2a29a043818453380f765d4676f1bc91a88053f7468d3e77029
                                                                                                                                                                                    
Match found!
  Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
  Subtracted Scalar (k): 0x0
  Fission Depth: 16 / 20
  Fission Path (+1 choices): 0010000110110100
  Matched Public Key: 039c600d26249ba3a60179e48afef08f673d9b0c88623e9532c9b09bf5e82ca29c
 .
 .
 .
 .
                                                                                                                                                                                    
Match found!
  Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
  Subtracted Scalar (k): 0x0
  Fission Depth: 15 / 20
  Fission Path (+1 choices): 010000110110100
  Matched Public Key: 0260e19bb8e275d2a29a043818453380f765d4676f1bc91a88053f7468d3e77029
                                                                                                                                                                                    
Match found!
  Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
  Subtracted Scalar (k): 0x0
  Fission Depth: 16 / 20
  Fission Path (+1 choices): 0010000110110100
  Matched Public Key: 039c600d26249ba3a60179e48afef08f673d9b0c88623e9532c9b09bf5e82ca29c
Processed: 5542447 | Speed: 163.94 k/s | Fission: [26%] | Range: [0%] 0x0/0x1 | PK: [1/1]: 02532257fdfc2104d151d8d813f...  ^C
```
# Recovery Instructions

The above is an example of collision and matching.

-------------------- Run Configuration --------------------
Input Source (-p): test_pu.txt (1 keys)
Comparison File (-f): f4240.txt (1000000 keys)
Subtraction Range (-s): 0x0 to 0x1
Fission Depth (-n): 20
Threads (-t): 8
---------------------------------------------------------

These values explain a lot of information. f4240.txt is the public key corresponding to the private key from 1 to f4240. The base point is the tamed kangaroo, which collides with the public key in the wild kangaroo -p loading document Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579.
You can use advanced commands to search for the private key corresponding to the public key in the document:
```
find -type f -exec grep -B 3 "02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579" {} +
```
Matched Public Key: 039c600d26249ba3a60179e48afef08f673d9b0c88623e9532c9b09bf5e82ca29c , which is the public key of the tamed kangaroo, and the private key we know is: 6373d .
If you use the cloner to calculate the cloned public key, you must remember the number of digits, otherwise you will not be able to recover it because it has become wild.

The base point you created with generate_keys is the tamed kangaroo public key, and there is a private key behind it. You can find it out by checking it. If you forget it, you can use VanitySearch to search the address corresponding to the public key from 1 to f4240 and get the private key.

**************************************************************************************************************************************

We use the invert_fission program to restore it and find the home of the wild kangaroo, which is the private key.
```
./invert_fission
Usage: ./invert_fission <matched_private_key_hex> <fission_path_binary> <subtraction_k_hex>
Example: ./invert_fission 6373d 0010000110110100 0
```
First, 6373d 0010000110110100 0 must be understood.

6373d is the private key, which is the tamed kangaroo.

0010000110110100 is the path that allows us to find his home, which is the private key

0 is the subtracted value, which also means shifting. You can think of it as the fork in the path.
```
./invert_fission 6373d 0010000110110100 0
--- Input Parameters for INVERSE Calculation ---
Matched Private Key (sk_matched): 0x6373d
Subtraction Scalar (k):           0x0
Fission Path (binary):            0010000110110100 (d=16)
Fission Path Value (c):           8628

--- INVERSE Calculation Steps ---
Curve Order (N): 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141
Step 1: 2^d = 2^16 = 65536
Step 2: sk_matched * 2^d = 26696548352
Step 3: (Step 2) - c = 26696539724
Step 4: (Step 3) + k = 26696539724
Step 5: (Step 4) mod N

--- Result ---
Restored INITIAL Private Key (hex): 00000000000000000000000000000000000000000000000000000006373cde4c
```
Recovery successful!

*************************************************************************************************************************************

Now there is a private key on it, which can be considered as the kangaroo's home.

Next, use the tool to verify whether it is the private key corresponding to the public key.
```
./key 6373cde4c
Input Private Key (Hex): 6373cde4c
Private Key (Hex, 32 bytes): 00000000000000000000000000000000000000000000000000000006373cde4c
WIF (Uncompressed): 5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3pq2rcqa4Cv8H
WIF (Compressed):   KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9MU71C1BKVQJw9A

Public Key (Compressed,   33 bytes): 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
Public Key (Uncompressed, 65 bytes): 04532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a897657938dec83a96596a2ebdef9d7b2181c63ab35a3a1402d954bea1b2700529895c38

=== Addresses Generated from Compressed Public Key ===
P2PKH:        15yUZeoK3JKH8d9c1PVoz8yqiqBva23KfQ
P2SH:         36fVVCHkbCdfDnr38VAQQmLmsMUe67Ewax
P2SH-P2WPKH:  3DT5D6KFyXvTT4yvzVUqsB9X6RFZPd2SvB
BECH32:       bc1qx68vutnvnytpwsseer2yqcdpjujpvm349xssxf
BECH32M:      bc1px68vutnvnytpwsseer2yqcdpjujpvm34wc8mtz
P2WSH:        bc1pvk7rzqzw7tj75sdn5cr4dsxhucmdf72w8hp2kly3nav6t9hve7vqcf9cnj
P2WSH-P2WPKH: bc1qvwhrpx24mlvmcdv9qrf3kj2ej6xmcglz88f24ncpjryup2j9805s3tt58f

=== Addresses Generated from Uncompressed Public Key ===
P2PKH:        1BrD5DzPkpoD3t3VnfPnN13eLFUwyZDJzZ
P2SH:         3CYDzmUqJj7b93jvum4NndQaUmmfXT8GZx
P2SH-P2WPKH:  3BxiH6mZtPgdBpRnPFZPkHDmoVNd4s59Uu (Note: Non-standard)
BECH32:       bc1qwmlm032yq4qakq4vr9a66smrttks46psusqava (Note: Non-standard)
BECH32M:      bc1pwmlm032yq4qakq4vr9a66smrttks46pshwhkpk (Note: Non-standard)
P2WSH:        bc1psh263mc3qka02cj0st7kr5zszslpsz0tr367y0krr7n9ymxdh2dqq5plxf
P2WSH-P2WPKH: bc1qjgtpzhjq6ccxyv3ewr3z528h53zt6kj3yqdtf73ku9dvky5ch6tqfcw4wm (Note: Non-standard)
```


Public Key (Compressed,   33 bytes): 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579

Just like the matching public key, the kangaroo's home was found.

Match found!
  Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579
  Subtracted Scalar (k): 0x0
  Fission Depth: 16 / 20
  Fission Path (+1 choices): 0010000110110100
  Matched Public Key: 039c600d26249ba3a60179e48afef08f673d9b0c88623e9532c9b09bf5e82ca29c

**********************************************************************************************************************************************************

# findkeys Instructions and Tests
```
./findkeys -h
Usage: ./findkeys -p <pubkey/file> -s <start_hex> <end_hex> -v <iter> <divisor> -t <threads> -f <compare_file>

  -p: File containing public keys, or a single public key.
  -s: Hexadecimal range for subtraction (start and end).
  -v iter divisor
  -t: Number of threads.
  -f: File with public keys to compare against.
```
1. It is similar to the above program except that the -v command is different. It does not have a fission shrinking range.
2. -v controls the shrinking range through the number of iterations and the divisor. The number of iterations is in front and the number of divisors is in the back.
3. It requires you to customize the shift range, iterations and divisors. If you know the public key in the block chain or puzzle game, but we don't know the private key, we still need to use kangaroos to make them collide. Assuming that the private key is not random and can be divided by an integer, then it is dangerous. The following example.

# Run the instance to match the output
```
./findkeys -p test_pu.txt -s 1 2 -v 134 2 -t 8 -f f4240.txt
Calculating lines in input file...
Total lines approximately: 2
Loading input public keys: [==================================================] 100 %
Successfully loaded 1 public keys to process.
Calculating lines in comparison file...
Total lines approximately: 1000001
Loading  comparison  file: [==================================================] 100 %
Successfully loaded 1000000 comparison keys (after deduplication).
Initializing Bloom filter: [==================================================] 100 %
Bloom filter initialized.

-------------------- Run Configuration --------------------
Input Source: test_pu.txt (1 keys)
Comparison File: f4240.txt (1000000 keys)
Range (start, end): 0x1, 0x2
Division Iterations/Divisor: 134, 2
Threads: 8
---------------------------------------------------------

                                                                                                                                                      
Match found!
Original Public Key: 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Subtracted Scalar: 0 (Original Key)
Division: Iteration 115, Divisor: 2
Matched Public Key: 0338ab8a9dfb8d8a9cfd414caeee5b3c57cff19ad8dced2589af26da58c8cee140
                                                                                                                                                      
Match found!
Original Public Key: 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Subtracted Scalar: 0 (Original Key)
Division: Iteration 115, Divisor: 2
Matched Public Key: 0338ab8a9dfb8d8a9cfd414caeee5b3c57cff19ad8dced2589af26da58c8cee140

.
.
.
                                                                                                                                                     
Match found!
Original Public Key: 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Subtracted Scalar: 0 (Original Key)
Division: Iteration 117, Divisor: 2
Matched Public Key: 03587da3f60e3ac73cb6c9af40e3c5a3dcaf37df584345bdf306c48eacf1629cf1
                                                                                                                                                      
Match found!
Original Public Key: 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Subtracted Scalar: 0 (Original Key)
Division: Iteration 117, Divisor: 2
Matched Public Key: 03587da3f60e3ac73cb6c9af40e3c5a3dcaf37df584345bdf306c48eacf1629cf1
All threads finished. Total operations: 1340
```
# Recovery Instructions

The above is an example of collision and matching.

-------------------- Run Configuration --------------------
Input Source: test_pu.txt (1 keys)
Comparison File: f4240.txt (1000000 keys)
Range (start, end): 0x1, 0x2
Division Iterations/Divisor: 134, 2
Threads: 8
---------------------------------------------------------

These values explain a lot of information. f4240.txt is the public key corresponding to the private key from 1 to f4240. The base point is the tamed kangaroo, which collides with the public key in the wild kangaroo -p loading document Original Public Key: 02532257fdfc2104d151d8d813fe067e66ee72cbbfd842b6af8f23a8b8a8976579.
You can use advanced commands to search for the private key corresponding to the public key in the document:
```
find -type f -exec grep -B 3 "03587da3f60e3ac73cb6c9af40e3c5a3dcaf37df584345bdf306c48eacf1629cf1" {} +
```
Matched Public Key: 03587da3f60e3ac73cb6c9af40e3c5a3dcaf37df584345bdf306c48eacf1629cf1 , which is the public key of the tamed kangaroo, and the private key we know is: 24293 .
If you use the cloner to calculate the cloned public key, you must remember the number of digits, otherwise you will not be able to recover it because it has become wild.

The base point you created with generate_keys is the tamed kangaroo public key, and there is a private key behind it. You can find it out by checking it. If you forget it, you can use VanitySearch to search the address corresponding to the public key from 1 to f4240 and get the private key.

**************************************************************************************************************************************

We use the calc_hex program to restore it and find the home of the wild kangaroo, which is the private key.
```
./calc_hex -h
Usage: ./calc_hex <matched_priv_key_hex> <iterations> <divisor> <subtracted_scalar_hex>

Description:
  Calculates the original private key. All key/scalar inputs are treated as HEXADECIMAL.

Example:
  ./calc_hex 24293 117 2 0

Arguments:
  <matched_priv_key_hex>    : The private key (in hex) of the matched public key.
  <iterations>              : The number of division iterations.
  <divisor>                 : The divisor used in each iteration.
  <subtracted_scalar_hex>   : The scalar (in hex) that was subtracted.
```

First, we need to understand what 24293 117 2 0 is.

24293 is the private key, which is the kangaroo we tamed.

117 is the 117th iteration.

2 is the value used in the division, which you can think of as their path, so that we can find his home, which is the private key.

0 is the subtracted value, which also means shift, and you can think of it as the fork in the path.
```
./calc_hex 24293 117 2 0
=============== Original Key Calculator (Hex Mode) ===============
Input Parameters:
 - Matched Private Key (hex): 0x24293
 - Division Iterations: 117
 - Divisor: 2
 - Subtracted Scalar (hex): 0x0
------------------------------------------------------------------
Calculated Original Private Key:
  - Hex: 0x4852600000000000000000000000000000
  - Dec: 24609825574460351814393422841677904609280
==================================================================
```
As shown above, the private key is in hexadecimal and decimal. The recovery is successful!

Verify it.
```
./key 0x4852600000000000000000000000000000
Input Private Key (Hex): 0x4852600000000000000000000000000000
Private Key (Hex, 32 bytes): 0000000000000000000000000000004852600000000000000000000000000000
WIF (Uncompressed): 5HpHagT65TZzG1PH3CSu63pa9ukSNJoPaPL793acavCA2pQvb9W
WIF (Compressed):   KwDiBf89QgGbjEhKnhXJuHSyLKpiy3ZkeMZL6WPN4hnErnjujRLP

Public Key (Compressed,   33 bytes): 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Public Key (Uncompressed, 65 bytes): 046fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acca9452619257f0fd46cb05aa27e52b372a86d65f3dcb2cadd323e8da776b6ec88

=== Addresses Generated from Compressed Public Key ===
P2PKH:        1Ei1Xn1Vb1WZJPQW5xYWmidH9x6nZSbETu
P2SH:         3FQ2TKVw8upwPZ6wD4D7CLzDJUPWBVunhi
P2SH-P2WPKH:  39uUzRezVH7yqU5PDzeiBnuBAwpBVH5aBR
BECH32:       bc1qjedjrmuu5lle7ucrxppctsza227v9j4tvk4efr
BECH32M:      bc1pjedjrmuu5lle7ucrxppctsza227v9j4t8gzjyg
P2WSH:        bc1pc9j300f6aft929ksf0g4rgnfl94aycvukd50j6jf42n3dccuku4qjeljac
P2WSH-P2WPKH: bc1qfjee8sznkec9q68t26cyltf9u8x4vdxt5taj5xvakgkjqsk6vddqzgptny

=== Addresses Generated from Uncompressed Public Key ===
P2PKH:        1ALED4ACwHSCSk7Lqiu6RM29yQiADFPFM2
P2SH:         3B2F8beeVBkaXuomxpZgqyP67vzshWx9TL
P2SH-P2WPKH:  35AVsUJ4G9swJiriHmmd24NvXGHeHCKXeS (Note: Non-standard)
BECH32:       bc1qvewqlzzj05vxtaeqqdm5zlhypwmj5zxnrnnwlt (Note: Non-standard)
BECH32M:      bc1pvewqlzzj05vxtaeqqdm5zlhypwmj5zxngdy9jq (Note: Non-standard)
P2WSH:        bc1pz7f4acgyfgpqvxdyqa9fa04rkjm9av9ge2rpkhlxcgjqrpwudwysr50prh
P2WSH-P2WPKH: bc1qcg0q44hvesv3rq6098e2x9xyyhf5kxacza7ms370tlpmv0ga236q7kcxsg (Note: Non-standard)
```

Public Key (Compressed,   33 bytes): 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc

Just like the matching public key, the kangaroo's home was found.

Match found!
Original Public Key: 026fbda13593015f261556814178267207824a5a457a5067d1c58398351aae0acc
Subtracted Scalar: 0 (Original Key)
Division: Iteration 117, Divisor: 2
Matched Public Key: 03587da3f60e3ac73cb6c9af40e3c5a3dcaf37df584345bdf306c48eacf1629cf1

**********************************************************************************************************************************************************************
The following is an introduction to auxiliary tools.
```
./pkconvert
Usage 1: ./pkconvert <public_key_hex> [-u|-c]
  <public_key_hex>: Compressed (66 chars) or uncompressed (130 chars) public key hex string.
  -u: Output uncompressed public key.
  -c: Output compressed public key (default).
Usage 2: ./pkconvert <input_file> <output_file> [-u|-c]
  <input_file>: File containing public key hex strings, one per line.
  <output_file>: File to write the converted public key hex strings.
  -u: Output uncompressed public keys.
  -c: Output compressed public keys (default).
```
```
./generate_keys
Usage: ./generate_keys <start_key_hex> <end_key_hex> -o <output_file> [-c|-u] -t <threads> [-v]

Arguments:
  <start_key_hex>   : The starting private key in hexadecimal.
  <end_key_hex>     : The ending private key in hexadecimal.
  -o <output_file>  : The file to save the generated keys to.
  -c | -u           : Key format. -c for compressed, -u for uncompressed.
  -t <threads>      : The number of threads to use for generation.
  -v                : (Optional) Verbose mode. Appends the private key to each public key (e.g., PubKey:PrivKey).

Example:
  ./generate_keys 1 ffffffff -o keys.txt -c -t 8 -v
```
```
./search_script -b 135 135
[+] Analyzing numbers divisible by 2^n (1 <= n <= 135)
[+] Search Range (Hex): [0x4000000000000000000000000000000000, 0x7fffffffffffffffffffffffffffffffff]
----------------------------------------------------------------
n=1: Found 5444517870735015415413993718908291383296 results. First: 0x4000000000000000000000000000000002, Last: 0x7ffffffffffffffffffffffffffffffffe, Step: 0x4
n=2: Found 2722258935367507707706996859454145691648 results. First: 0x4000000000000000000000000000000004, Last: 0x7ffffffffffffffffffffffffffffffffc, Step: 0x8
n=3: Found 1361129467683753853853498429727072845824 results. First: 0x4000000000000000000000000000000008, Last: 0x7ffffffffffffffffffffffffffffffff8, Step: 0x10
n=4: Found 680564733841876926926749214863536422912 results. First: 0x4000000000000000000000000000000010, Last: 0x7ffffffffffffffffffffffffffffffff0, Step: 0x20
n=5: Found 340282366920938463463374607431768211456 results. First: 0x4000000000000000000000000000000020, Last: 0x7fffffffffffffffffffffffffffffffe0, Step: 0x40
n=6: Found 170141183460469231731687303715884105728 results. First: 0x4000000000000000000000000000000040, Last: 0x7fffffffffffffffffffffffffffffffc0, Step: 0x80
n=7: Found 85070591730234615865843651857942052864 results. First: 0x4000000000000000000000000000000080, Last: 0x7fffffffffffffffffffffffffffffff80, Step: 0x100
n=8: Found 42535295865117307932921825928971026432 results. First: 0x4000000000000000000000000000000100, Last: 0x7fffffffffffffffffffffffffffffff00, Step: 0x200
n=9: Found 21267647932558653966460912964485513216 results. First: 0x4000000000000000000000000000000200, Last: 0x7ffffffffffffffffffffffffffffffe00, Step: 0x400
n=10: Found 10633823966279326983230456482242756608 results. First: 0x4000000000000000000000000000000400, Last: 0x7ffffffffffffffffffffffffffffffc00, Step: 0x800
n=11: Found 5316911983139663491615228241121378304 results. First: 0x4000000000000000000000000000000800, Last: 0x7ffffffffffffffffffffffffffffff800, Step: 0x1000
n=12: Found 2658455991569831745807614120560689152 results. First: 0x4000000000000000000000000000001000, Last: 0x7ffffffffffffffffffffffffffffff000, Step: 0x2000
n=13: Found 1329227995784915872903807060280344576 results. First: 0x4000000000000000000000000000002000, Last: 0x7fffffffffffffffffffffffffffffe000, Step: 0x4000
n=14: Found 664613997892457936451903530140172288 results. First: 0x4000000000000000000000000000004000, Last: 0x7fffffffffffffffffffffffffffffc000, Step: 0x8000
n=15: Found 332306998946228968225951765070086144 results. First: 0x4000000000000000000000000000008000, Last: 0x7fffffffffffffffffffffffffffff8000, Step: 0x10000
n=16: Found 166153499473114484112975882535043072 results. First: 0x4000000000000000000000000000010000, Last: 0x7fffffffffffffffffffffffffffff0000, Step: 0x20000
n=17: Found 83076749736557242056487941267521536 results. First: 0x4000000000000000000000000000020000, Last: 0x7ffffffffffffffffffffffffffffe0000, Step: 0x40000
n=18: Found 41538374868278621028243970633760768 results. First: 0x4000000000000000000000000000040000, Last: 0x7ffffffffffffffffffffffffffffc0000, Step: 0x80000
n=19: Found 20769187434139310514121985316880384 results. First: 0x4000000000000000000000000000080000, Last: 0x7ffffffffffffffffffffffffffff80000, Step: 0x100000
n=20: Found 10384593717069655257060992658440192 results. First: 0x4000000000000000000000000000100000, Last: 0x7ffffffffffffffffffffffffffff00000, Step: 0x200000
n=21: Found 5192296858534827628530496329220096 results. First: 0x4000000000000000000000000000200000, Last: 0x7fffffffffffffffffffffffffffe00000, Step: 0x400000
n=22: Found 2596148429267413814265248164610048 results. First: 0x4000000000000000000000000000400000, Last: 0x7fffffffffffffffffffffffffffc00000, Step: 0x800000
n=23: Found 1298074214633706907132624082305024 results. First: 0x4000000000000000000000000000800000, Last: 0x7fffffffffffffffffffffffffff800000, Step: 0x1000000
n=24: Found 649037107316853453566312041152512 results. First: 0x4000000000000000000000000001000000, Last: 0x7fffffffffffffffffffffffffff000000, Step: 0x2000000
n=25: Found 324518553658426726783156020576256 results. First: 0x4000000000000000000000000002000000, Last: 0x7ffffffffffffffffffffffffffe000000, Step: 0x4000000
n=26: Found 162259276829213363391578010288128 results. First: 0x4000000000000000000000000004000000, Last: 0x7ffffffffffffffffffffffffffc000000, Step: 0x8000000
n=27: Found 81129638414606681695789005144064 results. First: 0x4000000000000000000000000008000000, Last: 0x7ffffffffffffffffffffffffff8000000, Step: 0x10000000
n=28: Found 40564819207303340847894502572032 results. First: 0x4000000000000000000000000010000000, Last: 0x7ffffffffffffffffffffffffff0000000, Step: 0x20000000
n=29: Found 20282409603651670423947251286016 results. First: 0x4000000000000000000000000020000000, Last: 0x7fffffffffffffffffffffffffe0000000, Step: 0x40000000
n=30: Found 10141204801825835211973625643008 results. First: 0x4000000000000000000000000040000000, Last: 0x7fffffffffffffffffffffffffc0000000, Step: 0x80000000
n=31: Found 5070602400912917605986812821504 results. First: 0x4000000000000000000000000080000000, Last: 0x7fffffffffffffffffffffffff80000000, Step: 0x100000000
n=32: Found 2535301200456458802993406410752 results. First: 0x4000000000000000000000000100000000, Last: 0x7fffffffffffffffffffffffff00000000, Step: 0x200000000
n=33: Found 1267650600228229401496703205376 results. First: 0x4000000000000000000000000200000000, Last: 0x7ffffffffffffffffffffffffe00000000, Step: 0x400000000
n=34: Found 633825300114114700748351602688 results. First: 0x4000000000000000000000000400000000, Last: 0x7ffffffffffffffffffffffffc00000000, Step: 0x800000000
n=35: Found 316912650057057350374175801344 results. First: 0x4000000000000000000000000800000000, Last: 0x7ffffffffffffffffffffffff800000000, Step: 0x1000000000
n=36: Found 158456325028528675187087900672 results. First: 0x4000000000000000000000001000000000, Last: 0x7ffffffffffffffffffffffff000000000, Step: 0x2000000000
n=37: Found 79228162514264337593543950336 results. First: 0x4000000000000000000000002000000000, Last: 0x7fffffffffffffffffffffffe000000000, Step: 0x4000000000
n=38: Found 39614081257132168796771975168 results. First: 0x4000000000000000000000004000000000, Last: 0x7fffffffffffffffffffffffc000000000, Step: 0x8000000000
n=39: Found 19807040628566084398385987584 results. First: 0x4000000000000000000000008000000000, Last: 0x7fffffffffffffffffffffff8000000000, Step: 0x10000000000
n=40: Found 9903520314283042199192993792 results. First: 0x4000000000000000000000010000000000, Last: 0x7fffffffffffffffffffffff0000000000, Step: 0x20000000000
n=41: Found 4951760157141521099596496896 results. First: 0x4000000000000000000000020000000000, Last: 0x7ffffffffffffffffffffffe0000000000, Step: 0x40000000000
n=42: Found 2475880078570760549798248448 results. First: 0x4000000000000000000000040000000000, Last: 0x7ffffffffffffffffffffffc0000000000, Step: 0x80000000000
n=43: Found 1237940039285380274899124224 results. First: 0x4000000000000000000000080000000000, Last: 0x7ffffffffffffffffffffff80000000000, Step: 0x100000000000
n=44: Found 618970019642690137449562112 results. First: 0x4000000000000000000000100000000000, Last: 0x7ffffffffffffffffffffff00000000000, Step: 0x200000000000
n=45: Found 309485009821345068724781056 results. First: 0x4000000000000000000000200000000000, Last: 0x7fffffffffffffffffffffe00000000000, Step: 0x400000000000
n=46: Found 154742504910672534362390528 results. First: 0x4000000000000000000000400000000000, Last: 0x7fffffffffffffffffffffc00000000000, Step: 0x800000000000
n=47: Found 77371252455336267181195264 results. First: 0x4000000000000000000000800000000000, Last: 0x7fffffffffffffffffffff800000000000, Step: 0x1000000000000
n=48: Found 38685626227668133590597632 results. First: 0x4000000000000000000001000000000000, Last: 0x7fffffffffffffffffffff000000000000, Step: 0x2000000000000
n=49: Found 19342813113834066795298816 results. First: 0x4000000000000000000002000000000000, Last: 0x7ffffffffffffffffffffe000000000000, Step: 0x4000000000000
n=50: Found 9671406556917033397649408 results. First: 0x4000000000000000000004000000000000, Last: 0x7ffffffffffffffffffffc000000000000, Step: 0x8000000000000
n=51: Found 4835703278458516698824704 results. First: 0x4000000000000000000008000000000000, Last: 0x7ffffffffffffffffffff8000000000000, Step: 0x10000000000000
n=52: Found 2417851639229258349412352 results. First: 0x4000000000000000000010000000000000, Last: 0x7ffffffffffffffffffff0000000000000, Step: 0x20000000000000
n=53: Found 1208925819614629174706176 results. First: 0x4000000000000000000020000000000000, Last: 0x7fffffffffffffffffffe0000000000000, Step: 0x40000000000000
n=54: Found 604462909807314587353088 results. First: 0x4000000000000000000040000000000000, Last: 0x7fffffffffffffffffffc0000000000000, Step: 0x80000000000000
n=55: Found 302231454903657293676544 results. First: 0x4000000000000000000080000000000000, Last: 0x7fffffffffffffffffff80000000000000, Step: 0x100000000000000
n=56: Found 151115727451828646838272 results. First: 0x4000000000000000000100000000000000, Last: 0x7fffffffffffffffffff00000000000000, Step: 0x200000000000000
n=57: Found 75557863725914323419136 results. First: 0x4000000000000000000200000000000000, Last: 0x7ffffffffffffffffffe00000000000000, Step: 0x400000000000000
n=58: Found 37778931862957161709568 results. First: 0x4000000000000000000400000000000000, Last: 0x7ffffffffffffffffffc00000000000000, Step: 0x800000000000000
n=59: Found 18889465931478580854784 results. First: 0x4000000000000000000800000000000000, Last: 0x7ffffffffffffffffff800000000000000, Step: 0x1000000000000000
n=60: Found 9444732965739290427392 results. First: 0x4000000000000000001000000000000000, Last: 0x7ffffffffffffffffff000000000000000, Step: 0x2000000000000000
n=61: Found 4722366482869645213696 results. First: 0x4000000000000000002000000000000000, Last: 0x7fffffffffffffffffe000000000000000, Step: 0x4000000000000000
n=62: Found 2361183241434822606848 results. First: 0x4000000000000000004000000000000000, Last: 0x7fffffffffffffffffc000000000000000, Step: 0x8000000000000000
n=63: Found 1180591620717411303424 results. First: 0x4000000000000000008000000000000000, Last: 0x7fffffffffffffffff8000000000000000, Step: 0x10000000000000000
n=64: Found 590295810358705651712 results. First: 0x4000000000000000010000000000000000, Last: 0x7fffffffffffffffff0000000000000000, Step: 0x20000000000000000
n=65: Found 295147905179352825856 results. First: 0x4000000000000000020000000000000000, Last: 0x7ffffffffffffffffe0000000000000000, Step: 0x40000000000000000
n=66: Found 147573952589676412928 results. First: 0x4000000000000000040000000000000000, Last: 0x7ffffffffffffffffc0000000000000000, Step: 0x80000000000000000
n=67: Found 73786976294838206464 results. First: 0x4000000000000000080000000000000000, Last: 0x7ffffffffffffffff80000000000000000, Step: 0x100000000000000000
n=68: Found 36893488147419103232 results. First: 0x4000000000000000100000000000000000, Last: 0x7ffffffffffffffff00000000000000000, Step: 0x200000000000000000
n=69: Found 18446744073709551616 results. First: 0x4000000000000000200000000000000000, Last: 0x7fffffffffffffffe00000000000000000, Step: 0x400000000000000000
n=70: Found 9223372036854775808 results. First: 0x4000000000000000400000000000000000, Last: 0x7fffffffffffffffc00000000000000000, Step: 0x800000000000000000
n=71: Found 4611686018427387904 results. First: 0x4000000000000000800000000000000000, Last: 0x7fffffffffffffff800000000000000000, Step: 0x1000000000000000000
n=72: Found 2305843009213693952 results. First: 0x4000000000000001000000000000000000, Last: 0x7fffffffffffffff000000000000000000, Step: 0x2000000000000000000
n=73: Found 1152921504606846976 results. First: 0x4000000000000002000000000000000000, Last: 0x7ffffffffffffffe000000000000000000, Step: 0x4000000000000000000
n=74: Found 576460752303423488 results. First: 0x4000000000000004000000000000000000, Last: 0x7ffffffffffffffc000000000000000000, Step: 0x8000000000000000000
n=75: Found 288230376151711744 results. First: 0x4000000000000008000000000000000000, Last: 0x7ffffffffffffff8000000000000000000, Step: 0x10000000000000000000
n=76: Found 144115188075855872 results. First: 0x4000000000000010000000000000000000, Last: 0x7ffffffffffffff0000000000000000000, Step: 0x20000000000000000000
n=77: Found 72057594037927936 results. First: 0x4000000000000020000000000000000000, Last: 0x7fffffffffffffe0000000000000000000, Step: 0x40000000000000000000
n=78: Found 36028797018963968 results. First: 0x4000000000000040000000000000000000, Last: 0x7fffffffffffffc0000000000000000000, Step: 0x80000000000000000000
n=79: Found 18014398509481984 results. First: 0x4000000000000080000000000000000000, Last: 0x7fffffffffffff80000000000000000000, Step: 0x100000000000000000000
n=80: Found 9007199254740992 results. First: 0x4000000000000100000000000000000000, Last: 0x7fffffffffffff00000000000000000000, Step: 0x200000000000000000000
n=81: Found 4503599627370496 results. First: 0x4000000000000200000000000000000000, Last: 0x7ffffffffffffe00000000000000000000, Step: 0x400000000000000000000
n=82: Found 2251799813685248 results. First: 0x4000000000000400000000000000000000, Last: 0x7ffffffffffffc00000000000000000000, Step: 0x800000000000000000000
n=83: Found 1125899906842624 results. First: 0x4000000000000800000000000000000000, Last: 0x7ffffffffffff800000000000000000000, Step: 0x1000000000000000000000
n=84: Found 562949953421312 results. First: 0x4000000000001000000000000000000000, Last: 0x7ffffffffffff000000000000000000000, Step: 0x2000000000000000000000
n=85: Found 281474976710656 results. First: 0x4000000000002000000000000000000000, Last: 0x7fffffffffffe000000000000000000000, Step: 0x4000000000000000000000
n=86: Found 140737488355328 results. First: 0x4000000000004000000000000000000000, Last: 0x7fffffffffffc000000000000000000000, Step: 0x8000000000000000000000
n=87: Found 70368744177664 results. First: 0x4000000000008000000000000000000000, Last: 0x7fffffffffff8000000000000000000000, Step: 0x10000000000000000000000
n=88: Found 35184372088832 results. First: 0x4000000000010000000000000000000000, Last: 0x7fffffffffff0000000000000000000000, Step: 0x20000000000000000000000
n=89: Found 17592186044416 results. First: 0x4000000000020000000000000000000000, Last: 0x7ffffffffffe0000000000000000000000, Step: 0x40000000000000000000000
n=90: Found 8796093022208 results. First: 0x4000000000040000000000000000000000, Last: 0x7ffffffffffc0000000000000000000000, Step: 0x80000000000000000000000
n=91: Found 4398046511104 results. First: 0x4000000000080000000000000000000000, Last: 0x7ffffffffff80000000000000000000000, Step: 0x100000000000000000000000
n=92: Found 2199023255552 results. First: 0x4000000000100000000000000000000000, Last: 0x7ffffffffff00000000000000000000000, Step: 0x200000000000000000000000
n=93: Found 1099511627776 results. First: 0x4000000000200000000000000000000000, Last: 0x7fffffffffe00000000000000000000000, Step: 0x400000000000000000000000
n=94: Found 549755813888 results. First: 0x4000000000400000000000000000000000, Last: 0x7fffffffffc00000000000000000000000, Step: 0x800000000000000000000000
n=95: Found 274877906944 results. First: 0x4000000000800000000000000000000000, Last: 0x7fffffffff800000000000000000000000, Step: 0x1000000000000000000000000
n=96: Found 137438953472 results. First: 0x4000000001000000000000000000000000, Last: 0x7fffffffff000000000000000000000000, Step: 0x2000000000000000000000000
n=97: Found 68719476736 results. First: 0x4000000002000000000000000000000000, Last: 0x7ffffffffe000000000000000000000000, Step: 0x4000000000000000000000000
n=98: Found 34359738368 results. First: 0x4000000004000000000000000000000000, Last: 0x7ffffffffc000000000000000000000000, Step: 0x8000000000000000000000000
n=99: Found 17179869184 results. First: 0x4000000008000000000000000000000000, Last: 0x7ffffffff8000000000000000000000000, Step: 0x10000000000000000000000000
n=100: Found 8589934592 results. First: 0x4000000010000000000000000000000000, Last: 0x7ffffffff0000000000000000000000000, Step: 0x20000000000000000000000000
n=101: Found 4294967296 results. First: 0x4000000020000000000000000000000000, Last: 0x7fffffffe0000000000000000000000000, Step: 0x40000000000000000000000000
n=102: Found 2147483648 results. First: 0x4000000040000000000000000000000000, Last: 0x7fffffffc0000000000000000000000000, Step: 0x80000000000000000000000000
n=103: Found 1073741824 results. First: 0x4000000080000000000000000000000000, Last: 0x7fffffff80000000000000000000000000, Step: 0x100000000000000000000000000
n=104: Found 536870912 results. First: 0x4000000100000000000000000000000000, Last: 0x7fffffff00000000000000000000000000, Step: 0x200000000000000000000000000
n=105: Found 268435456 results. First: 0x4000000200000000000000000000000000, Last: 0x7ffffffe00000000000000000000000000, Step: 0x400000000000000000000000000
n=106: Found 134217728 results. First: 0x4000000400000000000000000000000000, Last: 0x7ffffffc00000000000000000000000000, Step: 0x800000000000000000000000000
n=107: Found 67108864 results. First: 0x4000000800000000000000000000000000, Last: 0x7ffffff800000000000000000000000000, Step: 0x1000000000000000000000000000
n=108: Found 33554432 results. First: 0x4000001000000000000000000000000000, Last: 0x7ffffff000000000000000000000000000, Step: 0x2000000000000000000000000000
n=109: Found 16777216 results. First: 0x4000002000000000000000000000000000, Last: 0x7fffffe000000000000000000000000000, Step: 0x4000000000000000000000000000
n=110: Found 8388608 results. First: 0x4000004000000000000000000000000000, Last: 0x7fffffc000000000000000000000000000, Step: 0x8000000000000000000000000000
n=111: Found 4194304 results. First: 0x4000008000000000000000000000000000, Last: 0x7fffff8000000000000000000000000000, Step: 0x10000000000000000000000000000
n=112: Found 2097152 results. First: 0x4000010000000000000000000000000000, Last: 0x7fffff0000000000000000000000000000, Step: 0x20000000000000000000000000000
n=113: Found 1048576 results. First: 0x4000020000000000000000000000000000, Last: 0x7ffffe0000000000000000000000000000, Step: 0x40000000000000000000000000000
n=114: Found 524288 results. First: 0x4000040000000000000000000000000000, Last: 0x7ffffc0000000000000000000000000000, Step: 0x80000000000000000000000000000
n=115: Found 262144 results. First: 0x4000080000000000000000000000000000, Last: 0x7ffff80000000000000000000000000000, Step: 0x100000000000000000000000000000
n=116: Found 131072 results. First: 0x4000100000000000000000000000000000, Last: 0x7ffff00000000000000000000000000000, Step: 0x200000000000000000000000000000
n=117: Found 65536 results. First: 0x4000200000000000000000000000000000, Last: 0x7fffe00000000000000000000000000000, Step: 0x400000000000000000000000000000
n=118: Found 32768 results. First: 0x4000400000000000000000000000000000, Last: 0x7fffc00000000000000000000000000000, Step: 0x800000000000000000000000000000
n=119: Found 16384 results. First: 0x4000800000000000000000000000000000, Last: 0x7fff800000000000000000000000000000, Step: 0x1000000000000000000000000000000
n=120: Found 8192 results. First: 0x4001000000000000000000000000000000, Last: 0x7fff000000000000000000000000000000, Step: 0x2000000000000000000000000000000
n=121: Found 4096 results. First: 0x4002000000000000000000000000000000, Last: 0x7ffe000000000000000000000000000000, Step: 0x4000000000000000000000000000000
n=122: Found 2048 results. First: 0x4004000000000000000000000000000000, Last: 0x7ffc000000000000000000000000000000, Step: 0x8000000000000000000000000000000
n=123: Found 1024 results. First: 0x4008000000000000000000000000000000, Last: 0x7ff8000000000000000000000000000000, Step: 0x10000000000000000000000000000000
n=124: Found 512 results. First: 0x4010000000000000000000000000000000, Last: 0x7ff0000000000000000000000000000000, Step: 0x20000000000000000000000000000000
n=125: Found 256 results. First: 0x4020000000000000000000000000000000, Last: 0x7fe0000000000000000000000000000000, Step: 0x40000000000000000000000000000000
n=126: Found 128 results. First: 0x4040000000000000000000000000000000, Last: 0x7fc0000000000000000000000000000000, Step: 0x80000000000000000000000000000000
n=127: Found 64 results. First: 0x4080000000000000000000000000000000, Last: 0x7f80000000000000000000000000000000, Step: 0x100000000000000000000000000000000
n=128: Found 32 results. First: 0x4100000000000000000000000000000000, Last: 0x7f00000000000000000000000000000000, Step: 0x200000000000000000000000000000000
n=129: Found 16 results. First: 0x4200000000000000000000000000000000, Last: 0x7e00000000000000000000000000000000, Step: 0x400000000000000000000000000000000
n=130: Found 8 results. First: 0x4400000000000000000000000000000000, Last: 0x7c00000000000000000000000000000000, Step: 0x800000000000000000000000000000000
n=131: Found 4 results. First: 0x4800000000000000000000000000000000, Last: 0x7800000000000000000000000000000000, Step: 0x1000000000000000000000000000000000
n=132: Found 2 results. First: 0x5000000000000000000000000000000000, Last: 0x7000000000000000000000000000000000, Step: 0x2000000000000000000000000000000000
n=133: Found 1 results. First: 0x6000000000000000000000000000000000, Last: 0x6000000000000000000000000000000000, Step: 0x4000000000000000000000000000000000
n=134: Found 1 results. First: 0x4000000000000000000000000000000000, Last: 0x4000000000000000000000000000000000, Step: 0x8000000000000000000000000000000000
----------------------------------------------------------------
[+] Analysis complete.
```


# Acknowledgements

Assisted by: gemini.

# Sponsorship
If this project is helpful to you, please consider sponsoring. Your support is greatly appreciated. Thank you!
```
BTC: bc1qt3nh2e6gjsfkfacnkglt5uqghzvlrr6jahyj2k
ETH: 0xD6503e5994bF46052338a9286Bc43bC1c3811Fa1
DOGE: DTszb9cPALbG9ESNJMFJt4ECqWGRCgucky
TRX: TAHUmjyzg7B3Nndv264zWYUhQ9HUmX4Xu4
```
# 📜 Disclaimer
This code is only for learning and understanding how it works.
Please ensure that the program is run in a safe environment and comply with local laws and regulations!
The developer is not responsible for any financial losses or legal liabilities caused by the use of this code.

