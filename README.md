# Mailspamer
The program has been closed since the summer of 2022.

A program that can be used to spam emails.Extensive functionality. The program is written in the c++programming language

With this program, you can very much (if you have a sufficient number of Google accounts) load / spam someone's mail (not necessarily google), I checked, there are failures and instability due to a large number of emails , they begin to go not to the spam folder :)

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
+ You need to file "emails" enter gmail mails in the format :**gmail@gmail.com:pass**
+ Gmail must have access enabled through unreliable sources (otherwise it will not allow the program to send mail)

If you specify too many threads , a segmentation error may occur, since your device may not be able to handle the load (I have a limit on my phone of 50 )

I see some difficulties with the strings in the file, so if the string is very long, then most likely there will be a segmentation error


```
__________*_____*
_________*_*****_*
________*_(O)_(O)_*
_______**____V____**
_______**_________**
_______**_________**
________*_________*
_________***___***

```
