#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <curl/curl.h>
#include <cstring>
#include <locale.h>

#define CREATE_BY		"IGORERFC"
#define VERSION			"VERSION 3.50"

#define cout_use_rs 	std::cout << endl << " ! Write r for random  ! "<< endl<< " ! Write s for sequence !  "<< endl << endl
#define cout_nouse_rs 	std::cout << endl << " ! r/s doesn't work here ! "<< endl << endl


using namespace std;

enum{
		fatalerror = 1,
		error_nof  = 0
};

static std::string payloadText[11];

const string name_file_mails = "postal.txt";//The files must be created and located in the same folder as the program

const string google_service = "smtp://smtp.gmail.com:587" ;

int number_of_errors = 0;
unsigned long long number_sequence=0;

struct upload_status { int lines_read; };
struct logpass {
	string mail;
	string mail_pass;
};

void v_error(string error_text,bool fatal_or_not) 
{
	std::cout <<endl<<" FATAL ERROR :" <<error_text << endl << endl;
	if(fatal_or_not)exit(-1);
}

void setPayloadText(const std::string &to,
                    const std::string &from,
                    const std::string &cc,
                    const std::string &nameFrom,
                    const std::string &subject,
                    const std::string &body)
{
    payloadText[ 0] = "Date: " ;
    payloadText[ 1] = "To: <"   + to   + ">\r\n";
    payloadText[ 2] = "From: "+nameFrom+" <sender@example.org>\r\n",
    payloadText[ 3] = "Cc: <"   + cc   + "> (" + nameFrom + ")\r\n";
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

CURLcode sendEmail(const std::string &to,
                   const std::string &from,
                   const std::string &cc,
                   const std::string &nameFrom,
                   const std::string &subject,
                   const std::string &body,
                   const std::string &url,
                   const std::string &password)
{
    CURLcode ret = CURLE_OK;

    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.lines_read = 0;

    CURL *curl = curl_easy_init();

    setPayloadText(to, from, cc, nameFrom, subject, body);

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME,     from    .c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD,     password.c_str());
        curl_easy_setopt(curl, CURLOPT_URL,          url     .c_str());

        curl_easy_setopt(curl, CURLOPT_USE_SSL,      (long)CURLUSESSL_ALL);

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM,    ("<" + from + ">").c_str());
        recipients = curl_slist_append(recipients,   ("<" + to   + ">").c_str());
        recipients = curl_slist_append(recipients,   ("<" + cc   + ">").c_str());

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,    recipients);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA,     &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD,       1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE,      1L);
        
     // curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5); 
     // curl_easy_setopt(curl, CURLOPT_PROXY, "109.194.175.135:9050");//Google blocks ;-(
        
		curl_easy_setopt(curl,CURLOPT_VERBOSE,0);
		
        ret = curl_easy_perform(curl);
        
        if (ret != CURLE_OK){
            v_error("curl_easy_perform() failed: \n "+(string)curl_easy_strerror(ret),error_nof);
            number_of_errors++;
		}
        else
           std::cout<<" SEND "<<endl;

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    return ret;
}

void get_mail_from_txt(ifstream * fout, vector <logpass> *mails)
{
	char a[200];
	while (fout->getline(a, INT_MAX)) {
		logpass lp;
		bool b = false;
		for (int i = 0; a[i] != '\0'; i++) {
			if (b == true)
				lp.mail_pass += a[i];
			if (a[i] != ':' && b == false)
				lp.mail += a[i];
			else b = true;
		}
		mails->push_back(lp);
	}
}

string gen_random(const int len) {
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

string rand_str(string str_input)
{
	if (str_input[0]=='r')
		return gen_random(rand() % 25 + 1);
	else
		if(str_input[0]=='s')
			return string(to_string(++number_sequence));
		else
			return str_input;
}

void use_threads_for_send_mail(	vector <logpass> mails, 
								int num_cy,
								string mail_for_send, 
								string mail_name,
								string mail_subject,
								string mail_body) 
{
	for (int i = 0; i < num_cy; i++)
		for (const auto & mail : mails)
		{
			  sendEmail
			  (
				mail_for_send			.c_str(),
				mail.mail				.c_str(),
				mail_for_send			.c_str(),
				rand_str(mail_name)		.c_str(),
				rand_str(mail_subject)	.c_str(),
				rand_str(mail_body)		.c_str(),
				google_service			.c_str(),
				mail.mail_pass			.c_str()
				);
		}
}

void preview()
{
	cout << " CREATE BY " << CREATE_BY << endl;
	cout << " VERSION "   << VERSION   << endl << endl;;
}

int main()
{
	preview();
	srand(time(NULL));
	setlocale(LC_ALL,"Rus"); //Eng also works
	
	vector <logpass> mails;
	
	string mail_for_send;
	string mail_name;
	string mail_subject;
	string mail_body;
	
	
	int num_threads = 0;
	int num_cy = 0;
	
	
	ifstream fout(name_file_mails);
	if (!fout.is_open())
	{
		 v_error(" File opening error! ",fatalerror);
		 exit(-1);
	 }

	get_mail_from_txt(&fout, &mails);
	std::cout << " " << mails.size() << " - GOOGLE MAIL WAS FOUND  " << endl << endl;
	
	std::cout << " Write an email address for sending messages : "	;	std::getline ( std::cin,mail_for_send);

	cout_use_rs;
	
	std::cout << " Enter your name : "								;	std::getline ( std::cin,mail_name);
	std::cout << " Enter the subject of the letter (should r/s) : "	;	std::getline ( std::cin,mail_subject);
	std::cout << " Enter the text of the letter  : "				;	std::getline ( std::cin,mail_body );
	
	cout_nouse_rs;
	
	std::cout << " Enter the number of cycles  ( min - 1 ) : ";
	std::cin >> num_cy;
	
	if (num_cy < 1)	v_error(" Invalid input data ",fatalerror);
	
	std::cout << endl << " Don't specify too much threads, google services will block you ( 10 < )" << endl << endl;
	std::cout << " Enter the number of threads ( min - 1 ) : ";std::cin>>num_threads;
	
	if (num_threads < 1)v_error(" Invalid input data ",fatalerror);
	
	thread* th = new thread[num_threads];//Fine,pc is dead
	for (int i = 0; i < num_threads; i++)
		th[i] = std::thread
		(
		use_threads_for_send_mail, 
		mails, 
		num_cy,
		mail_for_send,
		mail_name,
		mail_subject,
		mail_body
		);
	for (int i = 0; i < num_threads; i++)
		th[i].join();
	cout<<endl<<" "<<num_cy*num_threads*mails.size()-number_of_errors<<" - emails sent successfully !"<<endl;
}