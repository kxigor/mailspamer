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

If you specify too many threads , a segmentation error may occur, since your device may not be able to handle the load (I have a limit on my phone of 50 )

I see some difficulties with the strings in the file, so if the string is very long, then most likely there will be a segmentation error

Number formula sent email: number of threads * number of cycles * number of emails

![alt text](https://i.yapx.ru/IQ36w.jpg)
![alt text](https://i.yapx.ru/IQ366.jpg)
![alt text](https://i.yapx.ru/IQ37B.jpg)
![alt text](https://i.yapx.ru/IQ37M.png)
![alt text](https://i.yapx.ru/IQ37S.jpg)
![alt text](https://i.yapx.ru/IQ37Y.jpg)
