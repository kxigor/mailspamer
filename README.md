# mailsender
A program that can be used to spam emails.Extensive functionality . Written in c++

### Installation and use
To install it you will need the curl library and of course c++

```
sudo apt-get install g++
sudo apt-get install curl
sudo apt-get install curl libcurl3 libcurl3-dev
// or libcurl4
git clone https://github.com/Garik11/mailsender
cd mailsender
g++ mailsender.cpp -lcurl -o name -pthread
./name
```
+ You need to file postal.txt enter gmail mail in the format :**gmail@gmail.com:pass**
+ Gmail must have access enabled through unreliable sources (otherwise it will not allow the program to send mail)
