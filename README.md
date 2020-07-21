# Mailspamer
A program that can be used to spam emails.Extensive functionality . Written in c++

### Installation and use
To install it you will need the curl library and of course c++

```
sudo apt-get install g++
sudo apt-get install curl

//Depends on your OS
sudo apt-get install curl libcurl4 libcurl3-dev 

git clone https://github.com/Garik11/mailspamer
cd mailspamer
g++ mailspamer.cpp -lcurl -o name -pthread
./name
```
+ You need to file postal.txt enter gmail mail in the format :**gmail@gmail.com:pass**
+ Gmail must have access enabled through unreliable sources (otherwise it will not allow the program to send mail)
![alt text](https://i.yapx.ru/IGXoW.png)
![alt text](https://i.yapx.ru/IGXo1.jpg)

If you specify too many threads , a segmentation error may occur, since your device may not be able to handle the load (I have a limit on my phone of 50 )
