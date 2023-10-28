# ARTrade

## always running trading system

### how to use? (Ubuntu)
1. download cmake
    ``` shell
    sudo apt install cmake
    ```
2. download g++
    ```shell
    sudo apt install g++
    ```
3. download OpenSSL
    ```shell
    sudo apt-get install libssl-dev
    ```
4. download Boost
    ```shell
    sudo apt-get install libboost-all-dev
    ```
5. compile project
    ``` shell
    compile bybit
    > sh compile.sh -p bybit
    ```
6. run (example: bybit)
    ```shell
    > cd bybit
    > ./bybit -r ./
    ```